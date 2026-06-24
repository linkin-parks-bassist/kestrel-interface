#ifndef KEST_FPGA_COMMAND_H_
#define KEST_FPGA_COMMAND_H_

#define COMMAND_BEGIN_PROGRAM	 	1
#define COMMAND_WRITE_BLOCK_INSTR 	2
#define COMMAND_WRITE_BLOCK_REG_0 	3
#define COMMAND_WRITE_BLOCK_REG_1 	4
#define COMMAND_ALLOC_DELAY 		5
#define COMMAND_END_PROGRAM	 		10
#define COMMAND_SET_INPUT_GAIN 		11
#define COMMAND_SET_OUTPUT_GAIN 	12
#define COMMAND_UPDATE_BLOCK_REG_0 	13
#define COMMAND_UPDATE_BLOCK_REG_1 	14
#define COMMAND_COMMIT_REG_UPDATES 	15
#define COMMAND_ALLOC_FILTER	 	16
#define COMMAND_WRITE_FILTER_COEF 	17
#define COMMAND_UPDATE_FILTER_COEF 	18
#define COMMAND_COMMIT_FILTER_COEF 	19
#define COMMAND_READOUT				20
#define COMMAND_CLEAR_TIMEOUT_FLAG	35
#define COMMAND_CLEAR_BAD_FLAG		36
#define COMMAND_CLEAR_CMD_ERR_FLAG	37
#define COMMAND_READ				38
#define COMMAND_ENABLE_TAIL			39

typedef struct {
	int type;
	
	union {
		int block;
		int handle;
		int size;
		int order_ff;
	} data_1;
	
	union {
		int delay;
		int coef;
		int order_fb;
		uint32_t instr;
	} data_2;
	
	float val;
	int format;
} kest_fpga_command;

DECLARE_LIST(kest_fpga_command);

kest_fpga_command kest_fpga_command_begin_program();
kest_fpga_command kest_fpga_command_end_program();

kest_fpga_command kest_fpga_command_write_block_instr(int block, uint32_t instr);

kest_fpga_command kest_fpga_command_write_block_reg_0 (int block, float val, int format);
kest_fpga_command kest_fpga_command_write_block_reg_1 (int block, float val, int format);
kest_fpga_command kest_fpga_command_update_block_reg_0(int block, float val, int format);
kest_fpga_command kest_fpga_command_update_block_reg_1(int block, float val, int format);

kest_fpga_command kest_fpga_command_commit_reg_updates();

kest_fpga_command kest_fpga_command_alloc_delay(int size, int delay);
kest_fpga_command kest_fpga_command_alloc_filter(int format, int order_ff, int order_fb);

kest_fpga_command kest_fpga_command_write_filter_coef (int handle, int coef, float val, int format);
kest_fpga_command kest_fpga_command_update_filter_coef(int handle, int coef, float val, int format);

kest_fpga_command kest_fpga_command_commit_filter_coefs(int handle);

kest_fpga_command kest_fpga_command_enable_tail();

int kest_fpga_command_append_encoded(kest_fpga_command cmd, kest_fpga_transfer_batch *batch);
int kest_fpga_command_list_append_encoded(kest_fpga_command_list *cmds, kest_fpga_transfer_batch *batch);

char *kest_fpga_command_to_string(int command);
int kest_fpga_command_to_string_(kest_fpga_command cmd, kest_string *str);

#endif
