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
#define KEST_FPGA_MSG_TYPE_STATUS			7
#define KEST_FPGA_MSG_TYPE_CALLBACK		8

typedef struct {
	int addr;
	void *data;
	void (*callback)(void *data, int64_t result);
} kest_fpga_mem_read_spec;

#define KEST_FPGA_READ32 256 // Host read kind; the SPI command is COMMAND_READ32.

typedef struct kest_fpga_read_spec {
	int type;
	int id;
	uint8_t addr[6];
	size_t addr_size;
	size_t ret_size;
	int64_t result;
	void *data;

	int (*callback)(struct kest_fpga_read_spec*);
} kest_fpga_read_spec;

typedef struct {
	int type;

	union {
		float level;
		uint8_t command;
		kest_fpga_transfer_batch batch;
		kest_fpga_read_spec *read;
		kest_fpga_mem_read_spec mem_read;
		struct { void (*call)(void *); void *data; } callback;
		void (*status_callback)(int result, uint8_t flags);
	} data;
} kest_fpga_msg;

int kest_init_fpga_comms();

void kest_fpga_comms_task(void *param);

int kest_fpga_queue_transfer_batch(kest_fpga_transfer_batch batch);
int kest_fpga_queue_program_batch(kest_fpga_transfer_batch batch);

int kest_fpga_queue_input_gain_set(float gain_db);
int kest_fpga_queue_output_gain_set(float gain_db);

// Runs on the SPI task after all earlier messages/callbacks have finished.
int kest_fpga_queue_callback(void (*callback)(void *), void *data);

int kest_fpga_queue_register_commit();
int kest_fpga_queue_read(kest_fpga_read_spec *spec);
int kest_fpga_queue_status(void (*callback)(int result, uint8_t flags));

// data must remain alive until SPI invokes callback; the request itself is copied.
int kest_fpga_queue_mem_read(int addr, void *data, void (*callback)(void*, int64_t));

#endif
