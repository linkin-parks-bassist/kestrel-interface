#include "m_int.h"

static const char *FNAME = "m_fpga_comms.c";

#define M_FPGA_MSG_TYPE_BATCH 			0
#define M_FPGA_MSG_TYPE_PROGRAM_BATCH 	1
#define M_FPGA_MSG_TYPE_SET_INPUT_GAIN 	2
#define M_FPGA_MSG_TYPE_SET_OUTPUT_GAIN 3
#define M_FPGA_MSG_TYPE_COMMAND			4

#define FPGA_BOOT_MS 2000

typedef struct {
	int type;
	
	union {
		float level;
		uint8_t command;
		m_fpga_transfer_batch batch;
	} data;
} m_fpga_msg;

static QueueHandle_t fpga_msg_queue;
static int initialised = 0;

#define PROGRAM_RETRIES 3

void m_fpga_comms_task(void *param)
{
	m_fpga_spi_init();
	
	fpga_msg_queue = xQueueCreate(32, sizeof(m_fpga_msg));
	initialised = 1;
	
	vTaskDelay(pdMS_TO_TICKS(FPGA_BOOT_MS));
	
	uint8_t byte;
	byte = m_fpga_read_byte();
	
	m_printf("Starting FPGA comms. FPGA reports status code %d\n", byte);
	
	m_fpga_set_input_gain(global_cxt.input_gain.value);
	m_fpga_set_output_gain(global_cxt.output_gain.value);
	
	int program_check_ms = 5;
	int program_check_ticks = 5 / portTICK_PERIOD_MS;
	int program_check_delay = (program_check_ticks == 0) ? 1 : program_check_ticks;
	
	
	m_fpga_msg msg;
	
	while (1)
	{
		xQueueReceive(fpga_msg_queue, &msg, portMAX_DELAY);
		
		switch (msg.type)
		{
			case M_FPGA_MSG_TYPE_PROGRAM_BATCH:
				#ifdef PRINT_TRANSFER_BATCHES
				m_fpga_batch_print(msg.data.batch);
				#endif
				for (int i = 0; i < PROGRAM_RETRIES; i++)
				{
					m_fpga_transfer_batch_send(msg.data.batch);
					vTaskDelay(program_check_delay);
					byte = m_fpga_read_byte();
					
					if (byte != SPI_RESPONSE_OK)
					{
						m_printf("FPGA responded with code %d after programming. Retrying...\n", byte);
					}
					else
					{
						m_printf("FPGA accepted the new pipeline :)\n");
						break;
					}
				}
				
				m_free_fpga_transfer_batch(msg.data.batch);
				break;
			
			case M_FPGA_MSG_TYPE_BATCH:
				#ifdef PRINT_TRANSFER_BATCHES
				m_fpga_batch_print(msg.data.batch);
				#endif
				m_fpga_transfer_batch_send(msg.data.batch);
				m_free_fpga_transfer_batch(msg.data.batch);
				break;
			
			case M_FPGA_MSG_TYPE_SET_INPUT_GAIN:
				m_fpga_set_input_gain(msg.data.level);
				break;
			
			case M_FPGA_MSG_TYPE_SET_OUTPUT_GAIN:
				m_fpga_set_output_gain(msg.data.level);
				break;
				
			case M_FPGA_MSG_TYPE_COMMAND:
				#ifdef PRINT_COMMANDS
				m_printf("send FPGA command %s\n", m_fpga_command_to_string(msg.data.command));
				#endif
				m_fpga_send_byte(msg.data.command);
				break;
		}
	}
	
	vTaskDelete(NULL);
}

static inline int m_fpga_queue_msg(m_fpga_msg msg)
{
	while (!initialised);
	
	if (xQueueSend(fpga_msg_queue, (void*)&msg, (TickType_t)1) != pdPASS)
	{
		return ERR_QUEUE_SEND_FAILED;
	}
	return NO_ERROR;
}

int m_fpga_queue_transfer_batch(m_fpga_transfer_batch batch)
{
	m_fpga_msg msg;
	
	msg.type = M_FPGA_MSG_TYPE_BATCH;
	msg.data.batch = batch;
	
	int ret_val = m_fpga_queue_msg(msg);
	
	return ret_val;
}

int m_fpga_queue_program_batch(m_fpga_transfer_batch batch)
{
	m_fpga_msg msg;
	
	msg.type = M_FPGA_MSG_TYPE_PROGRAM_BATCH;
	msg.data.batch = batch;
	
	int ret_val = m_fpga_queue_msg(msg);
	
	return ret_val;
}

int m_fpga_queue_input_gain_set(float gain_db)
{
	m_fpga_msg msg;
	
	msg.type = M_FPGA_MSG_TYPE_SET_INPUT_GAIN;
	msg.data.level = gain_db;
	
	return m_fpga_queue_msg(msg);
}

int m_fpga_queue_output_gain_set(float gain_db)
{
	m_fpga_msg msg;
	
	msg.type = M_FPGA_MSG_TYPE_SET_OUTPUT_GAIN;
	msg.data.level = gain_db;
	
	return m_fpga_queue_msg(msg);
}

int m_fpga_queue_register_commit()
{
	m_fpga_msg msg;
	
	msg.type = M_FPGA_MSG_TYPE_COMMAND;
	msg.data.command = COMMAND_COMMIT_REG_UPDATES;
	
	return m_fpga_queue_msg(msg);
}
