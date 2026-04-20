#ifndef KEST_INT_FPGA_H_
#define KEST_INT_FPGA_H_

#define KEST_FPGA_SAMPLE_RATE 44100

#define FPGA_BOOT_MS 2500

//#define PRINT_SPI_BYTES

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
#define COMMAND_GET_N_BLOCKS 		21
#define COMMAND_GET_BLOCK_INSTR		22
#define COMMAND_GET_BLOCK_REG		23
#define COMMAND_GET_N_DELAY_BUF		24
#define COMMAND_GET_DELAY_BUF_SIZE  25
#define COMMAND_GET_DELAY_BUF_DELAY 26
#define COMMAND_GET_DELAY_BUF_ADDR 	27
#define COMMAND_GET_DELAY_BUF_POS 	28
#define COMMAND_GET_DELAY_BUF_GAIN 	29
#define COMMAND_GET_DELAY_BUF_LRWA 	30
#define COMMAND_GET_SDRAM_READ_CNT	31
#define COMMAND_GET_SDRAM_WRITE_CNT	32
#define COMMAND_READ_COMMAND_LOG	33
#define COMMAND_GET_SAMPLE_COUNT	34
#define COMMAND_CLEAR_TIMEOUT_FLAG	35
#define COMMAND_CLEAR_BAD_FLAG		36
#define COMMAND_CLEAR_CMD_ERR_FLAG	37
#define COMMAND_READ				38

#define SPI_RESPONSE_OK 1

#define KEST_FPGA_N_BLOCKS 256

#define KEST_FPGA_DATA_WIDTH 16
#define KEST_FPGA_FILTER_WIDTH 18
#define KEST_FPGA_DATA_BYTES (KEST_FPGA_DATA_WIDTH / 8)

#if KEST_FPGA_DATA_WIDTH == 16
  typedef int16_t kest_fpga_sample_t;
#elif KEST_FPGA_DATA_WIDTH == 24
  typedef int32_t kest_fpga_sample_t;
#endif

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

#define DATA_REQ_N_BLOCKS 		14
#define DATA_REQ_BLOCK_INSTR 	1
#define DATA_REQ_BLOCK_REG	 	2

#define DATA_REQ_N_DELAY_BUF	 3
#define DATA_REQ_DELAY_BUF_SIZE	 4
#define DATA_REQ_DELAY_BUF_DELAY 5
#define DATA_REQ_DELAY_BUF_ADDR  6
#define DATA_REQ_DELAY_BUF_POS   7
#define DATA_REQ_DELAY_BUF_GAIN  8
#define DATA_REQ_DELAY_BUF_LRWA  9
#define DATA_REQ_SAMPLE_COUNT	 10
#define DATA_REQ_SDRAM_READ_CNT	 11
#define DATA_REQ_SDRAM_WRITE_CNT 12
#define DATA_REQ_STUCK_FLAGS	 13


#endif
