#include "kest_int.h"

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "kest_fixed_point.c";

int kest_numeric_format_bounds(kest_numeric_format format, int width,
	float *minimum, float *maximum)
{
	if (!minimum || !maximum)
		return ERR_NULL_PTR;
	if (width < 1 || width > 24 ||
		format.fractional_bits > width - !format.is_unsigned ||
		format.overflow > KEST_NUMERIC_REJECT)
		return ERR_BAD_ARGS;

	int magnitude_bits = width - !format.is_unsigned;
	float scale = ldexpf(1.0f, format.fractional_bits);
	*minimum = format.is_unsigned ? 0.0f : -ldexpf(1.0f, magnitude_bits) / scale;
	*maximum = (ldexpf(1.0f, magnitude_bits) - 1.0f) / scale;
	return NO_ERROR;
}

int kest_encode_numeric(float value, kest_numeric_format format, int width,
	uint32_t *word)
{
	if (!word)
		return ERR_NULL_PTR;
	float minimum, maximum;
	int result = kest_numeric_format_bounds(format, width, &minimum, &maximum);
	if (result != NO_ERROR)
		return result;
	if (isnan(value))
		return ERR_VALUE_OUT_OF_BOUNDS;
	if (value < minimum || value > maximum)
	{
		if (format.overflow == KEST_NUMERIC_REJECT)
			return ERR_VALUE_OUT_OF_BOUNDS;
		value = fminf(maximum, fmaxf(minimum, value));
	}
	int64_t encoded = llrintf(ldexpf(value, format.fractional_bits));
	*word = (uint32_t)encoded & ((UINT32_C(1) << width) - 1);
	return NO_ERROR;
}

int32_t float_to_q_nminus1_filter_width(float x, int shift)
{
	if (shift < 0 || shift > KEST_FPGA_FILTER_WIDTH - 1) return 0;
	
	int n = (KEST_FPGA_FILTER_WIDTH - 1) - shift;

    float scale = ldexpf(1.0f, n);

    float max =  (float)((1 << (KEST_FPGA_FILTER_WIDTH - 1)) - 1) / scale;
    float min = -(float)(1  << (KEST_FPGA_FILTER_WIDTH - 1))      / scale;

    if (x > max) x = max;
    if (x < min) x = min;

    return lrintf(x * scale);
}

kest_fpga_sample_t float_to_q_nminus1(float x, int shift)
{
	if (shift < 0 || shift > KEST_FPGA_DATA_WIDTH - 1) return 0;
	
    int n = (KEST_FPGA_DATA_WIDTH - 1) - shift;

    float scale = (float)(1 << n);

    float max =  (float)((1 << (KEST_FPGA_DATA_WIDTH - 1)) - 1) / scale;
    float min = -(float)(1  << (KEST_FPGA_DATA_WIDTH - 1))      / scale;

    if (x > max) x = max;
    if (x < min) x = min;

    return (kest_fpga_sample_t)lrintf(x * scale);
}


int16_t float_to_q15(float x)
{
	if (x >= 0.999969482421875f) return  32767;
    if (x <= -1.0f)              return -32768;
    
    return (int16_t)lrintf(x * 32768.0f);
}

int kest_expression_compute_format(kest_expression *expr, kest_scope *scope, int fmax, int width)
{
	KEST_PRINTF("kest_expression_compute_format([%s])\n", kest_expression_to_string(expr));
	
	fflush(stdout);
	
	if (!expr) return 0;
	
	float min = kest_expression_compute_min(expr, scope);
	float max = kest_expression_compute_max(expr, scope);
	
	kest_interval range = kest_interval_ab(min, max);
	
	kest_string string;
	kest_string_init(&string);
	char *str;
	
	kest_string_appendf(&string, "Expression \"%s\" has range [%.06f, %.06f], so ", kest_expression_to_string(expr), min, max);
	
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
	
	kest_string_appendf(&string, "therefore, it requires format q%d.%d\n", 1 + format, width - 1 - format);
	
	str = kest_string_to_native(&string);
	KEST_PRINTF(str);
	kest_free(str);
	kest_string_destroy(&string);
	
	return format;
}


int kest_filter_compute_format(kest_filter *filter, kest_scope *scope)
{
	KEST_PRINTF("Compute format for filter %p\n", filter);
	if (!filter)
		return ERR_NULL_PTR;
	
	int format = 0;
	int current_format;
	
	for (int i = 0; i < filter->coefs.count; i++)
	{
		current_format = kest_expression_compute_format(filter->coefs.entries[i], scope, KEST_FPGA_DATA_WIDTH, KEST_FPGA_DATA_WIDTH);
		format = (current_format > format) ? current_format : format;
	}
	
	KEST_PRINTF("Filter format: q%d.%d\n", 1 + format, KEST_FPGA_FILTER_WIDTH - format);
	filter->format = format;
	
	return NO_ERROR;
}
