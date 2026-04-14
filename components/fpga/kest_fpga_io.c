#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_fpga_io.c";

#ifndef KEST_FPGA_SIMULATED
#include <driver/spi_master.h>

#define PIN_NUM_MISO  	14
#define PIN_NUM_MOSI  	6
#define PIN_NUM_CLK   	5
#define PIN_NUM_CS		4

spi_device_handle_t spi_handle;
#endif

int kest_fpga_spi_init()
{
	#ifndef KEST_FPGA_SIMULATED
    esp_err_t ret;

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK)
        return ERR_SPI_FAIL;

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 4,
    };

    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);
    if (ret != ESP_OK)
        return ERR_SPI_FAIL;
    
    #endif
    return NO_ERROR;
}

int kest_fpga_txrx(uint8_t *tx, uint8_t *rx, size_t len)
{
	#ifdef KEST_FPGA_SIMULATED
	if (rx)
	{
		for (int i = 0; i < len; i++)
			rx[i] = 1;
	}
	return ERR_FEATURE_DISABLED;
	#else
	if (len == 0)
		return 0;
	
	uint8_t reply_buf[len];
	if (!rx)
		rx = reply_buf;
	
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };

	esp_err_t err = spi_device_transmit(spi_handle, &t);
	
	#ifdef PRINT_SPI_BYTES
	char print_buf[len * 10 + 1];
	int buf_index = 0;
	for (int i = 0; i < len; i++)
	{
		sprintf(&print_buf[buf_index], "SEND 0x%02x\n", tx[i]);
		buf_index += 10;
	}
	print_buf[buf_index] = 0;
	KEST_PRINTF_FORCE("\nSPI send to FPGA (%d bytes):\n", len);
	KEST_PRINTF_FORCE(print_buf);
	
	KEST_PRINTF_FORCE("FPGA replied with\n");
	buf_index = 0;
	for (int i = 0; i < len; i++)
	{
		sprintf(&print_buf[buf_index], "RCVD 0x%02x\n", rx[i]);
		buf_index += 10;
	}
	print_buf[buf_index] = 0;
	KEST_PRINTF_FORCE(print_buf);
	#endif
	
	return (err == ESP_OK) ? NO_ERROR : ERR_SPI_FAIL;
	#endif
}

int kest_fpga_send_byte(uint8_t byte)
{
	return kest_fpga_txrx(&byte, NULL, 1);
}

int kest_fpga_send_byte_get_flags(uint8_t tx, kest_fpga_status_flags *flags)
{
	uint8_t rx;
	int ret_val = kest_fpga_txrx(&tx, &rx, 1);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	if (flags)
	{
		return kest_fpga_decode_status_flags(flags, rx);
	}
	
	return NO_ERROR;
}

uint8_t kest_fpga_read_byte()
{
	uint8_t tx = 0xFF;
	uint8_t rx;
	kest_fpga_txrx(&tx, &rx, 1);
	return rx;
}

uint8_t kest_fpga_readout()
{
	uint8_t tx = COMMAND_READOUT;
	uint8_t rx;
	kest_fpga_txrx(&tx, &rx, 1);
	return rx;
}

uint8_t kest_fpga_readout_byte()
{
	uint8_t tx = COMMAND_READOUT;
	uint8_t rx;
	kest_fpga_txrx(&tx, &rx, 1);
	return rx;
}

int kest_fpga_readout_bytes(uint8_t *buf, int n)
{
	if (!buf) return ERR_NULL_PTR;
	if (n < 0) return ERR_BAD_ARGS;
	uint8_t tx[n];
	memset(tx, COMMAND_READOUT, n);
	kest_fpga_send_byte(COMMAND_READOUT);
	return kest_fpga_txrx(tx, buf, n);
}

kest_fpga_transfer_batch kest_new_fpga_transfer_batch()
{
	kest_fpga_transfer_batch seq;
	
	seq.buf = kest_alloc(sizeof(uint32_t) * KEST_FPGA_N_BLOCKS);
	seq.len = 0;
	seq.buf_len = (int)(sizeof(uint32_t) * KEST_FPGA_N_BLOCKS);
	seq.buffer_owned = 0;
	
	return seq;
}

void kest_free_fpga_transfer_batch(kest_fpga_transfer_batch batch)
{
	if (batch.buf && !batch.buffer_owned) kest_free(batch.buf);
}

