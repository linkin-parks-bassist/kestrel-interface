#ifndef KEST_NUMERIC_FORMAT_H_
#define KEST_NUMERIC_FORMAT_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	KEST_NUMERIC_SATURATE,
	KEST_NUMERIC_REJECT
} kest_numeric_overflow;

/* A resolved conversion, retained with the value until it is sent. */
typedef struct {
	uint8_t fractional_bits;
	bool is_unsigned;
	kest_numeric_overflow overflow;
} kest_numeric_format;

int kest_numeric_format_bounds(kest_numeric_format format, int width,
	float *minimum, float *maximum);
int kest_encode_numeric(float value, kest_numeric_format format, int width,
	uint32_t *word);

#endif
