#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "m_fixed_point.c";

int32_t float_to_q_nminus1_18bit(float x, int shift)
{
	if (shift < 0 || shift > 17) return 0;
	
	int n = (18 - 1) - shift;

    float scale = ldexpf(1.0f, n);

    float max =  (float)((1 << (18 - 1)) - 1) / scale;
    float min = -(float)(1  << (18 - 1))      / scale;

    if (x > max) x = max;
    if (x < min) x = min;

    return lrintf(x * scale);
}

m_fpga_sample_t float_to_q_nminus1(float x, int shift)
{
	if (shift < 0 || M_FPGA_DATA_WIDTH - 1) return 0;
	
    int n = (M_FPGA_DATA_WIDTH - 1) - shift;

    float scale = (float)(1 << n);

    float max =  (float)((1 << (M_FPGA_DATA_WIDTH - 1)) - 1) / scale;
    float min = -(float)(1  << (M_FPGA_DATA_WIDTH - 1))      / scale;

    if (x > max) x = max;
    if (x < min) x = min;

    return (m_fpga_sample_t)lrintf(x * scale);
}


int16_t float_to_q15(float x)
{
	if (x >= 0.999969482421875f) return  32767;
    if (x <= -1.0f)              return -32768;
    
    return (int16_t)lrintf(x * 32768.0f);
}

int m_expression_compute_format(m_expression *expr, m_expr_scope *scope, int fmax, int width)
{
	if (!expr) return 0;
	
	m_interval range = m_expression_compute_range(expr, scope);
	float min = range.a;
	float max = range.b;
	
	m_string string;
	m_string_init(&string);
	char *str;
	
	m_string_appendf(&string, "Expression \"%s\" has range [%.06f, %.06f], so ", m_expression_to_string(expr), min, max);
	
	int format = 0;
	float p2;
	float p2_r;
	
	while (format < fmax)
	{
		p2 = ldexpf(1.0, format);
		p2_r = ldexp(1.0, -(width - 1 - format));
		if (min >= -p2 && max < p2 - p2_r)
			break;
		format++;
	}
	
	m_string_appendf(&string, "therefore, it requires format q%d.%d\n", 1 + format, width - 1 - format);
	
	str = m_string_to_native(&string);
	M_PRINTF(str);
	m_free(str);
	m_string_destroy(&string);
	
	return format;
}


int m_filter_compute_format(m_filter *filter, m_expr_scope *scope)
{
	M_PRINTF("Compute format for filter %p\n", filter);
	if (!filter)
		return ERR_NULL_PTR;
	
	int format = 0;
	int current_format;
	
	for (int i = 0; i < filter->coefs.count; i++)
	{
		current_format = m_expression_compute_format(filter->coefs.entries[i], scope, 8, M_FPGA_FILTER_WIDTH);
		format = (current_format > format) ? current_format : format;
	}
	
	M_PRINTF("Filter format: q%d.%d\n", 1 + format, 18 - format);
	filter->format = format;
	
	return NO_ERROR;
}