int kest_fpga_batch_append(kest_fpga_transfer_batch *seq, uint8_t x)
{
	if (!seq)
		return ERR_NULL_PTR;
	
	if (seq->len >= seq->buf_len)
	{
		uint8_t *new_ptr = kest_realloc(seq->buf,(seq->buf_len * 2) * sizeof(uint8_t));
		
		if (!new_ptr)
			return ERR_ALLOC_FAIL;
		
		seq->buf = new_ptr;
		seq->buf_len *= 2;
	}
	
	seq->buf[seq->len++] = x;
	
	return NO_ERROR;
}


int kest_fpga_batch_append_16(kest_fpga_transfer_batch *seq, uint16_t x)
{
	uint8_t bytes[2];
	
	bytes[0] = range_bits(x, 8,  8);
	bytes[1] = range_bits(x, 8,  0);
	
	int ret_val;
	
	if ((ret_val = kest_fpga_batch_append(seq, bytes[0])) != NO_ERROR) return ret_val;
	if ((ret_val = kest_fpga_batch_append(seq, bytes[1])) != NO_ERROR) return ret_val;
	
	return ret_val;
}


int kest_fpga_batch_append_24(kest_fpga_transfer_batch *seq, uint32_t x)
{
	uint8_t bytes[3];
	
	bytes[0] = range_bits(x, 8,  16);
	bytes[1] = range_bits(x, 8,  8);
	bytes[2] = range_bits(x, 8,  0);
	
	int ret_val;
	
	if ((ret_val = kest_fpga_batch_append(seq, bytes[0])) != NO_ERROR) return ret_val;
	if ((ret_val = kest_fpga_batch_append(seq, bytes[1])) != NO_ERROR) return ret_val;
	if ((ret_val = kest_fpga_batch_append(seq, bytes[2])) != NO_ERROR) return ret_val;
	
	return ret_val;
}

int kest_fpga_batch_append_32(kest_fpga_transfer_batch *seq, uint32_t x)
{
	uint8_t bytes[4];
	
	bytes[0] = range_bits(x, 8, 24);
	bytes[1] = range_bits(x, 8, 16);
	bytes[2] = range_bits(x, 8,  8);
	bytes[3] = range_bits(x, 8,  0);
	
	int ret_val;
	
	if ((ret_val = kest_fpga_batch_append(seq, bytes[0])) != NO_ERROR) return ret_val;
	if ((ret_val = kest_fpga_batch_append(seq, bytes[1])) != NO_ERROR) return ret_val;
	if ((ret_val = kest_fpga_batch_append(seq, bytes[2])) != NO_ERROR) return ret_val;
	if ((ret_val = kest_fpga_batch_append(seq, bytes[3])) != NO_ERROR) return ret_val;
	
	return ret_val;
}

int kest_fpga_batch_concat(kest_fpga_transfer_batch *seq, kest_fpga_transfer_batch *seq2)
{
	if (!seq || !seq2)
		return ERR_NULL_PTR;
	
	if (!seq2->buf)
		return ERR_BAD_ARGS;
	
	if (!seq->buf)
	{
		seq->buf = kest_alloc(seq2->len);
		
		if (!seq->buf)
			return ERR_ALLOC_FAIL;
		
		seq->buf_len = seq2->len;
		seq->len = 0;
	}
	else if (seq->len + seq2->len > seq->buf_len)
	{
		int new_len = seq->buf_len;
		while (new_len < seq->len + seq2->len)
			new_len *= 2;
		
		uint8_t *new_ptr = kest_realloc(seq->buf, new_len * sizeof(uint8_t));
		
		if (!new_ptr) return ERR_ALLOC_FAIL;
		
		seq->buf_len = new_len;
		seq->buf = new_ptr;
	}
	
	for (int i = 0; i < seq2->len; i++)
		seq->buf[seq->len + i] = seq2->buf[i];
	
	seq->len += seq2->len;
	
	return 0;
}

int kest_fpga_transfer_batch_send(kest_fpga_transfer_batch batch)
{	
	return kest_fpga_txrx(batch.buf, NULL, batch.len);
}

void kest_fpga_set_input_gain(float gain_db)
{
	float v = powf(10, gain_db / 20.0);
	uint16_t s = float_to_q_nminus1(v, 5);
	
	uint8_t buf[3];
	
	kest_fpga_transfer_batch batch;
	
	batch.buf = buf;
	
	buf[0] = COMMAND_SET_INPUT_GAIN;
	buf[1] = (s & 0xFF00) >> 8;
	buf[2] = s & 0x00FF;
	
	batch.len = 3;
	batch.buf_len = 3;
	batch.buffer_owned = 1;
	
	#ifdef PRINT_TRANSFER_BATCHES
	kest_fpga_batch_print(batch);
	#endif
	
	kest_fpga_transfer_batch_send(batch);
}

