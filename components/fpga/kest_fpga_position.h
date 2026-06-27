#ifndef KEST_FPGA_POSITION_H_
#define KEST_FPGA_POSITION_H_

typedef struct {
	int block_start;
	int mem_start;
	int filter_start;
	int delay_start;
} kest_effect_fpga_position;

int kest_effect_fpga_position_init(kest_effect_fpga_position *pos);

int kest_effect_fpga_position_resolve_block(kest_effect_fpga_position *pos, int block);

int kest_effect_fpga_position_resolve_mem(kest_effect_fpga_position *pos, int addr);
int kest_effect_fpga_position_resolve_filter(kest_effect_fpga_position *pos, int handle);
int kest_effect_fpga_position_resolve_delay(kest_effect_fpga_position *pos, int handle);

kest_effect_fpga_position kest_fpga_position_start();

#endif
