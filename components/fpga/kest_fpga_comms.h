#ifndef KEST_FPGA_COMMS_H_
#define KEST_FPGA_COMMS_H_

//#define PRINT_TRANSFER_BATCHES
//#define PRINT_SCAN
//#define PRINT_PROGRAM_BATCHES
//#define PRINT_FLAGS
//#define PRINT_COMMANDS

int kest_init_fpga_comms();

void kest_fpga_comms_task(void *param);

int kest_fpga_queue_transfer_batch(kest_fpga_transfer_batch batch);
int kest_fpga_queue_program_batch(kest_fpga_transfer_batch batch);

int kest_fpga_queue_input_gain_set(float gain_db);
int kest_fpga_queue_output_gain_set(float gain_db);

int kest_fpga_queue_register_commit();
int kest_fpga_queue_read(kest_fpga_read_spec *spec);

#endif
