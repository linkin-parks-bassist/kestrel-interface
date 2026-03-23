#include "kest_int.h"

int kest_effect_fpga_position_init(kest_effect_fpga_position *pos)
{
	if (!pos)
		return ERR_NULL_PTR;
	
	pos->block_start = 0;
	pos->filter_start = 0;
	
	return NO_ERROR;
}
