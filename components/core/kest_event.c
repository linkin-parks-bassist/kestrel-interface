#include "kest_int.h"

#define PRINTLINES_ALLOWED 1

static const char *FNAME = "kest_event.c";

QueueHandle_t event_queue = NULL;

void kest_event_task(void *arg);

int kest_event_task_start()
{
	event_queue = xQueueCreate(16, sizeof(kest_event));
	
	if (!event_queue)
		return ERR_UNKNOWN_ERR;
	
	xTaskCreate(kest_event_task, "kest_event_task", 4096, NULL, 8, NULL);
	
	return NO_ERROR;
}

void kest_event_handle(kest_event event);

void kest_event_task(void *arg)
{
	kest_event event;
	
	while (xQueueReceive(event_queue, &event, portMAX_DELAY) == pdTRUE)
	{
		kest_event_handle(event);
	}
	
	vTaskDelete(NULL);
}

int kest_event_log(kest_event event)
{
	if (xQueueSend(event_queue, &event, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}

void kest_event_handle(kest_event event)
{
	switch (event.type)
	{
		case KEST_EVENT_STARTUP:
			kest_init();
			break;
		
		case KEST_EVENT_PARAM_CHANGE:
			KEST_PRINTF("tee hee that tickles\n");
			kest_update param_update;
			kest_update_queue(param_update);
			break;
	}
}
