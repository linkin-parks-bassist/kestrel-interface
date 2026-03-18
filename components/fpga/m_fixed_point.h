#ifndef M_FIXED_POINT_H_
#define M_FIXED_POINT_H_

// Floating point -> fixed point format conversion
m_fpga_sample_t float_to_q_nminus1(float x, int shift);
int16_t float_to_q15(float x);

int32_t float_to_q_nminus1_18bit(float x, int shift);

int m_expression_compute_format(m_expression *expr, m_expr_scope *scope, int fmax, int width);

int m_filter_compute_format(m_filter *filter, m_expr_scope *scope);

#endif
