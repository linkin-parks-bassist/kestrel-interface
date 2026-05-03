#ifndef KEST_FPGA_READ_H_
#define KEST_FPGA_READ_H_

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
	int active;
	int period_ms;
	lv_timer_t *timer;
	kest_fpga_read_spec spec;
} kest_fpga_periodic_read;

int kest_fpga_periodic_read_init(kest_fpga_periodic_read *read);
int kest_fpga_periodic_read_init_mem(kest_fpga_periodic_read *read);

int kest_periodic_mem_read_cb(kest_fpga_read_spec *spec);

int kest_fpga_periodic_read_activate  (kest_fpga_periodic_read *read);
int kest_fpga_periodic_read_deactivate(kest_fpga_periodic_read *read);

int kest_fpga_periodic_read_activate_async  (kest_fpga_periodic_read *read);
int kest_fpga_periodic_read_deactivate_async(kest_fpga_periodic_read *read);

#endif
