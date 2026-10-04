#include "kest_test.h"

#define FORMAT_BIT(n) (UINT32_C(1) << (n))

static void put_expression(kest_block *block, int argument, int reg, kest_expression *expr)
{
    kest_block_operand *op = argument == 0 ? &block->arg_a :
        (argument == 1 ? &block->arg_b : &block->arg_c);
    *op = (kest_block_operand){BLOCK_OPERAND_TYPE_R, reg};
    kest_block_reg_val *value = reg ? &block->reg_1 : &block->reg_0;
    value->active = 1;
    value->expr = expr;
}

static int matching_field(const kest_block *block, const kest_numeric_format formats[3], int *field)
{
    if (formats[0].fractional_bits != formats[1].fractional_bits)
        return ERR_BAD_ARGS;
    *field = 15 - formats[0].fractional_bits;
    return NO_ERROR;
}

KEST_TEST(kest_test_joint_formats_choose_a_compatible_tuple)
{
    const kest_instr_numeric_policy policy = {
        .args = {
            {.signed_formats = FORMAT_BIT(15) | FORMAT_BIT(14), .overflow = KEST_NUMERIC_REJECT},
            {.signed_formats = FORMAT_BIT(15) | FORMAT_BIT(14), .overflow = KEST_NUMERIC_REJECT},
            {.signed_formats = FORMAT_BIT(15)}
        },
        .resolve_field = matching_field
    };
    const kest_asm_instr_desc desc = {.numeric = &policy};
    kest_block block;
    assert(kest_init_block_from_instr_desc(&block, &desc) == NO_ERROR);
    kest_expression a = kest_expression_const(0.25f), b = kest_expression_const(1.25f);
    put_expression(&block, 0, 0, &a);
    put_expression(&block, 1, 1, &b);
    assert(kest_resolve_block_formats(&block, NULL) == NO_ERROR);
    assert(block.reg_0.format.fractional_bits == 14);
    assert(block.reg_1.format.fractional_bits == 14);
    assert(block.shift == 1);
    /* A previously resolved result is not changed by an impossible instance. */
    assert(kest_expr_init_const(&b, 3) == NO_ERROR);
    assert(kest_resolve_block_formats(&block, NULL) == ERR_VALUE_OUT_OF_BOUNDS);
    assert(block.shift == 1 && block.reg_0.format.fractional_bits == 14);
    /* One physical register cannot satisfy conflicting consumer formats. */
    const kest_instr_numeric_policy conflict = {
        .args = {
            {.signed_formats = FORMAT_BIT(15)},
            {.signed_formats = FORMAT_BIT(14)},
            {.signed_formats = FORMAT_BIT(15)}
        },
        .resolve_field = kest_instr_name_to_desc("nop")->numeric->resolve_field
    };
    const kest_asm_instr_desc conflict_desc = {.numeric = &conflict};
    block.desc = &conflict_desc;
    block.arg_b = block.arg_a;
    assert(kest_resolve_block_formats(&block, NULL) == ERR_VALUE_OUT_OF_BOUNDS);

}

KEST_TEST(kest_test_aliases_use_actual_constant_register_formats)
{
    const char *names[] = {"mov", "add", "sub"};
    for (int i = 0; i < 3; i++)
    {
        kest_block block;
        assert(kest_init_block_from_instr_desc(&block, kest_instr_name_to_desc(names[i])) == NO_ERROR);
        block.arg_b = i == 2 ? operand_const_minus_one() : operand_const_one();
        block.arg_c = operand_const_zero();
        assert(kest_resolve_block_formats(&block, NULL) == NO_ERROR);
        assert(block.shift == (i == 2 ? 0 : 1));
        assert(block.desc == kest_instr_name_to_desc(names[i]));
    }
    kest_block unsigned_mac;
    assert(kest_init_block_from_instr_desc(&unsigned_mac, kest_instr_name_to_desc("umac")) == NO_ERROR);
    kest_expression value = kest_expression_const(1.5f);
    put_expression(&unsigned_mac, 0, 0, &value);
    assert(kest_resolve_block_formats(&unsigned_mac, NULL) == NO_ERROR);
    assert(unsigned_mac.reg_0.format.is_unsigned && unsigned_mac.reg_0.format.fractional_bits == 15);
    kest_fpga_transfer_batch batch;
    assert(kest_fpga_transfer_batch_init(&batch) == NO_ERROR);
    assert(kest_fpga_batch_append_block_regs(&batch, &unsigned_mac, NULL, 0) == NO_ERROR);
    assert(batch.len == 5 && batch.buf[3] == 0xc0 && batch.buf[4] == 0);
    kest_free_fpga_transfer_batch(batch);

}

static kest_expression *send_erf_bipolar(kest_expression *source)
{
    kest_expression *erf_expr = kest_expr_new_unary(KEST_EXPR_ERF, source);
    kest_expression *twice = kest_expr_new_binary(KEST_EXPR_MUL, &kest_expression_two, erf_expr);
    return kest_expr_new_binary(KEST_EXPR_SUB, twice, &kest_expression_one);
}

