#ifndef KEST_FIXED_POINT_H_
#define KEST_FIXED_POINT_H_

// Floating point -> fixed point format conversion
kest_fpga_sample_t float_to_q_nminus1(float x, int shift);
int16_t float_to_q15(float x);

int32_t float_to_q_nminus1_18bit(float x, int shift);

int kest_expression_compute_format(kest_expression *expr, kest_scope *scope, int fmax, int width);

int kest_filter_compute_format(kest_filter *filter, kest_scope *scope);

#endif
