#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static const char *FNAME = "kest_fpga_encoding.c";

#include "kest_int.h"

//#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 1
//#endif
/*
#ifdef PRINT_TRANSFER_BATCHES
#ifdef PRINTLINES_ALLOWED
#undef PRINTLINES_ALLOWED
#endif
#define PRINTLINES_ALLOWED 1
#endif
*/

int kest_fpga_block_opcode_format(int opcode)
{
	return (opcode == BLOCK_INSTR_LUT_READ
		 || opcode == BLOCK_INSTR_DELAY_READ
		 || opcode == BLOCK_INSTR_DELAY_WRITE
		 || opcode == BLOCK_INSTR_MEM_READ
		 || opcode == BLOCK_INSTR_MEM_WRITE
		 || opcode == BLOCK_INSTR_FILTER)
		 ? INSTR_FORMAT_B : INSTR_FORMAT_A;
}

int kest_block_instr_format(kest_block *block)
{
	if (!block) return 0;
	
	return kest_fpga_block_opcode_format(block->instr);
}

uint32_t kest_encode_dsp_block_instr_type_a(kest_block *block)
{
	if (!block) return 0;
	
	return place_bits( 5,  0, block->instr)
	 | place_bits( 9,  6, block->arg_a.addr) | ((!!(block->arg_a.type == KEST_ASM_ARG_EXPR)) << 10)
	 | place_bits(14, 11, block->arg_b.addr) | ((!!(block->arg_b.type == KEST_ASM_ARG_EXPR)) << 15)
	 | place_bits(19, 16, block->arg_c.addr) | ((!!(block->arg_c.type == KEST_ASM_ARG_EXPR)) << 20)
	 | place_bits(24, 21, block->dest)
	 | place_bits(29, 25, block->shift) | ((!!block->saturate_disable) << 30) | ((block->shift == 255) << 31);
}

uint32_t kest_encode_dsp_block_instr_type_b(kest_block *block, int res_handle)
{
	return place_bits(5, 0, block->instr) | (1 << 5)
			 | place_bits( 9,  6, block->arg_a.addr) | ((!!(block->arg_a.type == KEST_ASM_ARG_EXPR)) << 10)
			 | place_bits(14, 11, block->arg_b.addr) | ((!!(block->arg_b.type == KEST_ASM_ARG_EXPR)) << 15)
			 | place_bits(19, 16, block->dest)
			 | place_bits(31, 20, res_handle);
}

uint32_t kest_block_instr_encode_resource_aware(kest_block *block, const kest_eff_resource_report *res)
{
	if (!block)
		return 0;
	
	int res_handle = 0;
	
	if (kest_block_instr_format(block))
	{
		if (!block->res)
			return 0;
		
		switch (block->res->type)
		{
			case KEST_DSP_RESOURCE_LUT:
				res_handle = block->res->handle;
				break;
				
			case KEST_DSP_RESOURCE_MEM:
				res_handle = block->res->handle + (res ? res->memory : 0);
				break;
				
			case KEST_DSP_RESOURCE_DELAY:
				res_handle = block->res->handle + (res ? res->delays : 0);
				break;
				
			case KEST_DSP_RESOURCE_FILTER:
				res_handle = block->res->handle + (res ? res->filters : 0);
				break;
				
		}
		
		return kest_encode_dsp_block_instr_type_b(block, res_handle);
	}
	else
	{
		return kest_encode_dsp_block_instr_type_a(block);
	}
}

int kest_fpga_batch_append_block_number(kest_fpga_transfer_batch *batch, int block)
{
	if (KEST_FPGA_N_BLOCKS > 256)
		return kest_fpga_batch_append_16(batch, block & 0xFFFF);
	else
		return kest_fpga_batch_append(batch, block & 0xFF);
	
	return NO_ERROR;
}


