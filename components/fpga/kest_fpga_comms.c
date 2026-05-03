#include "kest_int.h"

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "kest_fpga_comms.c";

int kest_init_fpga_comms()
{
	xTaskCreate(kest_fpga_comms_task,   "kest_fpga_comms_task",   4096, NULL, 8, NULL);
	return NO_ERROR;
}

#define KEST_FPGA_MSG_TYPE_BATCH 			0
#define KEST_FPGA_MSG_TYPE_PROGRAM_BATCH 	1
#define KEST_FPGA_MSG_TYPE_SET_INPUT_GAIN 	2
#define KEST_FPGA_MSG_TYPE_SET_OUTPUT_GAIN  3
#define KEST_FPGA_MSG_TYPE_COMMAND			4
#define KEST_FPGA_MSG_TYPE_READ				5

typedef struct {
	int type;
	
	union {
		float level;
		uint8_t command;
		kest_fpga_transfer_batch batch;
		kest_fpga_read_spec *read;
	} data;
} kest_fpga_msg;

static QueueHandle_t fpga_msg_queue;
static int initialised = 0;

#define PROGRAM_RETRIES 3

void kest_fpga_comms_print_full_scan();
void kest_fpga_handle_misalignment();
void kest_command_log_test();

void kest_fpga_comms_task(void *param)
{
	kest_fpga_spi_init();
	
	fpga_msg_queue = xQueueCreate(32, sizeof(kest_fpga_msg));
	initialised = 1;
	
	#ifndef KEST_FPGA_SIMULATED
	vTaskDelay(pdMS_TO_TICKS(FPGA_BOOT_MS));
	#endif
	
	//kest_command_log_test();
	
	//vTaskDelete(NULL);
	
	kest_fpga_status_flags status;
	
	int status_check_ms = 5;
	int status_check_ticks = 5 / portTICK_PERIOD_MS;
	int status_check_delay = (status_check_ticks == 0) ? 1 : status_check_ticks;
	/*
	do {
		kest_fpga_get_status_flags(&status);
		vTaskDelay(status_check_delay);
	} while (!status.initialised);
	*/
	uint8_t byte;
	
	//kest_fpga_status_flags_print(&status);
	
	int busy_tries;
	
	int64_t addr_;
	
	kest_fpga_msg msg;
	BaseType_t ret;
	
	while (1)
	{
		do {
			ret = xQueueReceive(fpga_msg_queue, &msg, pdMS_TO_TICKS(1000));
			if (ret != pdPASS)
			{
				kest_fpga_get_status_flags(&status);
				
				#ifdef PRINT_SCAN
				kest_fpga_comms_print_full_scan();
				#else
				#ifdef PRINT_FLAGS
				kest_fpga_status_flags_print(&status);
				#endif
				#endif
				
								
				if (status.timeout)
				{
					kest_fpga_send_byte(COMMAND_CLEAR_TIMEOUT_FLAG);
					KEST_PRINTF("FPGA timed out =/ clearing the flag...\n");
				}
			}
		} while (ret != pdPASS);
		
		switch (msg.type)
		{
			case KEST_FPGA_MSG_TYPE_PROGRAM_BATCH:
				#ifdef PRINT_TRANSFER_BATCHES
				kest_fpga_batch_print(msg.data.batch);
				#else
				#ifdef PRINT_PROGRAM_BATCHES
				kest_fpga_batch_print(msg.data.batch);
				#endif
				#endif
				#ifndef KEST_FPGA_SIMULATED
				for (int i = 0; i < PROGRAM_RETRIES; i++)
				{
					KEST_PRINTF("Sending program batch... first, check flags\n");
					kest_fpga_get_status_flags(&status);
					#ifdef PRINT_FLAGS
					kest_fpga_status_flags_print(&status);
					#endif
					KEST_PRINTF("Starting...\n");
					kest_fpga_program_batch_send(msg.data.batch);
					vTaskDelay(status_check_delay);
					kest_fpga_get_status_flags(&status);
					
					#ifdef PRINT_FLAGS
					kest_fpga_status_flags_print(&status);
					#endif
					
					if (status.programming)
					{
						KEST_PRINTF("Alignment mismatch or incomplete command in programming batch!\n");
						kest_fpga_handle_misalignment();
						break;
					}
					
					if (status.timeout)
					{
						KEST_PRINTF("FPGA controller timed out!!!!!!!!!!!!\n");
						kest_fpga_send_byte(COMMAND_CLEAR_TIMEOUT_FLAG);
						KEST_PRINTF("Clear flag...\n");
						kest_fpga_get_status_flags(&status);
						kest_fpga_status_flags_print(&status);
						KEST_PRINTF("Retrying...\n");
						continue;
					}
					
					while (status.swapping)
					{
						vTaskDelay(status_check_delay);
						kest_fpga_get_status_flags(&status);
					}
					
					if (status.bad)
					{
						KEST_PRINTF("Pipeline failed health check. Retrying\n");
						kest_fpga_send_byte(COMMAND_CLEAR_BAD_FLAG);
						KEST_PRINTF("Retrying...\n");
						continue;
					}
					else
					{
						KEST_PRINTF("FPGA accepted the new pipeline :)\n");
						vTaskDelay(pdMS_TO_TICKS(50));
						activate_active_preset_dma();
						break;
					}
				}
				#else
				activate_active_preset_dma();
				#endif
				
				kest_free_fpga_transfer_batch(msg.data.batch);
				break;
			
			case KEST_FPGA_MSG_TYPE_BATCH:
				#ifdef PRINT_TRANSFER_BATCHES
				kest_fpga_batch_print(msg.data.batch);
				#endif
				kest_fpga_transfer_batch_send(msg.data.batch);
				kest_free_fpga_transfer_batch(msg.data.batch);
				break;
			
			case KEST_FPGA_MSG_TYPE_SET_INPUT_GAIN:
				#ifdef PRINT_COMMANDS
				KEST_PRINTF_FORCE("Set FPGA input gain to %f\n", msg.data.level);
				#endif
				kest_fpga_set_input_gain(msg.data.level);
				break;
			
			case KEST_FPGA_MSG_TYPE_SET_OUTPUT_GAIN:
				#ifdef PRINT_COMMANDS
				KEST_PRINTF_FORCE("Set FPGA output gain to %f\n", msg.data.level);
				#endif
				kest_fpga_set_output_gain(msg.data.level);
				break;
				
			case KEST_FPGA_MSG_TYPE_COMMAND:
				#ifdef PRINT_COMMANDS
				KEST_PRINTF_FORCE("send FPGA command %s\n", kest_fpga_command_to_string(msg.data.command));
				#endif
				kest_fpga_send_byte(msg.data.command);
				break;
				
			case KEST_FPGA_MSG_TYPE_READ:
				if (msg.data.read)
				{
					msg.data.read->result = kest_fpga_req_data_p(msg.data.read->type, msg.data.read->addr, msg.data.read->addr_size, msg.data.read->ret_size, &status);
					
					if (msg.data.read->callback)
						msg.data.read->callback(msg.data.read);
				}
				break;
		}
	}
	
	vTaskDelete(NULL);
}

