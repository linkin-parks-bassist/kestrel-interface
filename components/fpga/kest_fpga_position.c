#include "kest_int.h"

int kest_effect_fpga_position_init(kest_effect_fpga_position *pos)
{
	if (!pos)
		return ERR_NULL_PTR;
	
	pos->block_start = 0;
	pos->filter_start = 0;
	
	return NO_ERROR;
}


int kest_effect_fpga_position_resolve_block(kest_effect_fpga_position *pos, int block)
{
	if (!pos)
		return block;
	
	return pos->block_start + block;
}

int kest_effect_fpga_position_resolve_mem(kest_effect_fpga_position *pos, int addr)
{
	if (!pos)
		return addr;
	
	return pos->mem_start + addr;
}

int kest_effect_fpga_position_resolve_filter(kest_effect_fpga_position *pos, int handle)
{
	if (!pos)
		return handle;
	
	return pos->filter_start + handle;
}

int kest_effect_fpga_position_resolve_delay(kest_effect_fpga_position *pos, int handle)
{
	if (!pos)
		return handle;
	
	return pos->delay_start + handle;
}

kest_effect_fpga_position kest_fpga_position_start()
{
	kest_effect_fpga_position start;
	
	start.block_start 	= 0;
	start.filter_start 	= 0;
	start.mem_start 	= 0;
	start.delay_start 	= 0;
	
	return start;
}
