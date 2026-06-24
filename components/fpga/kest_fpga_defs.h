#ifndef KEST_FPGA_DEFS_H_
#define KEST_FPGA_DEFS_H_

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
