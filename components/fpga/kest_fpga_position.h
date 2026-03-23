#ifndef KEST_FPGA_POSITION_H_
#define KEST_FPGA_POSITION_H_

typedef struct {
	int block_start;
	
	int filter_start;
} kest_effect_fpga_position;

int kest_effect_fpga_position_init(kest_effect_fpga_position *pos);

#endif