static inline int kest_fpga_queue_msg(kest_fpga_msg msg)
{
	while (!initialised);
	
	if (xQueueSend(fpga_msg_queue, (void*)&msg, (TickType_t)1) != pdPASS)
	{
		return ERR_QUEUE_SEND_FAILED;
	}
	return NO_ERROR;
}

int kest_fpga_queue_transfer_batch(kest_fpga_transfer_batch batch)
{
	kest_fpga_msg msg;
	
	msg.type = KEST_FPGA_MSG_TYPE_BATCH;
	msg.data.batch = batch;
	
	int ret_val = kest_fpga_queue_msg(msg);
	
	return ret_val;
}

int kest_fpga_queue_program_batch(kest_fpga_transfer_batch batch)
{
	kest_fpga_msg msg;
	
	KEST_PRINTF("QUEUEING PROGRAM BATCH\n");
	msg.type = KEST_FPGA_MSG_TYPE_PROGRAM_BATCH;
	msg.data.batch = batch;
	
	int ret_val = kest_fpga_queue_msg(msg);
	
	return ret_val;
}

int kest_fpga_queue_input_gain_set(float gain_db)
{
	kest_fpga_msg msg;
	
	msg.type = KEST_FPGA_MSG_TYPE_SET_INPUT_GAIN;
	msg.data.level = gain_db;
	
	return kest_fpga_queue_msg(msg);
}

