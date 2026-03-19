#include <stdio.h>

#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 1
#endif

static const char *FNAME = "kest_reg_format.c";

int kest_compute_register_formats(kest_block_pll *blocks, kest_expr_scope *scope)
{
	KEST_PRINTF("kest_compute_register_formats\n");
	if (!blocks)
	{
		KEST_PRINTF("kest_compute_register_formats: no blocks!\n");
		return NO_ERROR;
	}
	
	kest_block_pll *current = blocks;
	
	kest_interval range;
	float min, max;
	float abs_min, abs_max;
	float max_abs;
	int format_0, format_1;
	int shift_set;
	float p2;
	
	kest_string string;
	kest_string_init(&string);
	char *str = NULL;
	
	int i = 0;
	while (current)
	{
		if (current->data)
		{
			shift_set = current->data->shift_set;
			format_0 = 0;
			format_1 = 0;
			
			/*
			kest_string_appendf(&string, "kest_compute_register_formats(current->data = %p), shift_set = %d, reg_0_active: %d, reg_1_active: %d\n",
				current->data, shift_set, current->data->reg_0.active, current->data->reg_1.active);
			*/
			if (current->data->reg_0.active)
			{
				if (!shift_set && current->data->reg_0.expr)
					format_0 = kest_expression_compute_format(current->data->reg_0.expr, scope, 8, KEST_FPGA_DATA_WIDTH);
				
				current->data->reg_0.format = format_0;
			}
			
			if (current->data->reg_1.active)
			{
				if (!shift_set && current->data->reg_1.expr)
					format_1 = kest_expression_compute_format(current->data->reg_1.expr, scope, 8, KEST_FPGA_DATA_WIDTH);
				
				current->data->reg_1.format = format_1;
			}
			
			current->data->shift = format_0 + format_1;
			/*
			str = kest_string_to_native(&string);
			KEST_PRINTF("%s", str);
			kest_free(str);
			kest_string_drain(&string);
			*/
		}
		
		current = current->next;
		i++;
	}
	
	kest_string_destroy(&string);
	
	KEST_PRINTF("kest_compute_register_formats done\n");
	
	return NO_ERROR;
}
