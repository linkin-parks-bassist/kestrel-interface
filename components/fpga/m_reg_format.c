#include <stdio.h>

#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 1
#endif

static const char *FNAME = "m_reg_format.c";

int m_compute_register_formats(m_block_pll *blocks, m_expr_scope *scope)
{
	M_PRINTF("m_compute_register_formats\n");
	if (!blocks)
	{
		M_PRINTF("m_compute_register_formats: no blocks!\n");
		return NO_ERROR;
	}
	
	m_block_pll *current = blocks;
	
	m_interval range;
	float min, max;
	float abs_min, abs_max;
	float max_abs;
	int format_0, format_1;
	int shift_set;
	float p2;
	
	m_string string;
	m_string_init(&string);
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
			m_string_appendf(&string, "m_compute_register_formats(current->data = %p), shift_set = %d, reg_0_active: %d, reg_1_active: %d\n",
				current->data, shift_set, current->data->reg_0.active, current->data->reg_1.active);
			*/
			if (current->data->reg_0.active)
			{
				if (!shift_set && current->data->reg_0.expr)
					format_0 = m_expression_compute_format(current->data->reg_0.expr, scope, 8, M_FPGA_DATA_WIDTH);
				
				current->data->reg_0.format = format_0;
			}
			
			if (current->data->reg_1.active)
			{
				if (!shift_set && current->data->reg_1.expr)
					format_1 = m_expression_compute_format(current->data->reg_1.expr, scope, 8, M_FPGA_DATA_WIDTH);
				
				current->data->reg_1.format = format_1;
			}
			
			current->data->shift = format_0 + format_1;
			/*
			str = m_string_to_native(&string);
			M_PRINTF("%s", str);
			m_free(str);
			m_string_drain(&string);
			*/
		}
		
		current = current->next;
		i++;
	}
	
	m_string_destroy(&string);
	
	M_PRINTF("m_compute_register_formats done\n");
	
	return NO_ERROR;
}