int kest_fpga_batch_append_block_instr(kest_fpga_transfer_batch *batch, kest_block *block, const kest_eff_resource_report *res, int pos)
{
	if (!batch || !block)
		return ERR_NULL_PTR;
	
	uint32_t instr;
	
	kest_fpga_batch_append(batch, COMMAND_WRITE_BLOCK_INSTR);
	kest_fpga_batch_append_block_number(batch, pos);
	kest_fpga_batch_append_32(batch, kest_block_instr_encode_resource_aware(block, res));
	
	return NO_ERROR;
}

int kest_fpga_batch_append_block_regs(kest_fpga_transfer_batch *batch, kest_block *block, kest_expr_scope *scope, int pos)
{
	KEST_PRINTF("kest_fpga_batch_append_block_regs(batch = %p, block = %p, scope = %p, pos = %d)\n");
	if (!batch || !block)
		return ERR_NULL_PTR;
	
	float v;
	int32_t s;
	
	if (block->reg_0.active && block->reg_0.expr)
	{
		KEST_PRINTF("register 0 active. evaluating...\n");
		v = kest_expression_evaluate(block->reg_0.expr, scope);
		KEST_PRINTF("result: %f. formatting to q%d.%d...\n", v, 1+block->reg_0.format, 15-block->reg_0.format);
		s = float_to_q_nminus1(v, block->reg_0.format);
		KEST_PRINTF("result: %d = 0x%02x\n", s, s);
		
		kest_fpga_batch_append(batch, COMMAND_WRITE_BLOCK_REG_0);
		kest_fpga_batch_append_block_number(batch, pos);
		kest_fpga_batch_append_16(batch, s);
	}
	
	if (block->reg_1.active && block->reg_1.expr)
	{
		KEST_PRINTF("register 1 active. evaluating...\n");
		v = kest_expression_evaluate(block->reg_1.expr, scope);
		KEST_PRINTF("result: %f. formatting to q%d.%d...\n", v, 1+block->reg_1.format, 15-block->reg_1.format);
		s = float_to_q_nminus1(v, block->reg_1.format);
		KEST_PRINTF("result: %d = 0x%02x\n", s, s);
		
		kest_fpga_batch_append(batch, COMMAND_WRITE_BLOCK_REG_1);
		kest_fpga_batch_append_block_number(batch, pos);
		kest_fpga_batch_append_16(batch, s);
	}
	
	return NO_ERROR;
}


int kest_fpga_batch_append_block_register_updates(kest_fpga_transfer_batch *batch, kest_block *block, kest_expr_scope *scope, int pos)
{
	if (!batch || !block)
		return ERR_NULL_PTR;
	
	float v;
	kest_fpga_sample_t s;
	
	if (block->reg_0.active)
	{
		if (block->reg_0.expr)
		{
			v = kest_expression_evaluate(block->reg_0.expr, scope);
			
			s = float_to_q_nminus1(v, block->reg_0.format);
			
			kest_fpga_batch_append(batch, COMMAND_UPDATE_BLOCK_REG_0);
			kest_fpga_batch_append_block_number(batch, pos);
			kest_fpga_batch_append_16(batch, s);
		}
	}
	
	if (block->reg_1.active)
	{
		if (block->reg_1.expr)
		{
			v = kest_expression_evaluate(block->reg_1.expr, scope);
			
			s = float_to_q_nminus1(v, block->reg_1.format);
			
			kest_fpga_batch_append(batch, COMMAND_UPDATE_BLOCK_REG_1);
			kest_fpga_batch_append_block_number(batch, pos);
			kest_fpga_batch_append_16(batch, s);
		}
	}
	
	return NO_ERROR;
}

int kest_fpga_transfer_batch_append_effect_register_updates(kest_fpga_transfer_batch *batch, kest_effect_desc *eff, kest_expr_scope *scope, int pos)
{
	if (!batch || !eff)
		return ERR_NULL_PTR;
	
	int i = 0;
	kest_block_pll *current = eff->blocks;
	
	while (current)
	{
		kest_fpga_batch_append_block_register_updates(batch, current->data, scope, pos + i);
		
		current = current->next;
		i++;
	}
	
	return NO_ERROR;
}

