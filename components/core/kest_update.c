#include "kest_int.h"

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "kest_update.c";

#define UPDATE_DATA_SIZE 32

#define KEST_EFFECT_UPDATE_BY_ID  0
#define KEST_EFFECT_UPDATE_BY_PTR 1

typedef struct kest_effect_update {
	int type;
	int ref_type;
	int preset_id;
	int effect_id;
	kest_effect *ptr;
} kest_effect_update;

#define KEST_UPDATE_TYPE_NONE 	0
#define KEST_UPDATE_TYPE_EFFECT 1

typedef struct kest_update {
	int type;
	uint8_t data[UPDATE_DATA_SIZE];
} kest_update;

QueueHandle_t update_queue = NULL;
static int initialised = 0;

#define UPDATE_RATE_HZ 200
#define UPDATE_PERIOD_MS (1000.0f / (float)UPDATE_RATE_HZ)

static const int update_period_ticks = (pdMS_TO_TICKS((int)UPDATE_PERIOD_MS) == 0) ? 1 : pdMS_TO_TICKS((int)UPDATE_PERIOD_MS);

void kest_active_preset_updater_task(void *arg)
{
	if (!update_queue)
	{
		KEST_PRINTF("Update queue failed to initialise. Aborting\n");
		vTaskDelete(NULL);
		return;
	}
	
	initialised = 1;
	
	TickType_t last_wake = xTaskGetTickCount();
	kest_effect_update *effect_update;
	kest_update update;
	kest_effect_pll *current_effect = NULL;
	kest_effect *effect = NULL;
	kest_mem_slot *mem = NULL;
	kest_dsp_resource *resource = NULL;
	kest_preset *active_preset = NULL;
	kest_preset *active_preset_prev = NULL;
	
	kest_effect_ptr_list updated_effects;
	kest_effect_ptr_list_init(&updated_effects);
	
	int skip;
	
	while (1)
	{
		KEST_PRINTF("Updater awoken\n");
		active_preset = global_cxt.active_preset;
		
		KEST_PRINTF("active_preset = %p\n", active_preset);
		
		if (!active_preset)
			goto updater_sleep;
		
		KEST_PRINTF("active_preset->id = %d\n", active_preset->id);
		KEST_PRINTF("active_preset->alive = %d, active_preset->active = %d, active_preset->pending = %d\n",
			active_preset->alive, active_preset->active, active_preset->pending);
		
		if (!active_preset->alive || !active_preset->active || active_preset->pending)
			goto updater_sleep;
		
		if (active_preset != active_preset_prev)
			KEST_PRINTF("Preset change detected !\n");
		
		current_effect = active_preset->pipeline.effects;
		
		while (current_effect)
		{
			if (current_effect->data)
			{
				effect = current_effect->data;
				
				for (size_t i = 0; i < effect->resources.count; i++)
				{
					resource = effect->resources.entries[i];
					
					if (!resource)
						continue;
					
					switch (resource->type)
					{
						case KEST_DSP_RESOURCE_MEM:
							mem = (kest_mem_slot*)resource->data;
							if (mem->read_enable)
							{
								KEST_PRINTF("Dispatching mem slot %p read\n", mem);
								kest_periodic_read_dispatch(&mem->read);
							}
							break;
					}
				}
			}
			
			current_effect = current_effect->next;
		}
		
		
		
		while (xQueueReceive(update_queue, &update, 0) == pdPASS)
		{
			skip = 0;
			switch (update.type)
			{
				case KEST_UPDATE_TYPE_EFFECT:
					effect_update = (kest_effect_update*)&update.data;
					
					switch (effect_update->ref_type)
					{
						case KEST_EFFECT_UPDATE_BY_ID:
							if (effect_update->preset_id == active_preset->id)
							{
								effect = kest_preset_get_effect_by_id(active_preset, effect_update->effect_id);
								
								if (!effect)
									skip = 1;
							}
							else
							{
								skip = 1;
							}
							break;
						
						case KEST_EFFECT_UPDATE_BY_PTR:
							effect = effect_update->ptr;
							
							if (!effect)
							{
								KEST_PRINTF("Effect updates by ptr reference, but the pointer is NULL! Ignoring\n");
								skip = 1;
							}
							
							if (effect->preset != active_preset)
							{
								KEST_PRINTF("The update applies outside the active preset! Ignoring\n");
								skip = 1;
							}
							
							break;
					}
					
					if (!skip)
					{
						
						KEST_PRINTF("The update applies to an effect in the active preset! Obtained pointer %p\n", effect);
						
						if (!kest_effect_ptr_list_contains(&updated_effects, effect))
							kest_effect_ptr_list_append(&updated_effects, effect);
					}
						
					break;
				
				default:
					break;
			}
		}
		
		KEST_PRINTF("There are %d effects that have been updated in the active preset in the last %dms\n", updated_effects.count, (int)UPDATE_PERIOD_MS);
			
		for (size_t i = 0; i < updated_effects.count; i++)
		{
			effect = updated_effects.entries[i];
			
			if (!effect)
				continue;
			
			if (global_cxt.pages.current_page == effect->view_page)
				kest_effect_handle_updates_inc_ui(effect);
			else
				kest_effect_handle_updates(effect);
		}
		
	updater_sleep:
		kest_effect_ptr_list_drain(&updated_effects);
		active_preset_prev = active_preset;
		xTaskDelayUntil(&last_wake, update_period_ticks);
	}
}

void kest_active_preset_updater_start()
{
	update_queue = xQueueCreate(16, sizeof(kest_update));
	xTaskCreate(kest_active_preset_updater_task, "kest_update_task", 4096, NULL, 8, NULL);
}

int kest_active_preset_updater_notify_effect_by_id(int preset_id, int effect_id)
{
	if (!initialised)
		return ERR_UNINITIALISED;
	
	kest_update update;
	kest_effect_update *effect_update = (kest_effect_update*)&update.data;
	
	memset(&update, 0, sizeof(kest_update));
	
	update.type = KEST_UPDATE_TYPE_EFFECT;
	effect_update->type = 0;
	effect_update->ref_type = KEST_EFFECT_UPDATE_BY_ID;
	effect_update->preset_id = preset_id;
	effect_update->effect_id = effect_id;
	
	if (xQueueSend(update_queue, &update, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}

int kest_active_preset_updater_notify_effect_by_ptr(kest_effect *effect)
{
	if (!initialised)
		return ERR_UNINITIALISED;
	
	if (!effect)
		return ERR_NULL_PTR;
	
	kest_update update;
	kest_effect_update *effect_update = (kest_effect_update*)&update.data;
	
	memset(&update, 0, sizeof(kest_update));
	
	update.type = KEST_UPDATE_TYPE_EFFECT;
	effect_update->type = 0;
	effect_update->ref_type = KEST_EFFECT_UPDATE_BY_PTR;
	effect_update->ptr = effect;
	
	if (xQueueSend(update_queue, &update, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}