int kest_fpga_queue_output_gain_set(float gain_db)
{
	kest_fpga_msg msg;
	
	msg.type = KEST_FPGA_MSG_TYPE_SET_OUTPUT_GAIN;
	msg.data.level = gain_db;
	
	return kest_fpga_queue_msg(msg);
}

int kest_fpga_queue_register_commit()
{
	kest_fpga_msg msg;
	
	msg.type = KEST_FPGA_MSG_TYPE_COMMAND;
	msg.data.command = COMMAND_COMMIT_REG_UPDATES;
	
	return kest_fpga_queue_msg(msg);
}

int kest_fpga_queue_read(kest_fpga_read_spec *spec)
{
	if (!spec)
		return ERR_NULL_PTR;
	
	kest_fpga_msg msg;
	
	msg.type = KEST_FPGA_MSG_TYPE_READ;
	msg.data.read = spec;
	
	return kest_fpga_queue_msg(msg);
}

void get_and_sprint_flags(kest_string *str, kest_fpga_status_flags *flags)
{
	if (!str || !flags) return;
	kest_fpga_get_status_flags(flags);
	kest_fpga_status_flags_sprint(str, flags);
	
	kest_puts(*str);
	kest_string_drain(str);
}

void get_and_sprint_flags_compact(kest_string *str, kest_fpga_status_flags *flags)
{
	if (!str || !flags) return;
	uint8_t fb = kest_fpga_read_byte();
	
	kest_fpga_decode_status_flags(flags, fb);
	
	kest_string_appendf(str, "Flags: %s\n", binary_print_8(fb));
}

void sprint_flags_compact(kest_string *str, kest_fpga_status_flags *flags)
{
	if (!str || !flags) return;
	
	uint8_t fb = ((!!flags->initialised) << 0)
			 | ((!!flags->listening)	 << 1)
			 | ((!!flags->timeout) 		 << 2)
			 | ((!!flags->programming) 	 << 3)
			 | ((!!flags->bad) 			 << 4)
			 | ((!!flags->data_ready) 	 << 5)
			 | ((!!flags->cmd_err) 		 << 6)
			 | ((!!flags->swapping) 	 << 7);
	
	kest_string_appendf(str, "\nFlags: %s\n", binary_print_8(fb));
}

