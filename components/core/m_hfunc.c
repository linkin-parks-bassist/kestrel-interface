#include <stdint.h>
#include <math.h>

#include "m_hfunc.h"

char binary_print_buffer[35];

char *binary_print_n(uint32_t x, int n)
{
	binary_print_buffer[0] = '0';
	binary_print_buffer[1] = 'b';
	
	int i = 0;
	while (i < n && i < 32)
	{
		binary_print_buffer[i+2] = '0' + ((x >> (n - i - 1)) & 1);
		i++;
	}
	
	binary_print_buffer[i+2] = 0;
	
	return binary_print_buffer;
}

char *binary_print_8 (uint8_t x)
{
	return binary_print_n(x, 8);
}

char *binary_print_16(uint16_t x)
{
	
	return binary_print_n(x, 16);
}


char *binary_print_24(uint32_t x)
{
	return binary_print_n(x, 24);
}

char *binary_print_32(uint32_t x)
{
	return binary_print_n(x, 32);
}

int format_float(char *buf, float val, int max_len)
{
	if (!buf) return 0;

	char tmp[10];
	int i = 0;
	
	// Handle negative numbers
	int is_neg = (val < 0);
	if (is_neg)
		val = -val;

	// Multiply and round to 2 decimal places
	int scaled 	  = (int)roundf(val * 100.0f);
	int int_part  = scaled / 100;
	int frac_part = scaled % 100;

	// Write manually to buffer
	int pos = 0;

	if (is_neg)
		buf[pos++] = '-';

	if (int_part == 0)
	{
		buf[pos++] = '0';
	}
	else
	{
		while (int_part > 0)
		{
			tmp[i++] = '0' + (int_part % 10);
			int_part /= 10;
		}
		
		while (i-- && pos < max_len - 1)
		{
			buf[pos++] = tmp[i];
		}
		
		if (pos == max_len - 1)
		{
			buf[pos] = 0;
			return pos;
		}
	}

	tmp[0] = (pos < 5) ? '.' : 0;
	tmp[1] = (pos < 5) ? '0' + (frac_part / 10) : 0;
	tmp[2] = (pos < 4) ? '0' + (frac_part % 10) : 0;
	tmp[3] = 0;
	
	for (i = 0; i < 4 && pos < max_len - 1 && tmp[i]; i++)
		buf[pos++] = tmp[i];

	buf[pos] = 0;
	
	return pos - 1;
}
