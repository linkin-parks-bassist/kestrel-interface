#include "kest_int.h"

static const char *FNAME = "kest_file_task.c";

#define PRINTLINES_ALLOWED 0

#define KEST_FILE_JOB_NONE 			0
#define KEST_FILE_JOB_SAVE_STATE 	1
#define KEST_FILE_JOB_SAVE_PRESET 	2
#define KEST_FILE_JOB_SAVE_SEQUENCE 3

typedef struct {
	int type;
	
	union {
		kest_state state;
		kest_preset *preset;
		kest_sequence *sequence;
	} data;
} kest_file_job;

volatile int job_queue_initd = 0;
QueueHandle_t file_job_queue;

void kest_file_task(void *)
{
	file_job_queue = xQueueCreate(16, sizeof(kest_file_job));
	job_queue_initd = 1;
	
	TickType_t last_wake = xTaskGetTickCount();
	
	kest_file_job job;
	
	BaseType_t queue_ret;
	int ret_val;
	kest_state state;
	
	while (1)
	{
		queue_ret = xQueueReceive(file_job_queue, &job, portMAX_DELAY);
		
		if (queue_ret != pdPASS)
			continue;
		
		switch (job.type)
		{
			case KEST_FILE_JOB_SAVE_STATE:
					kest_state_save(job.data.state);
				break;
			
			case KEST_FILE_JOB_SAVE_PRESET:
					save_preset(job.data.preset);
				break;
			
			case KEST_FILE_JOB_SAVE_SEQUENCE:
					save_sequence(job.data.sequence);
				break;
		}
	}
}

int kest_init_file_task()
{
	xTaskCreate(kest_file_task, "kest_param_update_task", 4096, NULL, 8, NULL);
	return NO_ERROR;
}

#define KEST_STATE_SAVE_MIN_TIME_MS 3000

int kest_queue_state_save()
{
	/* Avoid saving state to disk if the system hasn't been running
	 * for a few seconds. At this time, it's possible that the state 
	 * isn't fully initialised. Doing it here is easier than
	 * making a mess of other functions to try and avoid errant state saves */
	
	int64_t time = kest_system_time_ms();
	
	if (time < KEST_STATE_SAVE_MIN_TIME_MS)
		return NO_ERROR;
	
	int ret_val;
	kest_state state;
	
	memset(&state, 0, sizeof(kest_state));
	
	if ((ret_val = kest_cxt_clone_state(&global_cxt, &state)) != NO_ERROR)
	{
		KEST_PRINTF("Failed to clone state; aborting\n");
		return ret_val;
	}
	
	kest_file_job job;
	job.type = KEST_FILE_JOB_SAVE_STATE;
	job.data.state = state;
	
	if (!job_queue_initd)
		return ERR_UNINITIALISED;
	
	if (xQueueSend(file_job_queue, &job, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}

int kest_queue_preset_save(kest_preset *preset)
{
	int ret_val = NO_ERROR;

	kest_file_job job;
	job.type = KEST_FILE_JOB_SAVE_PRESET;
	job.data.preset = preset;
	
	if (!job_queue_initd)
		return ERR_UNINITIALISED;
	
	if (xQueueSend(file_job_queue, &job, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}

int kest_queue_sequence_save(kest_sequence *sequence)
{
	int ret_val = NO_ERROR;

	kest_file_job job;
	job.type = KEST_FILE_JOB_SAVE_SEQUENCE;
	job.data.sequence = sequence;
	
	if (!job_queue_initd)
		return ERR_UNINITIALISED;
	
	if (xQueueSend(file_job_queue, &job, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}