void kest_fpga_set_output_gain(float gain_db)
{
	float v = powf(10, gain_db / 20.0);
	uint16_t s = float_to_q_nminus1(v, 5);
	
	uint8_t buf[3];
	
	kest_fpga_transfer_batch batch;
	
	batch.buf = buf;
	
	buf[0] = COMMAND_SET_OUTPUT_GAIN;
	buf[1] = (s & 0xFF00) >> 8;
	buf[2] = s & 0x00FF;
	
	batch.len = 3;
	batch.buf_len = 3;
	batch.buffer_owned = 1;
	
	#ifdef PRINT_TRANSFER_BATCHES
	kest_fpga_batch_print(batch);
	#endif
	
	kest_fpga_transfer_batch_send(batch);
}


void kest_fpga_commit_reg_updates()
{
	kest_fpga_send_byte(COMMAND_COMMIT_REG_UPDATES);
}

char *kest_fpga_command_to_string(int command)
{
	switch (command)
	{
		case COMMAND_BEGIN_PROGRAM: 	 return "COMMAND_BEGIN_PROGRAM";
		case COMMAND_WRITE_BLOCK_INSTR:  return "COMMAND_WRITE_BLOCK_INSTR";
		case COMMAND_WRITE_BLOCK_REG_0:  return "COMMAND_WRITE_BLOCK_REG_0";
		case COMMAND_WRITE_BLOCK_REG_1:  return "COMMAND_WRITE_BLOCK_REG_1";
		case COMMAND_ALLOC_DELAY: 		 return "COMMAND_ALLOC_DELAY";
		case COMMAND_END_PROGRAM: 		 return "COMMAND_END_PROGRAM";
		case COMMAND_SET_INPUT_GAIN: 	 return "COMMAND_SET_INPUT_GAIN";
		case COMMAND_SET_OUTPUT_GAIN: 	 return "COMMAND_SET_OUTPUT_GAIN";
		case COMMAND_UPDATE_BLOCK_REG_0: return "COMMAND_UPDATE_BLOCK_REG_0";
		case COMMAND_UPDATE_BLOCK_REG_1: return "COMMAND_UPDATE_BLOCK_REG_1";
		case COMMAND_COMMIT_REG_UPDATES: return "COMMAND_COMMIT_REG_UPDATES";
		case COMMAND_ALLOC_FILTER: 		 return "COMMAND_ALLOC_FILTER";
		case COMMAND_WRITE_FILTER_COEF:  return "COMMAND_WRITE_FILTER_COEF";
		case COMMAND_UPDATE_FILTER_COEF: return "COMMAND_UPDATE_FILTER_COEF";
		case COMMAND_COMMIT_FILTER_COEF: return "COMMAND_COMMIT_FILTER_COEF";
		case COMMAND_READ: 				 return "COMMAND_READ";
	}
	
	return "UNKNOWN";
}

int kest_fpga_decode_status_flags(kest_fpga_status_flags *flags, uint8_t byte)
{
	if (!flags)
		return ERR_NULL_PTR;
	
	flags->initialised 	= !!(byte & (1 << 0));
	flags->busy 		= !!(byte & (1 << 1));
	flags->timeout 		= !!(byte & (1 << 2));
	flags->programming 	= !!(byte & (1 << 3));
	flags->bad 			= !!(byte & (1 << 4));
	flags->data_ready 	= !!(byte & (1 << 5));
	flags->cmd_err 		= !!(byte & (1 << 6));
	flags->swapping		= !!(byte & (1 << 7));
	
	return NO_ERROR;
}

int kest_fpga_get_status_flags(kest_fpga_status_flags *flags)
{
	if (!flags)
		return ERR_NULL_PTR;
	
	uint8_t byte = kest_fpga_read_byte();
	
	kest_fpga_decode_status_flags(flags, byte);
	
	return NO_ERROR;
}

