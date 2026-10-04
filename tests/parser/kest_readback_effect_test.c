#include "kest_test.h"

KEST_TEST(kest_test_readback_effect_descriptor)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/readback.eff");
    assert(desc != NULL);
    assert(strcmp(desc->name, "Readback Test") == 0);
    assert(desc->resources && !desc->resources->next);
    kest_dsp_resource *res = desc->resources->data;
    assert(res && res->type == KEST_DSP_RESOURCE_MEM && res->mem_size == 1);
    assert(strcmp(res->name, "probe") == 0);
    kest_mem_slot *mem = res->data;
    assert(mem && mem->read_enable && mem->read_period_ms == 10);
    assert(atomic_load(&mem->value) == 0);
    assert(desc->blocks && desc->blocks->next && !desc->blocks->next->next);
    kest_block *load = desc->blocks->data;
    assert(load->instr == BLOCK_INSTR_MADD && load->dest == 1);
    assert(load->arg_a.type == BLOCK_OPERAND_TYPE_R && load->arg_a.addr == 0);
    assert(load->reg_0.active && load->reg_0.expr);
    kest_scope *scope = kest_eff_desc_create_scope(desc);
    assert(scope != NULL);
    assert(kest_expression_evaluate(load->reg_0.expr, scope) == 0.25f);
    kest_block *write = desc->blocks->next->data;
    assert(write->instr == BLOCK_INSTR_MEM_WRITE);
    assert(write->res == res);
    assert(write->arg_a.type == BLOCK_OPERAND_TYPE_C && write->arg_a.addr == 1);
}

KEST_TEST(kest_test_parsed_svf_uses_descriptor_send_policy)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/svf.eff");
    assert(desc && desc->blocks && desc->blocks->next);
    kest_scope *scope = kest_eff_desc_create_scope(desc);
    assert(scope);
    kest_block *svf = desc->blocks->data;
    assert(svf->desc == kest_instr_name_to_desc("svf"));
    assert(svf->reg_0.expr->type == KEST_EXPR_MAX);
    assert(svf->reg_0.format.fractional_bits == 15);
    assert(kest_expression_compute_min(svf->reg_0.expr, scope) == 0);
    assert(kest_expression_evaluate(svf->reg_0.expr, scope) == 0.25f);
    assert(svf->reg_1.format.fractional_bits == 13 && svf->shift == 2);
    kest_block *channel_svf = desc->blocks->next->next->data;
    assert(channel_svf->arg_b.type == BLOCK_OPERAND_TYPE_C);
    assert(channel_svf->arg_b.addr == 1 && channel_svf->shift == 2);
}

KEST_TEST(kest_test_three_expression_arguments_are_rejected)
{
    assert(kest_read_eff_desc_from_file("tests/fixtures/too-many-registers.eff") == NULL);
}

KEST_TEST(kest_test_parsed_explicit_shift_fields)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/shifts.eff");
    assert(desc);
    kest_block_pll *node = desc->blocks;
    const char *names[] = {"arsh", "lsh", "rsh"};
    for (int i = 0; i < 3; i++, node = node->next)
    {
        assert(node && node->data->desc == kest_instr_name_to_desc(names[i]));
        assert(node->data->shift == i + 1);
        assert(node->data->reg_0.format.fractional_bits == 15);
    }
    assert(!node);
}

KEST_TEST(kest_test_polynomial_resources_have_programmable_filter_payloads)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/polynomial-state.eff");
    assert(desc && desc->resources && desc->resources->next);
    const int counts[] = {3, 1};
    kest_dsp_resource_pll *node = desc->resources;
    for (int i = 0; i < 2; i++, node = node->next)
    {
        assert(node && node->data->type == KEST_DSP_RESOURCE_FILTER);
        kest_filter *poly = node->data->data;
        assert(poly && poly->feed_forward == counts[i] && poly->feed_back == 0);
        assert(poly->coefs.count == counts[i]);
    }
    assert(!node);
    assert(desc->blocks->data->instr == BLOCK_INSTR_POLY);
}

KEST_TEST(kest_test_malformed_polynomial_reports_error_without_crashing)
{
    assert(kest_read_eff_desc_from_file("tests/fixtures/polynomial-invalid.eff") == NULL);
}
