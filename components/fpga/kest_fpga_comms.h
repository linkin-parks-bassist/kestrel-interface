#ifndef KEST_FPGA_COMMS_H_
#define KEST_FPGA_COMMS_H_

//#define PRINT_TRANSFER_BATCHES
//#define PRINT_SCAN
//#define PRINT_PROGRAM_BATCHES
//#define PRINT_FLAGS
//#define PRINT_COMMANDS
//#define PRINT_READS

#define KEST_FPGA_MSG_TYPE_BATCH 			0
#define KEST_FPGA_MSG_TYPE_PROGRAM_BATCH 	1
#define KEST_FPGA_MSG_TYPE_SET_INPUT_GAIN 	2
#define KEST_FPGA_MSG_TYPE_SET_OUTPUT_GAIN  3
#define KEST_FPGA_MSG_TYPE_COMMAND			4
#define KEST_FPGA_MSG_TYPE_READ				5
#define KEST_FPGA_MSG_TYPE_MEM_READ			6

typedef struct {
	int type;
	
	union {
		float level;
		uint8_t command;
		kest_fpga_transfer_batch batch;
		kest_fpga_read_spec *read;
		kest_fpga_mem_read_spec mem_read;
	} data;
} kest_fpga_msg;

int kest_init_fpga_comms();

void kest_fpga_comms_task(void *param);

int kest_fpga_queue_transfer_batch(kest_fpga_transfer_batch batch);
int kest_fpga_queue_program_batch(kest_fpga_transfer_batch batch);

int kest_fpga_queue_input_gain_set(float gain_db);
int kest_fpga_queue_output_gain_set(float gain_db);

int kest_fpga_queue_register_commit();
int kest_fpga_queue_read(kest_fpga_read_spec *spec);

int kest_fpga_queue_mem_read(int addr, void (*callback)(kest_fpga_sample_t, void*), void *cb_arg);

#endif
