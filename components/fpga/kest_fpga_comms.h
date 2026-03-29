#ifndef KEST_FPGA_COMMS_H_
#define KEST_FPGA_COMMS_H_

//#define PRINT_TRANSFER_BATCHES
#define PRINT_STATUS_CODES
//#define PRINT_COMMANDS

void kest_fpga_comms_task(void *param);

int kest_fpga_queue_transfer_batch(kest_fpga_transfer_batch batch);
int kest_fpga_queue_program_batch(kest_fpga_transfer_batch batch);

int kest_fpga_queue_input_gain_set(float gain_db);
int kest_fpga_queue_output_gain_set(float gain_db);

int kest_fpga_queue_register_commit();

#endif