void kest_fpga_comms_print_full_scan()
{
	kest_fpga_status_flags flags;
	kest_string str;
	kest_string_init_reserved(&str, 4096);
	
	kest_string_appendf(&str, "\nFPGA Full Status Report\n");
	
	get_and_sprint_flags(&str, &flags);
	
	int64_t n_blocks;
	
	uint64_t sample_count = kest_fpga_get_sample_count(&flags);
	kest_string_appendf(&str, "\nSample count:  %llu\n", sample_count);
	
	//uint32_t stuck_flags = kest_fpga_get_stuck_flags(&flags);
	//kest_string_appendf(&str, "\nStuck flags:  %s\n", binary_print_32(stuck_flags));
	
	kest_puts(str);
	kest_string_drain(&str);
	
	uint64_t sdram_read_count  = kest_fpga_get_sdram_read_count(&flags);
	kest_string_appendf(&str, "\nSDRAM Read count:  %llu\n", sdram_read_count);
	uint64_t sdram_write_count = kest_fpga_get_sdram_write_count(&flags);
	kest_string_appendf(&str, "SDRAM Write count: %llu\n\n", sdram_write_count);
	
	kest_puts(str);kest_string_drain(&str);
	
	n_blocks = kest_fpga_get_n_blocks(&flags);
	if (n_blocks < 0)
	{
		kest_puts(str);
		kest_string_destroy(&str);
		return;
	}
	kest_string_appendf(&str, "Blocks active: %d", n_blocks);
	
	uint32_t instr;
	uint16_t reg_0, reg_1;
	
	for (int i = 0; i < n_blocks; i++)
	{
		instr = kest_fpga_get_block_instr(i, &flags);
		reg_0 = kest_fpga_get_block_reg(i, 0, &flags);
		reg_1 = kest_fpga_get_block_reg(i, 1, &flags);
		
		kest_string_appendf(&str, "\n\t%d: 0x%08x (0x%04x, 0x%04x) ", i, instr, reg_0, reg_1);
		kest_instr_print(&str, instr);
		kest_puts(str);kest_string_drain(&str);
	}
	
	kest_string_appendf(&str, "\n");
	
	
	int n_delays = kest_fpga_get_n_delay_buffers(&flags);
	if (n_delays < 0)
	{
		kest_puts(str);
		kest_string_destroy(&str);
		return;
	}
	
	kest_string_appendf(&str, "\nDelay buffers active: %d\n", n_delays);
	
	int size, delay, addr, pos;
	uint16_t gain;
	uint16_t read_addr, write_addr;
	
	for (int i = 0; i < n_delays; i++)
	{
		kest_string_appendf(&str, "\tDelay buffer %d\n", i);
		size 	= kest_fpga_get_delay_buffer_size(i, &flags);
		kest_string_appendf(&str, "\t\tSize:  %d samples (%d ms),\n", size, (int)(((float)size)/44.1));
		delay 	= kest_fpga_get_delay_buffer_delay(i, &flags);
		kest_string_appendf(&str, "\t\tDelay: %d samples (%d ms),\n", delay, (int)(((float)delay)/44.1));
		addr 	= kest_fpga_get_delay_buffer_addr(i, &flags);
		kest_string_appendf(&str, "\t\tAddr:  0x%06x,\n", addr);
		pos 	= kest_fpga_get_delay_buffer_pos(i, &flags);
		kest_string_appendf(&str, "\t\tPos:   %d = 0x%06x,\n", pos, pos);
		gain 	= kest_fpga_get_delay_buffer_gain(i, &flags);
		kest_string_appendf(&str, "\t\tGain:  %u = %.05f\n", gain, (float)gain * powf(2, -14));
		
		kest_fpga_get_delay_buffer_last_rw_addr(i, &read_addr, &write_addr, &flags);
		kest_string_appendf(&str, "\t\tLast read addr:  0x%04x\n\t\tLast write addr: 0x%04x\n", read_addr, write_addr);
		kest_puts(str);kest_string_drain(&str);
	}
	
	kest_puts(str);
	kest_string_destroy(&str);
	
	if (flags.cmd_err)
		kest_fpga_send_byte(COMMAND_CLEAR_CMD_ERR_FLAG);
}

void kest_fpga_print_command_log()
{
	return;
	
	uint8_t command_buffer[8];
	//kest_fpga_read_command_log(command_buffer);
	
	kest_string str;
	kest_string_init(&str);
	
	kest_string_appendf(&str, "The last 8 commands recognised were, in reverse order, ...");
	
	char *cmd_str;
	for (int i = 0; i < 8; i++)
	{
		cmd_str = kest_fpga_command_to_string(command_buffer[7 - i]);
		kest_string_appendf(&str, "\n\t0x%02x (%s)",   command_buffer[7 - i], cmd_str ? cmd_str : "???");
	}
	
	kest_string_append(&str, '\n');
	
	kest_puts(str);
}

void kest_fpga_handle_misalignment()
{
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	kest_fpga_print_command_log();
}

void kest_command_log_test()
{
	kest_printf("Sending command COMMAND_ALLOC_FILTER\n");
	kest_fpga_send_byte(COMMAND_ALLOC_FILTER);
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	
	
	kest_fpga_send_byte(COMMAND_SET_INPUT_GAIN);
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	
	kest_fpga_send_byte(COMMAND_SET_OUTPUT_GAIN);
	kest_fpga_send_byte(0);
	kest_fpga_send_byte(0);
	
	kest_fpga_print_command_log();
}
