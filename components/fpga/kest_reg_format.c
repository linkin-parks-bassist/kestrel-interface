#include "kest_int.h"

typedef struct {
	kest_numeric_format format;
	int clips;
} format_candidate;

static int same_format(kest_numeric_format a, kest_numeric_format b)
{
	return a.fractional_bits == b.fractional_bits &&
		a.is_unsigned == b.is_unsigned && a.overflow == b.overflow;
}

static int expression_register(kest_block_operand op)
{
	return op.type == BLOCK_OPERAND_TYPE_R && op.addr >= 0 && op.addr < 2;
}

/* Existing channel/constant words cannot be freely rescaled like expressions. */
static int candidates_for(kest_block *block, kest_block_operand op,
	const kest_arg_numeric_policy *policy, kest_scope *scope,
	format_candidate *candidates)
{
	int expression = expression_register(op);
	int fixed_fraction = KEST_FPGA_DATA_WIDTH - 1;
	float minimum = 0, maximum = 0;
	if (expression)
	{
		kest_block_reg_val *reg = op.addr ? &block->reg_1 : &block->reg_0;
		if (!reg->active || !reg->expr) return 0;
		minimum = kest_expression_compute_min(reg->expr, scope);
		maximum = kest_expression_compute_max(reg->expr, scope);
		if (isnan(minimum) || isnan(maximum) || minimum > maximum) return 0;
	}
	else if (op.type == BLOCK_OPERAND_TYPE_R && op.addr == POS_ONE_REGISTER_ADDR)
		fixed_fraction--;

	int count = 0;
	for (int fraction = KEST_FPGA_DATA_WIDTH; fraction >= 0; fraction--)
	{
		if (!expression && fraction != fixed_fraction) continue;
		for (int unsigned_value = 0; unsigned_value < 2; unsigned_value++)
		{
			uint32_t permitted = unsigned_value ? policy->unsigned_formats : policy->signed_formats;
			if (!(permitted & (UINT32_C(1) << fraction))) continue;
			if (unsigned_value && op.type == BLOCK_OPERAND_TYPE_R && op.addr == NEG_ONE_REGISTER_ADDR)
				continue;
			kest_numeric_format format = {fraction, unsigned_value, policy->overflow};
			float low, high;
			if (kest_numeric_format_bounds(format, KEST_FPGA_DATA_WIDTH, &low, &high) != NO_ERROR)
				continue;
			int clips = expression && (minimum < low || maximum > high);
			if (clips && policy->overflow == KEST_NUMERIC_REJECT) continue;
			candidates[count++] = (format_candidate){format, clips};
		}
	}
	return count;
}

int kest_resolve_block_formats(kest_block *block, kest_scope *scope)
{
	if (!block) return ERR_NULL_PTR;
	const kest_asm_instr_desc *desc = block->desc;
	if (!desc || !desc->numeric || !desc->numeric->resolve_field)
		return ERR_BAD_ARGS;
	const kest_instr_numeric_policy *policy = desc->numeric;
	kest_block_operand operands[3] = {block->arg_a, block->arg_b, block->arg_c};
	format_candidate candidates[3][2 * (KEST_FPGA_DATA_WIDTH + 1)];
	int count[3];
	for (int arg = 0; arg < 3; arg++)
	{
		count[arg] = candidates_for(block, operands[arg], &policy->args[arg], scope, candidates[arg]);
		if (!count[arg]) return ERR_VALUE_OUT_OF_BOUNDS;
	}

	kest_numeric_format best[3];
	int best_field = 0, best_clips = 4, best_clipped_precision = 0, best_precision = -1;
	for (int a = 0; a < count[0]; a++)
	for (int b = 0; b < count[1]; b++)
	for (int c = 0; c < count[2]; c++)
	{
		format_candidate tuple[3] = {candidates[0][a], candidates[1][b], candidates[2][c]};
		kest_numeric_format formats[3] = {tuple[0].format, tuple[1].format, tuple[2].format};
		int compatible = 1;
		for (int i = 0; i < 3; i++)
		for (int j = i + 1; j < 3; j++)
			if (expression_register(operands[i]) && expression_register(operands[j]) &&
				operands[i].addr == operands[j].addr && !same_format(formats[i], formats[j]))
				compatible = 0;
		if (!compatible) continue;
		int field;
		if (policy->resolve_field(block, formats, &field) != NO_ERROR || field < 0 || field > 31)
			continue;
		int clips = tuple[0].clips + tuple[1].clips + tuple[2].clips;
		int clipped_precision = tuple[0].clips * formats[0].fractional_bits +
			tuple[1].clips * formats[1].fractional_bits + tuple[2].clips * formats[2].fractional_bits;
		int precision = formats[0].fractional_bits + formats[1].fractional_bits + formats[2].fractional_bits;
		/* When saturation is unavoidable, prefer range before precision. */
		if (clips > best_clips || (clips == best_clips &&
			(clipped_precision > best_clipped_precision ||
			(clipped_precision == best_clipped_precision && precision <= best_precision)))) continue;
		memcpy(best, formats, sizeof(best));
		best_field = field;
		best_clips = clips;
		best_clipped_precision = clipped_precision;
		best_precision = precision;
	}
	if (best_precision < 0) return ERR_VALUE_OUT_OF_BOUNDS;
	for (int arg = 0; arg < 3; arg++)
		if (expression_register(operands[arg]))
			(operands[arg].addr ? &block->reg_1 : &block->reg_0)->format = best[arg];
	block->shift = best_field;
	return NO_ERROR;
}

int kest_compute_register_formats(kest_block_pll *blocks, kest_scope *scope)
{
	for (; blocks; blocks = blocks->next)
	{
		if (!blocks->data) continue;
		int result = kest_resolve_block_formats(blocks->data, scope);
		if (result != NO_ERROR) return result;
	}
	return NO_ERROR;
}
