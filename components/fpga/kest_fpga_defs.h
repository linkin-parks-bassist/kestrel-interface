#ifndef KEST_FPGA_DEFS_H_
#define KEST_FPGA_DEFS_H_

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
#define DATA_REQ_N_BLOCKS 		 14
#define DATA_REQ_MEM			 15
#define DATA_REQ_COMMAND_LOG	 33

#define KEST_FPGA_SAMPLE_RATE 44100

#define FPGA_BOOT_MS 2500

#define SPI_RESPONSE_OK 1

#define KEST_FPGA_N_BLOCKS 256

#define KEST_FPGA_DATA_WIDTH 16
#define KEST_FPGA_DATA_BYTES (KEST_FPGA_DATA_WIDTH / 8)

#define KEST_FPGA_FILTER_WIDTH 18
#define KEST_FPGA_MEM_ADDR_BYTES 2
#define KEST_FPGA_DATA_BYTES (KEST_FPGA_DATA_WIDTH / 8)

#define KEST_FPGA_GAIN_FORMAT 5

#define KEST_FPGA_FILTER_COEF_INDEX_BYTES 2

#if KEST_FPGA_DATA_WIDTH == 16
  typedef int16_t kest_fpga_sample_t;
#elif KEST_FPGA_DATA_WIDTH == 24
  typedef int32_t kest_fpga_sample_t;
#endif

static inline float kest_fpga_sample_to_float(kest_fpga_sample_t s)
{
	return powf(2.0f, -(KEST_FPGA_DATA_WIDTH - 1)) * (float)s;
}

#endif
