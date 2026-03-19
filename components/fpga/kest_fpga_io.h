#ifndef KEST_INT_FPGA_H_
#define KEST_INT_FPGA_H_

#define KEST_FPGA_SAMPLE_RATE 44100

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

#define SPI_RESPONSE_OK 			0
#define SPI_RESPONSE_INITIALISING	1
#define SPI_RESPONSE_PROGRAMMING 	2
#define SPI_RESPONSE_REJECTED		3
#define SPI_RESPONSE_TIMEOUT		4

#define KEST_FPGA_N_BLOCKS 256

#if KEST_FPGA_N_BLOCKS > 256
  #define KEST_FPGA_BLOCK_ADDR_BYTES 2
  typedef uint16_t kest_fpga_block_addr_t;
#else
  #define KEST_FPGA_BLOCK_ADDR_BYTES 1
  typedef uint8_t kest_fpga_block_addr_t;
#endif

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

void kest_fpga_set_input_gain (float gain_db);
void kest_fpga_set_output_gain(float gain_db);
void kest_fpga_commit_reg_updates();

int kest_fpga_batch_append   (kest_fpga_transfer_batch *seq, uint8_t  x);
int kest_fpga_batch_append_16(kest_fpga_transfer_batch *seq, uint16_t x);
int kest_fpga_batch_append_24(kest_fpga_transfer_batch *seq, uint32_t x);
int kest_fpga_batch_append_32(kest_fpga_transfer_batch *seq, uint32_t x);

int kest_fpga_transfer_batch_send(kest_fpga_transfer_batch batch);

int kest_fpga_spi_init();

char *kest_fpga_command_to_string(int command);

#endif
