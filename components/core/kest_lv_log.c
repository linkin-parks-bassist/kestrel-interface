#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_lv_log.c";

#ifdef KEST_USE_FREERTOS
#ifdef KEST_ENABLE_LV_LOGGING
SemaphoreHandle_t kest_lv_log_mutex;

static char kest_lv_log_buf[KEST_INT_LV_LOG_BUF_LEN];
static int kest_lv_log_wrapped = 0;
static int kest_lv_log_pos = 0;

char *waiting_buf = NULL;

static const char *TAG = "kest_lv_log.c";

void kest_lv_log_flush_task(void *param);

int kest_log_init()
{
	for (int i = 0; i < KEST_INT_LV_LOG_BUF_LEN; i++)
		kest_lv_log_buf[i] = 0;
	
	kest_lv_log_mutex = xSemaphoreCreateMutex();
	assert(kest_lv_log_mutex != NULL);
	
	xTaskCreate(
		kest_lv_log_flush_task,
		"log_task",
		8192,
		NULL,
		5,                  
		NULL
	);
	
	return NO_ERROR;
}

void kest_lv_log_flush()
{
	char local_buf[KEST_INT_LV_LOG_BUF_LEN];
	int local_buf_position = 0;
	
	if (xSemaphoreTake(kest_lv_log_mutex, pdMS_TO_TICKS(0)) != pdTRUE)
	{
		KEST_PRINTF("Failed to obtain log mutex");
		return;
	}
	
	if (!kest_lv_log_pos && !kest_lv_log_wrapped)
	{
		xSemaphoreGive(kest_lv_log_mutex);
		return;
	}
	
	if (kest_lv_log_wrapped)
	{
		while (local_buf_position + kest_lv_log_pos + 2 < KEST_INT_LV_LOG_BUF_LEN)
		{
			local_buf[local_buf_position] = kest_lv_log_buf[local_buf_position + kest_lv_log_pos + 2];
			local_buf_position++;
		}
	}
	
	for (int i = 0; i < kest_lv_log_pos && local_buf_position + 1 < KEST_INT_LV_LOG_BUF_LEN; i++)
		local_buf[local_buf_position++] = kest_lv_log_buf[i];
	
	local_buf[local_buf_position++] = 0;
	
	puts(local_buf);
	
	for (int i = 0; i < KEST_INT_LV_LOG_BUF_LEN; i++)
		kest_lv_log_buf[i] = 0;
	
	kest_lv_log_pos = 0;
	kest_lv_log_wrapped = 0;
	
	xSemaphoreGive(kest_lv_log_mutex);
}

void kest_lv_log_cb(lv_log_level_t level, const char *buf)
{
	//kest_printf("kest_lv_log_cb\n");
	if (!buf)
	{
		//kest_printf("buf is NULL! returning\n");
		return;
	}
	
	//kest_printf("kest_lv_log_cb. buf = %p", buf);
	
	//if (buf)
	//	KEST_PRINTF(" = %s", buf ? buf : "(NULL)");
	
	//kest_printf("\n");
	if (waiting_buf)
	{
		char *local_waiting_buf = kest_strndup(waiting_buf, KEST_INT_LV_LOG_BUF_LEN);
		
		if (local_waiting_buf)
		{
			kest_free(waiting_buf);
			waiting_buf = NULL;
			kest_lv_log_cb(level, local_waiting_buf);
			kest_free(local_waiting_buf);
		}
	}
	
	if (xSemaphoreTake(kest_lv_log_mutex, pdMS_TO_TICKS(50)) != pdTRUE)
	{
		if (waiting_buf)
			kest_free(waiting_buf);
		waiting_buf = kest_strndup(buf, KEST_INT_LV_LOG_BUF_LEN);
		return;
	}
	
	int len = strlen(buf);
	int new_pos;
	
	for (int i = 0; i < len; i++)
	{
		kest_lv_log_buf[kest_lv_log_pos] = buf[i];
		
		new_pos = (kest_lv_log_pos + 1) % KEST_INT_LV_LOG_BUF_LEN;
		if (new_pos < kest_lv_log_pos)
			kest_lv_log_wrapped = 1;
		kest_lv_log_pos = new_pos;
	}
	
	if (kest_lv_log_pos + 1 < KEST_INT_LV_LOG_BUF_LEN)
		kest_lv_log_buf[kest_lv_log_pos + 1] = 0;
	
	xSemaphoreGive(kest_lv_log_mutex);
}

void kest_lv_log_flush_task(void *param)
{
	TickType_t last_wake = xTaskGetTickCount();

	while (true)
	{
		kest_lv_log_flush();
		vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(100));
	}


	vTaskDelete(NULL);
}

#endif
#endif
