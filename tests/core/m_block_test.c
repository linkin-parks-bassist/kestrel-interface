#include "m_test.h"

M_TEST(m_test_operand_const_zero)
{
    m_block_operand op = operand_const_zero();

    assert(op.type == BLOCK_OPERAND_TYPE_R);
    assert(op.addr == ZERO_REGISTER_ADDR);
}


M_TEST(m_test_operand_const_one)
{
    m_block_operand op = operand_const_one();

    assert(op.type == BLOCK_OPERAND_TYPE_R);
    assert(op.addr == POS_ONE_REGISTER_ADDR);
}


M_TEST(m_test_operand_const_minus_one)
{
    m_block_operand op = operand_const_minus_one();

    assert(op.type == BLOCK_OPERAND_TYPE_R);
    assert(op.addr == NEG_ONE_REGISTER_ADDR);
}


M_TEST(m_test_init_block_null)
{
    int rc = m_init_block(NULL);

    assert(rc == ERR_NULL_PTR);
}


M_TEST(m_test_init_block_defaults)
{
    m_block block;

    int rc = m_init_block(&block);

    assert(rc == NO_ERROR);

    /* instruction */
    assert(block.instr == BLOCK_INSTR_NOP);

    /* operands */
    assert(block.arg_a.type == BLOCK_OPERAND_TYPE_C);
    assert(block.arg_a.addr == 0);

    assert(block.arg_b.type == BLOCK_OPERAND_TYPE_C);
    assert(block.arg_b.addr == 0);

    assert(block.arg_c.type == BLOCK_OPERAND_TYPE_C);
    assert(block.arg_c.addr == 0);

    /* destination */
    assert(block.dest == 0);

    /* register values */
    assert(block.reg_0.format == 0);
    assert(block.reg_0.active == 0);
    assert(block.reg_0.expr == NULL);

    assert(block.reg_1.format == 0);
    assert(block.reg_1.active == 0);
    assert(block.reg_1.expr == NULL);

    /* shift configuration */
    assert(block.shift == 0);
    assert(block.shift_set == 0);

    /* saturation */
    assert(block.saturate_disable == 0);

    /* resource pointer */
    assert(block.res == NULL);
}


M_TEST(m_test_init_block_overwrites_existing_values)
{
    m_block block;

    /* fill with garbage values */
    memset(&block, 0xFF, sizeof(block));

    int rc = m_init_block(&block);

    assert(rc == NO_ERROR);

    /* confirm it resets everything */
    assert(block.instr == BLOCK_INSTR_NOP);

    assert(block.arg_a.type == BLOCK_OPERAND_TYPE_C);
    assert(block.arg_a.addr == 0);

    assert(block.arg_b.type == BLOCK_OPERAND_TYPE_C);
    assert(block.arg_b.addr == 0);

    assert(block.arg_c.type == BLOCK_OPERAND_TYPE_C);
    assert(block.arg_c.addr == 0);

    assert(block.reg_0.active == 0);
    assert(block.reg_1.active == 0);

    assert(block.shift == 0);
    assert(block.shift_set == 0);

    assert(block.res == NULL);
}