int kest_fpga_batch_append_block(kest_fpga_transfer_batch *batch, kest_block *block, const kest_eff_resource_report *res, kest_expr_scope *scope, int pos)
{
	if (!batch || !block)
		return ERR_NULL_PTR;
	
	kest_fpga_batch_append_block_instr(batch, block, res, pos);
	kest_fpga_batch_append_block_regs(batch, block, scope, pos);
	
	return NO_ERROR;
}

int kest_fpga_batch_append_blocks(kest_fpga_transfer_batch *batch, kest_block_pll *blocks, const kest_eff_resource_report *res, kest_expr_scope *scope, int pos)
{
	if (!batch || !blocks)
		return ERR_NULL_PTR;
	
	kest_block_pll *current = blocks;
	
	int i = 0;
	
	int ret_val;
	while (current)
	{
		if (current->data && (ret_val = kest_fpga_batch_append_block(batch, current->data, res, scope, pos + i)) != NO_ERROR)
			return ret_val;
		
		current = current->next;
		i++;
	}
	
	return NO_ERROR;
}

int kest_fpga_batch_append_resource(kest_fpga_transfer_batch *batch, kest_dsp_resource *res, const kest_eff_resource_report *rpt, kest_expr_scope *scope)
{
	if (!batch || !res || !rpt)
		return ERR_NULL_PTR;
	
	uint32_t size;
	uint32_t delay;
	
	int handle;
	kest_filter *filter = (kest_filter*)res->data;
	float c;
	int32_t s;
	
	switch (res->type)
	{
		case KEST_DSP_RESOURCE_DELAY:
			kest_fpga_batch_append(batch, COMMAND_ALLOC_DELAY);
			
			delay = (uint32_t)(ceilf(kest_expression_evaluate(res->delay, scope)) * 0.001 * KEST_FPGA_SAMPLE_RATE);
			
			if (res->size)
				size = (uint32_t)(ceilf(kest_expression_evaluate(res->size, scope)) * 0.001 * KEST_FPGA_SAMPLE_RATE);
			else
				size = delay;
			
			if (size < delay + 1)
				size = delay + 1;
			
			kest_fpga_batch_append_24(batch, size);
			kest_fpga_batch_append_24(batch, delay << DELAY_FORMAT);
			
			break;
		case KEST_DSP_RESOURCE_FILTER:
			if (!filter)
				return ERR_BAD_ARGS;
			
			kest_fpga_batch_append(batch, COMMAND_ALLOC_FILTER);
			kest_fpga_batch_append(batch, filter->format && 0xFF);
			kest_fpga_batch_append_16(batch, filter->feed_forward);
			kest_fpga_batch_append_16(batch, filter->feed_back);
			
			handle = res->handle + rpt->filters;
			
			KEST_PRINTF("Sending filter coefficients; handle %d = %d + %d.\n", handle, res->handle, rpt->filters);
			for (int i = 0; i < filter->coefs.count; i++)
			{
				kest_fpga_batch_append(batch, COMMAND_WRITE_FILTER_COEF);
				kest_fpga_batch_append(batch, handle & 0xFF);
				kest_fpga_batch_append_16(batch, i);
				c = kest_expression_evaluate(filter->coefs.entries[i], scope);
				KEST_PRINTF("Coefficient %d: %.06f\n", i, c);
				s = float_to_q_nminus1_18bit(c, filter->format);
				KEST_PRINTF("Converting to q%d.%d, we get %d. Masked to 18 bits, that's %d.\n",
					1 + filter->format, KEST_FPGA_FILTER_WIDTH - 1 - filter->format, s, s & ((1u << 18) - 1));
				kest_fpga_batch_append_24(batch, s & ((1u << 18) - 1));
			}
			
			break;
	}
	
	return NO_ERROR;
}

int kest_fpga_batch_append_resources(kest_fpga_transfer_batch *batch, kest_dsp_resource_pll *list, const kest_eff_resource_report *rpt, kest_expr_scope *scope)
{
	if (!batch || !list || !rpt)
		return ERR_NULL_PTR;
	
	kest_dsp_resource_pll *current = list;
	
	while (current)
	{
		kest_fpga_batch_append_resource(batch, current->data, rpt, scope);
		current = current->next;
	}
	
	return NO_ERROR;
}