int kest_fpga_status_flags_sprint(kest_string *str, kest_fpga_status_flags *flags)
{
	if (!flags || !str)
		return ERR_NULL_PTR;
	
	kest_string_appendf(str, "\nFPGA Status Flags:\n");
	kest_string_appendf(str, "     Initialised: %d\n", flags->initialised);
	kest_string_appendf(str, "     Busy:        %d\n", flags->busy);
	kest_string_appendf(str, "     Timeout:     %d\n", flags->timeout);
	kest_string_appendf(str, "     Programming: %d\n", flags->programming);
	kest_string_appendf(str, "     Bad:         %d\n", flags->bad);
	kest_string_appendf(str, "     Data ready:  %d\n", flags->data_ready);
	kest_string_appendf(str, "     Command err: %d\n", flags->cmd_err);
	kest_string_appendf(str, "     Swapping:    %d\n", flags->swapping);
	
	return NO_ERROR;
}

int kest_fpga_status_flags_print(kest_fpga_status_flags *flags)
{
	if (!flags)
		return ERR_NULL_PTR;
	
	kest_string str;
	kest_string_init(&str);
	
	kest_string_appendf(&str, "\nFPGA Status Flags:\n");
	kest_string_appendf(&str, "     Initialised: %d\n", flags->initialised);
	kest_string_appendf(&str, "     Busy:        %d\n", flags->busy);
	kest_string_appendf(&str, "     Timeout:     %d\n", flags->timeout);
	kest_string_appendf(&str, "     Programming: %d\n", flags->programming);
	kest_string_appendf(&str, "     Bad:         %d\n", flags->bad);
	kest_string_appendf(&str, "     Data ready:  %d\n", flags->data_ready);
	kest_string_appendf(&str, "     Command err: %d\n", flags->cmd_err);
	kest_string_appendf(&str, "     Swapping:    %d\n", flags->swapping);
	
	kest_puts(str);
	
	return NO_ERROR;
}

int64_t kest_fpga_req_data(int req, int n_bytes, kest_fpga_status_flags *flags)
{
	if (!flags) return -1;
	
	kest_fpga_send_byte_get_flags(COMMAND_READ, flags);
	kest_fpga_send_byte_get_flags(req, flags);
	
	if (flags->cmd_err)
		return -2;
	
	int tries = 16;
	
	do {
		kest_fpga_get_status_flags(flags);
		if (flags->cmd_err) return -4;
		if (!flags->data_ready)
			vTaskDelay(1);
	} while (!flags->data_ready && tries --> 0);
	
	int64_t data = 0;
	
	if (flags->data_ready)
	{
		kest_fpga_send_byte_get_flags(COMMAND_READOUT, flags);
		
		if (flags->cmd_err)
			return -6;
		
		for (int i = 0; i < n_bytes; i++)
			data = (data << 8) | kest_fpga_readout();
	}
	else
	{
		data = -5;
	}
	
	kest_fpga_send_byte_get_flags(COMMAND_CLEAR_CMD_ERR_FLAG, flags);
	
	return data;
}

int64_t kest_fpga_req_data_p(uint8_t req, uint8_t *p, int n, int m, kest_fpga_status_flags *flags)
{
	if (!p || !flags)
		return -1;
	
	kest_fpga_send_byte_get_flags(COMMAND_READ, flags);
	kest_fpga_send_byte_get_flags(req, flags);
	
	if (flags->cmd_err)
		return -2;
	
	for (int i = 0; i < n; i++)
	{
		kest_fpga_send_byte_get_flags(p[i], flags);
		
		if (flags->cmd_err)
			return -3;
	}
	
	int tries = 32;
	
	do {
		kest_fpga_get_status_flags(flags);
		if (flags->cmd_err) return -4;
		if (!flags->data_ready)
			vTaskDelay(1);
	} while (!flags->data_ready && tries --> 0);
	
	int64_t data = 0;
	
	if (flags->data_ready)
	{
		kest_fpga_send_byte_get_flags(COMMAND_READOUT, flags);
		
		if (flags->cmd_err)
			return -6;
		
		for (int i = 0; i < m; i++)
			data = (data << 8) | kest_fpga_readout();
	}
	else
	{
		data = -5;
	}
	
	kest_fpga_send_byte_get_flags(COMMAND_CLEAR_CMD_ERR_FLAG, flags);
	
	return data;
}


uint32_t kest_fpga_get_block_instr(int block, kest_fpga_status_flags *flags)
{
	uint8_t p[2];
	p[0] = (block & 0xFF00) >> 8;
	p[1] = (block & 0x00FF) >> 0;
	
	return kest_fpga_req_data_p(DATA_REQ_BLOCK_INSTR, p, 2, 4, flags);
}