KEST_TEST(kest_test_send_transform_range_dependencies_and_wire_updates)
{
    kest_scope scope;
    assert(kest_scope_init(&scope) == NO_ERROR);
    kest_parameter parameter = {.name_internal = "x", .min = 0, .max = 100, .value = 0.25f};
    kest_expression lo = kest_expression_const(0), hi = kest_expression_const(100);
    parameter.min_expr = &lo;
    parameter.max_expr = &hi;
    assert(kest_scope_add_param(&scope, &parameter) == NO_ERROR);
    kest_expression *source = kest_expr_new_reference("x");
    const kest_instr_numeric_policy policy = {
        .args = {
            {.signed_formats = FORMAT_BIT(15) | FORMAT_BIT(14), .send_expression = send_erf_bipolar},
            {.signed_formats = FORMAT_BIT(15)},
            {.signed_formats = FORMAT_BIT(15)}
        },
        .resolve_field = matching_field
    };
    const kest_asm_instr_desc desc = {.numeric = &policy};
    kest_block block;
    assert(kest_init_block_from_instr_desc(&block, &desc) == NO_ERROR);
    put_expression(&block, 0, 0, policy.args[0].send_expression(source));
    assert(kest_expression_compute_max(block.reg_0.expr, &scope) <= 1);
    assert(kest_resolve_block_formats(&block, &scope) == NO_ERROR);
    assert(block.reg_0.format.fractional_bits == 15);
    assert(kest_scope_add_block_reg_dependencies(&scope, block.reg_0.expr, 7, 0, block.reg_0.format) == NO_ERROR);
    kest_scope_entry *entry = kest_scope_lookup(&scope, "x");
    assert(entry && entry->dependents.count == 1);
    assert(entry->dependents.entries[0].encoding.fractional_bits == 15);

    for (int i = 0; i < 2; i++)
    {
        parameter.value = i ? 0.75f : 0.25f;
        float value = kest_expression_evaluate(block.reg_0.expr, &scope);
        uint32_t word;
        assert(kest_encode_numeric(value, block.reg_0.format, 16, &word) == NO_ERROR);
        assert(word == (uint16_t)lrintf((2 * erff(parameter.value) - 1) * 32768));
        kest_fpga_transfer_batch batch = {0};
        assert(kest_fpga_transfer_batch_init(&batch) == NO_ERROR);
        assert(kest_fpga_batch_append_block_regs(&batch, &block, &scope, 7) == NO_ERROR);
        assert(batch.len == 5);
        assert(batch.buf[3] == (word >> 8) && batch.buf[4] == (word & 255));
        kest_free_fpga_transfer_batch(batch);
        batch = (kest_fpga_transfer_batch){0};
        assert(kest_fpga_transfer_batch_init(&batch) == NO_ERROR);
        kest_fpga_command command = kest_fpga_command_update_block_reg_0(7, value, entry->dependents.entries[0].encoding);
        assert(kest_fpga_command_append_encoded(command, &batch) == NO_ERROR);
        assert(batch.len == 5 && batch.buf[0] == COMMAND_UPDATE_BLOCK_REG_0);
        assert(batch.buf[3] == (word >> 8) && batch.buf[4] == (word & 255));
        kest_free_fpga_transfer_batch(batch);
    }
}

KEST_TEST(kest_test_svf_cutoff_format_is_independent_of_declared_range)
{
    kest_scope scope;
    assert(kest_scope_init(&scope) == NO_ERROR);
    kest_expression minimum = kest_expression_const(0), maximum = kest_expression_const(0.5f);
    kest_parameter cutoff = {.name_internal = "cutoff", .value = 0.25f,
        .min_expr = &minimum, .max_expr = &maximum};
    assert(kest_scope_add_param(&scope, &cutoff) == NO_ERROR);
    const kest_asm_instr_desc *desc = kest_instr_name_to_desc("svf");
    kest_block block;
    assert(kest_init_block_from_instr_desc(&block, desc) == NO_ERROR);
    put_expression(&block, 1, 0, desc->numeric->args[1].send_expression(kest_expr_new_reference("cutoff")));
    kest_expression damping = kest_expression_const(1.25f);
    put_expression(&block, 2, 1, &damping);
    const float limits[] = {0.5f, 1, 2};
    for (int i = 0; i < 3; i++)
    {
        assert(kest_expr_init_const(&maximum, limits[i]) == NO_ERROR);
        assert(kest_resolve_block_formats(&block, &scope) == NO_ERROR);
        assert(block.reg_0.format.fractional_bits == 15);
        assert(block.reg_1.format.fractional_bits == 14 && block.shift == 1);
        uint32_t word;
        assert(kest_encode_numeric(kest_expression_evaluate(block.reg_0.expr, &scope), block.reg_0.format, 16, &word) == NO_ERROR);
        assert(word == 8192);
    }
}