int kest_fpga_batch_append_eff_desc(kest_fpga_transfer_batch *batch, kest_effect_desc *eff, const kest_eff_resource_report *res, kest_expr_scope *scope, int pos)
{
	if (!batch || !eff || !res)
		return ERR_NULL_PTR;
	
	kest_fpga_batch_append_resources(batch, eff->resources, res, scope);
	kest_fpga_batch_append_blocks(batch, eff->blocks, res, scope, pos);
	
	return NO_ERROR;
}

int kest_fpga_batch_append_effect(kest_fpga_transfer_batch *batch, kest_effect *effect, kest_eff_resource_report *res, int *pos)
{
	if (!batch || !effect || !res || !pos)
		return ERR_NULL_PTR;
	
	if (!effect->eff)
		return ERR_BAD_ARGS;
	
	kest_expr_scope *scope = effect->scope;
	
	effect->block_position = *pos;
	KEST_PRINTF("Updating effect %p's block position to %d. New value: %d\n", effect, *pos, effect->block_position);
	
	kest_fpga_batch_append_eff_desc(batch, effect->eff, res, scope, *pos);
	
	kest_resource_report_integrate(res, &effect->eff->res_rpt);
	
	*pos += effect->eff->res_rpt.blocks;
	
	return NO_ERROR;
}

int kest_fpga_batch_append_effects(kest_fpga_transfer_batch *batch, kest_effect_pll *list, kest_eff_resource_report *res, int *pos)
{
	if (!batch || !list || !res || !pos)
		return ERR_NULL_PTR;
	KEST_PRINTF("kest_fpga_batch_append_effects\n");
	
	kest_effect_pll *current = list;
	
	while (current)
	{
		kest_fpga_batch_append_effect(batch, current->data, res, pos);
		current = current->next;
	}
	
	KEST_PRINTF("kest_fpga_batch_append_effects done\n");
	return NO_ERROR;
}

char *kest_block_opcode_to_string(uint32_t opcode)
{
	switch (opcode)
	{
		case BLOCK_INSTR_NOP: 			return (char*)"BLOCK_INSTR_NOP";
		case BLOCK_INSTR_LSH: 			return (char*)"BLOCK_INSTR_LSH";
		case BLOCK_INSTR_RSH: 			return (char*)"BLOCK_INSTR_RSH";
		case BLOCK_INSTR_ARSH: 			return (char*)"BLOCK_INSTR_ARSH";
		case BLOCK_INSTR_MADD: 			return (char*)"BLOCK_INSTR_MADD";
		case BLOCK_INSTR_ABS: 			return (char*)"BLOCK_INSTR_ABS";
		case BLOCK_INSTR_LUT_READ:		return (char*)"BLOCK_INSTR_LUT_READ";
		case BLOCK_INSTR_DELAY_READ: 	return (char*)"BLOCK_INSTR_DELAY_READ";
		case BLOCK_INSTR_DELAY_WRITE: 	return (char*)"BLOCK_INSTR_DELAY_WRITE";
		case BLOCK_INSTR_MEM_WRITE:		return (char*)"BLOCK_INSTR_MEM_WRITE";
		case BLOCK_INSTR_FILTER:		return (char*)"BLOCK_INSTR_FILTER";
		case BLOCK_INSTR_MEM_READ:		return (char*)"BLOCK_INSTR_MEM_READ";
		case BLOCK_INSTR_MIN: 			return (char*)"BLOCK_INSTR_MIN";
		case BLOCK_INSTR_MAX: 			return (char*)"BLOCK_INSTR_MAX";
		case BLOCK_INSTR_CLAMP: 		return (char*)"BLOCK_INSTR_CLAMP";
		case BLOCK_INSTR_MACZ: 			return (char*)"BLOCK_INSTR_MACZ";
		case BLOCK_INSTR_MAC: 			return (char*)"BLOCK_INSTR_MAC";
		case BLOCK_INSTR_MOV_ACC: 		return (char*)"BLOCK_INSTR_MOV_ACC";
		case BLOCK_INSTR_MOV_UACC: 		return (char*)"BLOCK_INSTR_MOV_UACC";
		case BLOCK_INSTR_MOV_LACC: 		return (char*)"BLOCK_INSTR_MOV_LACC";
	}
	
	return NULL;
}

