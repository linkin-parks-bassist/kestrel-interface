#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_fpga_comms.c";

#define KEST_FPGA_MSG_TYPE_BATCH 			0
#define KEST_FPGA_MSG_TYPE_PROGRAM_BATCH 	1
#define KEST_FPGA_MSG_TYPE_SET_INPUT_GAIN 	2
#define KEST_FPGA_MSG_TYPE_SET_OUTPUT_GAIN  3
#define KEST_FPGA_MSG_TYPE_COMMAND			4

typedef struct {
	int type;
	
	union {
		float level;
		uint8_t command;
		kest_fpga_transfer_batch batch;
	} data;
} kest_fpga_msg;

static QueueHandle_t fpga_msg_queue;
static int initialised = 0;

#define PROGRAM_RETRIES 3

void kest_fpga_comms_task(void *param)
{
	kest_fpga_spi_init();
	
	fpga_msg_queue = xQueueCreate(32, sizeof(kest_fpga_msg));
	initialised = 1;
	
	vTaskDelay(pdMS_TO_TICKS(FPGA_BOOT_MS));
	
	kest_fpga_status_flags status;
	
	int status_check_ms = 5;
	int status_check_ticks = 5 / portTICK_PERIOD_MS;
	int status_check_delay = (status_check_ticks == 0) ? 1 : status_check_ticks;
	
	do {
		kest_fpga_get_status_flags(&status);
		vTaskDelay(status_check_delay);
	} while (!status.initialised);
	
	uint8_t byte;
	
	kest_fpga_status_flags_print(&status);
	
	kest_fpga_msg msg;
	BaseType_t ret;
	
	while (1)
	{
		do {
			ret = xQueueReceive(fpga_msg_queue, &msg, pdMS_TO_TICKS(1000));
			if (ret != pdPASS)
			{
				kest_fpga_get_status_flags(&status);
				#ifdef PRINT_STATUS_CODES
				kest_fpga_status_flags_print(&status);
				#endif
			}
		} while (ret != pdPASS);
		
		switch (msg.type)
		{
			case KEST_FPGA_MSG_TYPE_PROGRAM_BATCH:
				#ifdef PRINT_TRANSFER_BATCHES
				kest_fpga_batch_print(msg.data.batch);
				#endif
				#ifndef KEST_FPGA_SIMULATED
				for (int i = 0; i < PROGRAM_RETRIES; i++)
				{
					kest_fpga_transfer_batch_send(msg.data.batch);
					vTaskDelay(status_check_delay);
					byte = kest_fpga_read_byte();
					
					if (byte != SPI_RESPONSE_OK)
					{
						KEST_PRINTF("FPGA responded with code %d after programming. Retrying...\n", byte);
					}
					else
					{
						KEST_PRINTF("FPGA accepted the new pipeline :)\n");
						break;
					}
				}
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
				KEST_PRINTF("Set FPGA input gain to %f\n", msg.data.level);
				#endif
				kest_fpga_set_input_gain(msg.data.level);
				break;
			
			case KEST_FPGA_MSG_TYPE_SET_OUTPUT_GAIN:
				#ifdef PRINT_COMMANDS
				KEST_PRINTF("Set FPGA output gain to %f\n", msg.data.level);
				#endif
				kest_fpga_set_output_gain(msg.data.level);
				break;
				
			case KEST_FPGA_MSG_TYPE_COMMAND:
				#ifdef PRINT_COMMANDS
				KEST_PRINTF("send FPGA command %s\n", kest_fpga_command_to_string(msg.data.command));
				#endif
				kest_fpga_send_byte(msg.data.command);
				break;
		}
		
		byte = kest_fpga_read_byte();
		#ifdef PRINT_STATUS_CODES
		KEST_PRINTF("FPGA reports status code %d\n", byte);
		#endif
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
