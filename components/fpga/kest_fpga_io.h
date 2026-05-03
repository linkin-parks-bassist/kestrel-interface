#ifndef KEST_INT_FPGA_H_
#define KEST_INT_FPGA_H_

//#define PRINT_SPI_BYTES

typedef struct {
	int len;
	uint8_t *buf;
	int buf_len;
	int buffer_owned;
} kest_fpga_transfer_batch;

kest_fpga_transfer_batch kest_new_fpga_transfer_batch();
void kest_free_fpga_transfer_batch(kest_fpga_transfer_batch batch);

int kest_send_bytes_to_fpga(uint8_t *buf, int n);
int kest_send_byte_to_fpga(uint8_t byte);

int kest_fpga_send_byte(uint8_t byte);
uint8_t kest_fpga_read_byte();
uint8_t kest_fpga_readout();

void kest_fpga_set_input_gain (float gain_db);
void kest_fpga_set_output_gain(float gain_db);
void kest_fpga_commit_reg_updates();

int kest_fpga_batch_append   (kest_fpga_transfer_batch *seq, uint8_t  x);
int kest_fpga_batch_append_16(kest_fpga_transfer_batch *seq, uint16_t x);
int kest_fpga_batch_append_24(kest_fpga_transfer_batch *seq, uint32_t x);
int kest_fpga_batch_append_32(kest_fpga_transfer_batch *seq, uint32_t x);

int kest_fpga_transfer_batch_send(kest_fpga_transfer_batch batch);
int kest_fpga_transfer_batch_send_careful(kest_fpga_transfer_batch batch);
int kest_fpga_program_batch_send_careful(kest_fpga_transfer_batch batch);
int kest_fpga_program_batch_send(kest_fpga_transfer_batch batch);

int kest_fpga_spi_init();

char *kest_fpga_command_to_string(int command);

typedef struct
{
	int initialised;
	int listening;
	int timeout;
	int programming;
	int bad;
	int data_ready;
	int cmd_err;
	int swapping;
} kest_fpga_status_flags;

int kest_fpga_decode_status_flags(kest_fpga_status_flags *flags, uint8_t byte);
int kest_fpga_send_byte_get_flags(uint8_t tx, kest_fpga_status_flags *flags);
int kest_fpga_get_status_flags(kest_fpga_status_flags *flags);
int kest_fpga_status_flags_print(kest_fpga_status_flags *flags);
int kest_fpga_status_flags_sprint(kest_string *str, kest_fpga_status_flags *flags);

int64_t kest_fpga_req_data_p(uint8_t req, uint8_t *p, int n, int m, kest_fpga_status_flags *flags);

int kest_fpga_get_n_blocks(kest_fpga_status_flags *flags);
uint32_t kest_fpga_get_block_instr(int block, kest_fpga_status_flags *flags);
uint16_t kest_fpga_get_block_reg(int block, int reg, kest_fpga_status_flags *flags);

int kest_fpga_get_n_delay_buffers(kest_fpga_status_flags *flags);
int kest_fpga_get_delay_buffer_size(int handle, kest_fpga_status_flags *flags);
int kest_fpga_get_delay_buffer_delay(int handle, kest_fpga_status_flags *flags);

int kest_fpga_get_delay_buffer_addr(int handle, kest_fpga_status_flags *flags);
int kest_fpga_get_delay_buffer_pos(int handle, kest_fpga_status_flags *flags);
uint16_t kest_fpga_get_delay_buffer_gain(int handle, kest_fpga_status_flags *flags);
int kest_fpga_get_delay_buffer_last_rw_addr(int handle, uint16_t *read_addr, uint16_t *write_addr, kest_fpga_status_flags *flags);

uint64_t kest_fpga_get_sample_count(kest_fpga_status_flags *flags);

uint64_t kest_fpga_get_sdram_read_count(kest_fpga_status_flags *flags);
uint64_t kest_fpga_get_sdram_write_count(kest_fpga_status_flags *flags);

uint32_t kest_fpga_get_stuck_flags(kest_fpga_status_flags *flags);

int kest_fpga_read_command_log(uint8_t *buf);

#endif