char *kest_block_opcode_to_name(uint32_t opcode)
{
	switch (opcode)
	{
		case BLOCK_INSTR_NOP: 			return (char*)"nop";
		case BLOCK_INSTR_LSH: 			return (char*)"lsh";
		case BLOCK_INSTR_RSH: 			return (char*)"rsh";
		case BLOCK_INSTR_ARSH: 			return (char*)"arsh";
		case BLOCK_INSTR_MADD: 			return (char*)"madd";
		case BLOCK_INSTR_ABS: 			return (char*)"abs";
		case BLOCK_INSTR_LUT_READ:		return (char*)"lut";
		case BLOCK_INSTR_DELAY_READ: 	return (char*)"delay_read";
		case BLOCK_INSTR_DELAY_WRITE: 	return (char*)"delay_write";
		case BLOCK_INSTR_MEM_WRITE:		return (char*)"mem_write";
		case BLOCK_INSTR_MEM_READ:		return (char*)"mem_read";
		case BLOCK_INSTR_FILTER:		return (char*)"filter";
		case BLOCK_INSTR_MIN: 			return (char*)"min";
		case BLOCK_INSTR_MAX: 			return (char*)"max";
		case BLOCK_INSTR_CLAMP: 		return (char*)"clamp";
		case BLOCK_INSTR_MACZ: 			return (char*)"macz";
		case BLOCK_INSTR_MAC: 			return (char*)"mac";
		case BLOCK_INSTR_MOV_ACC: 		return (char*)"mov_acc";
		case BLOCK_INSTR_MOV_LACC: 		return (char*)"mov_lacc";
		case BLOCK_INSTR_MOV_UACC: 		return (char*)"mov_uacc";
	}
	
	return "";
}

void print_instruction_format_a(uint32_t instr)
{
	int opcode = range_bits(instr, 5, 0);
	
	int src_a 	 = range_bits(instr, 4, 6);
	int src_a_reg = !!(instr & (1 << 10));
	
	int src_b 	 = range_bits(instr, 4, 11);
	int src_b_reg = !!(instr & (1 << 15));
	
	int src_c 	 = range_bits(instr, 4, 16);
	int src_c_reg = !!(instr & (1 << 20));
	
	int dest = range_bits(instr, 4, 21);
	int shift = range_bits(instr, 5, 25);
	int sat = !!(instr & (1 << 30));
	int no_shift = !!(instr & (1 << 31));
	
	KEST_PRINTF_("%s %s%d %s%d %s%d, c%d (%d%s)",
		kest_block_opcode_to_name(opcode),
			src_a_reg ? "r" : "c", src_a,
			src_b_reg ? "r" : "c", src_b,
			src_c_reg ? "r" : "c", src_c, dest,
			shift, sat ? ", unsat" : "");
}

void print_instruction_format_b(uint32_t instr)
{
	int opcode = range_bits(instr, 5, 0);
	
	int src_a 	 = range_bits(instr, 4, 6);
	int src_a_reg = !!(instr & (1 << 10));
	
	int src_b 	 = range_bits(instr, 4, 11);
	int src_b_reg = !!(instr & (1 << 15));
	
	int no_shift = 0;
	
	int src_c = 0;
	int src_c_reg = 0;
	
	int dest = range_bits(instr, 4, 16);
	
	int res_addr = range_bits(instr, 8, 20);
	
	KEST_PRINTF_("%s %s%d %s%d $%d c%d",
		kest_block_opcode_to_name(opcode),
			src_a_reg ? "r" : "c", src_a,
			src_b_reg ? "r" : "c", src_b,
			res_addr, dest);
}