KEST_TEST(kest_test_saturation_fallback_and_explicit_field)
{
    kest_expression large = kest_expression_const(1000);
    kest_block block;
    assert(kest_init_block_from_instr_desc(&block, kest_instr_name_to_desc("mul")) == NO_ERROR);
    put_expression(&block, 0, 0, &large);
    assert(kest_resolve_block_formats(&block, NULL) == NO_ERROR);
    assert(block.reg_0.format.fractional_bits == 7);
    assert(block.shift == 8);
    kest_expression second = kest_expression_const(1000);
    put_expression(&block, 1, 1, &second);
    assert(kest_resolve_block_formats(&block, NULL) == NO_ERROR);
    assert(block.shift == 15);
    block.arg_b = (kest_block_operand){BLOCK_OPERAND_TYPE_C, 0};
    block.reg_1.active = 0;
    assert(kest_expr_init_const(&large, 0.25f) == NO_ERROR);
    assert(kest_resolve_block_formats(&block, NULL) == NO_ERROR);
    assert(block.reg_0.format.fractional_bits == 15 && block.shift == 0);
    assert(kest_init_block_from_instr_desc(&block, kest_instr_name_to_desc("arsh")) == NO_ERROR);
    put_expression(&block, 0, 0, &large);
    block.shift = 5;
    assert(kest_resolve_block_formats(&block, NULL) == NO_ERROR);
    assert(block.shift == 5 && block.reg_0.format.fractional_bits == 15);
    block.shift = 16;
    assert(kest_resolve_block_formats(&block, NULL) == ERR_VALUE_OUT_OF_BOUNDS);
}

KEST_TEST(kest_test_rejected_conversion_propagates_and_discards_tx)
{
    kest_expression value = kest_expression_const(2);
    kest_block block;
    assert(kest_init_block_from_instr_desc(&block, kest_instr_name_to_desc("mov")) == NO_ERROR);
    put_expression(&block, 0, 0, &value);
    block.reg_0.format = (kest_numeric_format){15, false, KEST_NUMERIC_REJECT};
    kest_fpga_transfer_batch batch;
    assert(kest_fpga_transfer_batch_init(&batch) == NO_ERROR);
    kest_eff_resource_report resources = empty_m_eff_resource_report();
    assert(kest_fpga_batch_append_block(&batch, &block, &resources, NULL, 0) == ERR_VALUE_OUT_OF_BOUNDS);
    batch.len = 0;
    kest_fpga_command command = kest_fpga_command_update_block_reg_0(0, 2, block.reg_0.format);
    assert(kest_fpga_command_append_encoded(command, &batch) == ERR_VALUE_OUT_OF_BOUNDS);
    kest_free_fpga_transfer_batch(batch);
    kest_updater_state updater;
    assert(kest_updater_state_init(&updater) == NO_ERROR);
    assert(kest_fpga_command_list_append(&updater.cmds, command) == NO_ERROR);
    assert(kest_updater_generate_tx_batch(&updater) == ERR_VALUE_OUT_OF_BOUNDS);
    assert(updater.batch.len == 0);
    kest_updater_state_destroy(&updater);
}

KEST_TEST(kest_test_erf_and_log10_upper_ranges)
{
    kest_expression x = kest_expression_const(100);
    kest_expression *logarithm = kest_expr_new_unary(KEST_EXPR_LOG10, &x);
    kest_expression *error_function = kest_expr_new_unary(KEST_EXPR_ERF, &x);
    assert(fabsf(kest_expression_compute_max(logarithm, NULL) - 2) < 1e-6f);
    assert(fabsf(kest_expression_compute_max(error_function, NULL) - 1) < 1e-6f);
}

KEST_TEST(kest_test_subtraction_range_uses_rhs_upper_bound)
{
    kest_scope scope;
    assert(kest_scope_init(&scope) == NO_ERROR);
    kest_expression lo = kest_expression_const(0.1f), hi = kest_expression_const(0.9f);
    kest_parameter p = {.name_internal = "p", .min_expr = &lo, .max_expr = &hi, .value = 0.5f};
    assert(kest_scope_add_param(&scope, &p) == NO_ERROR);
    kest_expression *difference = kest_expr_new_binary(KEST_EXPR_SUB, &kest_expression_one,
                                                       kest_expr_new_reference("p"));
    assert(fabsf(kest_expression_compute_min(difference, &scope) - 0.1f) < 1e-6f);
    assert(fabsf(kest_expression_compute_max(difference, &scope) - 0.9f) < 1e-6f);
    kest_block block;
    assert(kest_init_block_from_instr_desc(&block, kest_instr_name_to_desc("mul")) == NO_ERROR);
    put_expression(&block, 1, 0, difference);
    assert(kest_resolve_block_formats(&block, &scope) == NO_ERROR);
    assert(block.reg_0.format.fractional_bits == 15);
}
