#include "kest_int.h"

static const char *FNAME = "kest_fpga_update.c";

#define PRINTLINES_ALLOWED 0

void kest_fpga_update_task(void *arg);

#define UPDATE_RATE_HZ 100
#define UPDATE_PERIOD_MS (1000.0f / (float)UPDATE_RATE_HZ)

static const int update_period_ticks = (pdMS_TO_TICKS((int)UPDATE_PERIOD_MS) == 0) ? 1 : pdMS_TO_TICKS((int)UPDATE_PERIOD_MS);

TaskHandle_t kest_fpga_updater_task_handle;
int initialised = 0;

int kest_init_fpga_updater()
{
	return ERR_FEATURE_DISABLED;
	
	xTaskCreate(kest_fpga_update_task, "kest_fpga_update_task", 4096, NULL, 8, &kest_fpga_updater_task_handle);
	return NO_ERROR;
}

void kest_fpga_update_task(void *arg)
{
	initialised = 1;
	TickType_t last_wake = xTaskGetTickCount();
	
	uint32_t n;
	
	kest_preset *active_preset = NULL;
	
	while (1)
	{
		n = ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
		
		if (n == 0) goto nevermind;
		
		#ifdef KEST_ENABLE_GLOBAL_CONTEXT
		active_preset = global_cxt.active_preset;
		#endif
		
		if (!active_preset) goto nevermind;
		
		kest_preset_if_active_update_fpga(active_preset);
		
nevermind:
		xTaskDelayUntil(&last_wake, update_period_ticks);
	}
}

int kest_fpga_updater_wake()
{
	if (initialised)
	{
		xTaskNotifyGive(kest_fpga_updater_task_handle);
	}
	else
	{
		return ERR_CURRENTLY_EXHAUSTED;
	}
	
	return NO_ERROR;
}