void print_instruction(uint32_t instr)
{
	int format = !!(instr & (1 << 5));
	
	if (format)
		print_instruction_format_b(instr);
	else
		print_instruction_format_a(instr);
}

int kest_fpga_batch_print(kest_fpga_transfer_batch seq)
{
	int n = seq.len;
	
	KEST_PRINTF_("Reading out FPGA transfer batch %p (length %d)\n", seq.buf, n);
	
	if (!seq.buf)
	{
		KEST_PRINTF_("Buffer is NULL!\n");
	}
	
	if (n < 1)
	{
		KEST_PRINTF_("Batch has no bytes!\n");
	}
	
	int i = 0;
	
	int state = 0;
	int ret_state;
	int skip = 0;
	
	int shift = 0;
	
	uint8_t byte;
	
	int ctr = 0;
	int ctr_2 = 0;
	
	uint8_t reg_no = 0;
	kest_fpga_block_addr_t block = 0;
	uint32_t value = 0;
	int32_t signed_val;
	uint32_t instruction = 0;
	
	while (i < n)
	{
		byte = seq.buf[i];
		
		KEST_PRINTF_("\tByte %s%d: 0x%02X. ", (n > 9 && i < 10) ? " " : "", i, byte);
		
		switch (state)
		{
			case 0: // expecting a command
				switch (byte)
				{
					case COMMAND_BEGIN_PROGRAM:
						KEST_PRINTF_("Command BEGIN_PROGRAM");
						break;
						
					case COMMAND_WRITE_BLOCK_INSTR:
						KEST_PRINTF_("Command WRITE_BLOCK_INSTR");
						state = 1;
						ret_state = 2;
						ctr = 0;
						instruction = 0;
						break;

					case COMMAND_WRITE_BLOCK_REG_0:
						KEST_PRINTF_("Command WRITE_BLOCK_REG_0");
						state = 1;
						ret_state = 4;
						value = 0;
						ctr = 0;
						break;

					case COMMAND_WRITE_BLOCK_REG_1:
						KEST_PRINTF_("Command WRITE_BLOCK_REG_1");
						state = 1;
						ret_state = 4;
						value = 0;
						ctr = 0;
						break;

					case COMMAND_ALLOC_DELAY:
						KEST_PRINTF_("Command ALLOC_DELAY");
						state = 5;
						value = 0;
						ctr = 0;
						ctr_2 = 0;
						shift = 15;
						break;
						
					case COMMAND_END_PROGRAM:
						KEST_PRINTF_("Command END_PROGRAM");
						break;

					case COMMAND_UPDATE_BLOCK_REG_0:
						KEST_PRINTF_("Command UPDATE_BLOCK_REG_0");
						state = 1;
						ret_state = 4;
						value = 0;
						ctr = 0;
						break;

					case COMMAND_UPDATE_BLOCK_REG_1:
						KEST_PRINTF_("Command UPDATE_BLOCK_REG_1");
						state = 1;
						ret_state = 4;
						value = 0;
						ctr = 0;
						break;
						
					case COMMAND_COMMIT_REG_UPDATES:
						KEST_PRINTF_("Command COMMIT_REG_UPDATES");
						break;

					case COMMAND_SET_INPUT_GAIN:
						KEST_PRINTF_("Command SET_INPUT_GAIN");
						state = 4;
						ret_state = 0;
						value = 0;
						ctr = 0;
						shift = 4;
						break;

					case COMMAND_SET_OUTPUT_GAIN:
						KEST_PRINTF_("Command SET_OUTPUT_GAIN");
						state = 4;
						ret_state = 0;
						value = 0;
						ctr = 0;
						shift = 4;
						break;

					case COMMAND_ALLOC_FILTER:
						KEST_PRINTF_("Command COMMAND_ALLOC_FILTER");
						state = 6;
						ret_state = 0;
						value = 0;
						ctr = 0;
						ctr_2 = 0;
						shift = 0;
						break;

					case COMMAND_WRITE_FILTER_COEF:
						KEST_PRINTF_("Command COMMAND_WRITE_FILTER_COEF");
						state = 7;
						ret_state = 0;
						value = 0;
						ctr = 0;
						ctr_2 = 0;
						shift = 0;
						break;

				}
				break;
			
			case 1: // expecting block number
				block = (block << 8) | byte;
				
				if (ctr == KEST_FPGA_BLOCK_ADDR_BYTES - 1)
				{
					KEST_PRINTF_("Block number %d", byte);
					state = ret_state;
					ctr = 0;
				}
				else
				{
					ctr++;
				}
				
				break;
			
			case 2: // expecting instruction
				if (ctr == 3)
				{
					state = 0;
					instruction = (instruction << 8) | byte;
					//kest_printf("Word: %s; ", binary_print_32(instruction));
					print_instruction(instruction);
					if (kest_fpga_block_opcode_format(instruction & IBM(5)))
						shift = 0;
					else
						shift = range_bits(instruction, 5, 25);
				}
				else
				{
					instruction = (instruction << 8) | byte;
					ctr++;
				}
				break;
			
			case 3: // expecting register number then register value
				KEST_PRINTF_("Register %d", byte);
				state = 4;
				break;
			
			case 4: // expecting a value
				if (ctr == KEST_FPGA_DATA_BYTES - 1)
				{
					state = 0;
					
					value = (value << 8) | byte;
					KEST_PRINTF_("Value: %s = %d = %f (in q%d.%d)", binary_print_16(value), value, (float)value / (powf(2.0, 15 - shift)), 1 + shift, 15 - shift);
				}
				else
				{
					value = (value << 8) | byte;
					ctr++;
				}
				break;
			
			case 5: // expecting two 24-bit values. lol!
				if (ctr == 0)
				{
					value = byte;
					ctr++;
				}
				else if (ctr == 2)
				{
					value = (value << 8) | byte;
					KEST_PRINTF_("%s: %s = 0x%06x = %.02f", ctr_2 ? "Delay" : "Size", binary_print_24(value), value, (float)((uint32_t)value) / (powf(2.0, (15 - shift))));
					
					ctr = 0;
					
					if (ctr_2 == 1)
					{
						state = 0;
					}
					else
					{
						shift = 7;
						ctr_2 = 1;
					}
				}
				else
				{
					value = (value << 8) | byte;
					ctr++;
				}
				break;
			
			case 6:
				if (ctr_2 == 0)
				{
					KEST_PRINTF_("Format: q%d.%d", 1 + byte, 17 - byte);
					shift = byte;
					ctr_2++;
				}
				else if (ctr_2 == 1)
				{
					if (ctr == 0)
					{
						value = byte;
						ctr++;
					}
					else
					{
						value = (value << 8) | byte;
						KEST_PRINTF_("Feed-forward: %d", value);
						
						ctr_2++;
						ctr = 0;
					}
				}
				else
				{
					if (ctr == 0)
					{
						value = byte;
						ctr++;
					}
					else
					{
						value = (value << 8) | byte;
						KEST_PRINTF_("Feed-back:    %d", value);
						
						state = 0;
					}
				}
				
				break;
			
			case 7:
				if (ctr_2 == 0)
				{
					KEST_PRINTF_("Filter %d", byte);
					ctr_2++;
					ctr = 0;
					value = 0;
				}
				else if (ctr_2 == 1)
				{
					value = (value << 8) | byte;
					
					if (ctr == 1)
					{
						KEST_PRINTF_("Coefficient %d", value);
						ctr = 0;
						ctr_2++;
						
						signed_val = 0;
					}
					else
					{
						ctr++;
					}
				}
				else
				{
					signed_val = (signed_val << 8) | byte;
					ctr++;
					
					if (ctr == 3)
					{
						if (signed_val & (1u << 23))
						{
							signed_val |= 0xFF000000;
						}
						
						KEST_PRINTF_("Value: %.04f", ((float)signed_val) * pow(2, -(16 - shift)));
						state = 0;
					}
				}
				break;
			
			default:
				KEST_PRINTF_("Unknown :(\n");
				return 1;
		}
		
		KEST_PRINTF_("\n");
		
		i++;
	}
	
	return 0;
}