int kest_fpga_get_n_blocks(kest_fpga_status_flags *flags)
{
	return kest_fpga_req_data(DATA_REQ_N_BLOCKS, 2, flags);
}

uint32_t kest_fpga_get_stuck_flags(kest_fpga_status_flags *flags)
{
	return kest_fpga_req_data(DATA_REQ_STUCK_FLAGS, 4, flags);
}

uint16_t kest_fpga_get_block_reg(int block, int reg, kest_fpga_status_flags *flags)
{
	uint8_t p[3];
	p[0] = !!reg;
	p[1] = (block & 0xFF00) >> 8;
	p[2] = (block & 0x00FF) >> 0;
	
	return kest_fpga_req_data_p(DATA_REQ_BLOCK_REG, p, 3, 2, flags);
}

int kest_fpga_get_n_delay_buffers(kest_fpga_status_flags *flags)
{
	return kest_fpga_req_data(DATA_REQ_N_DELAY_BUF, 2, flags);
}

int kest_fpga_get_delay_buffer_size(int handle, kest_fpga_status_flags *flags)
{
	uint8_t p = handle & 0xFF;
	
	return kest_fpga_req_data_p(DATA_REQ_DELAY_BUF_SIZE, &p, 1, 3, flags);
}

int kest_fpga_get_delay_buffer_delay(int handle, kest_fpga_status_flags *flags)
{
	uint8_t p = handle & 0xFF;
	
	return kest_fpga_req_data_p(DATA_REQ_DELAY_BUF_DELAY, &p, 1, 3, flags);
}

int kest_fpga_get_delay_buffer_addr(int handle, kest_fpga_status_flags *flags)
{
	uint8_t p = handle & 0xFF;
	
	return kest_fpga_req_data_p(DATA_REQ_DELAY_BUF_ADDR, &p, 1, 3, flags);
}

int kest_fpga_get_delay_buffer_pos(int handle, kest_fpga_status_flags *flags)
{
	uint8_t p = handle & 0xFF;
	
	return kest_fpga_req_data_p(DATA_REQ_DELAY_BUF_POS, &p, 1, 3, flags);
}

uint16_t kest_fpga_get_delay_buffer_gain(int handle, kest_fpga_status_flags *flags)
{
	uint8_t p = handle & 0xFF;
	
	return kest_fpga_req_data_p(DATA_REQ_DELAY_BUF_GAIN, &p, 1, 2, flags);
}

int kest_fpga_get_delay_buffer_last_rw_addr(int handle, uint16_t *read_addr, uint16_t *write_addr, kest_fpga_status_flags *flags)
{
	uint8_t p = handle & 0xFF;
	
	uint32_t w = kest_fpga_req_data_p(DATA_REQ_DELAY_BUF_LRWA, &p, 1, 4, flags);
	
	if (read_addr) *read_addr = (w & 0x0000FFFF) >> 0;
	if (write_addr) *write_addr = (w & 0xFFFF0000) >> 16;
	
	return NO_ERROR;
}


uint64_t kest_fpga_get_sample_count(kest_fpga_status_flags *flags)
{
	return kest_fpga_req_data(DATA_REQ_SAMPLE_COUNT, 8, flags);
}

uint64_t kest_fpga_get_sdram_read_count(kest_fpga_status_flags *flags)
{
	return kest_fpga_req_data(DATA_REQ_SDRAM_READ_CNT, 8, flags);
}

uint64_t kest_fpga_get_sdram_write_count(kest_fpga_status_flags *flags)
{
	return kest_fpga_req_data(DATA_REQ_SDRAM_WRITE_CNT, 8, flags);
}


/*
int kest_fpga_read_command_log(uint8_t *buf)
{
	if (!buf)
		return ERR_NULL_PTR;
	
	kest_fpga_status_flags flags;
	int tries = 16;
	
	kest_fpga_send_byte(COMMAND_READ_COMMAND_LOG);
	
	do {
		kest_fpga_get_status_flags(&flags);
	} while (!flags.data_ready && tries --> 0);
	
	if (!flags.data_ready)
		return ERR_UNKNOWN_ERR;
	
	uint8_t tx[9];
	uint8_t rx[9];
	int ret_val;
	
	for (int i = 0; i < 9; i++)
		tx[i] = COMMAND_READOUT;
	
	ret_val = kest_fpga_txrx(tx, rx, 9);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	memcpy(&rx[1], buf, 8);
	
	return NO_ERROR;
}
*/
