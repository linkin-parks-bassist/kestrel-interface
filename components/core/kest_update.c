#include "kest_int.h"

#define PRINTLINES_ALLOWED 1

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

typedef struct kest_update_ {
	int type;
	uint8_t data[UPDATE_DATA_SIZE];
} kest_update_;

QueueHandle_t update_queue = NULL;
static int initialised = 0;

#define UPDATE_RATE_HZ 2
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
	kest_update_ update;
	kest_effect_pll *current_effect = NULL;
	kest_effect *effect = NULL;
	kest_mem_slot *mem = NULL;
	kest_lfo *lfo = NULL;
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
						case KEST_DSP_RESOURCE_LFO:
							lfo = (kest_lfo*)resource->data;
							if (lfo->scope_entry)
								lfo->scope_entry->updated = 1;
							kest_active_preset_updater_notify_effect_by_ptr(current_effect->data);
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
	return;
	
	update_queue = xQueueCreate(16, sizeof(kest_update_));
	xTaskCreate(kest_active_preset_updater_task, "kest_update_task", 4096, NULL, 8, NULL);
}

int kest_active_preset_updater_notify_effect_by_id(int preset_id, int effect_id)
{
	if (!initialised)
		return ERR_UNINITIALISED;
	
	kest_update_ update;
	kest_effect_update *effect_update = (kest_effect_update*)&update.data;
	
	memset(&update, 0, sizeof(kest_update_));
	
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
	
	kest_update_ update;
	kest_effect_update *effect_update = (kest_effect_update*)&update.data;
	
	memset(&update, 0, sizeof(kest_update_));
	
	update.type = KEST_UPDATE_TYPE_EFFECT;
	effect_update->type = 0;
	effect_update->ref_type = KEST_EFFECT_UPDATE_BY_PTR;
	effect_update->ptr = effect;
	
	if (xQueueSend(update_queue, &update, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}

/********************************************************************
 ********************************************************************
 ********************************************************************
 ********************************************************************
 ********************************************************************
 ********************************************************************
 ********************************************************************
 ********************************************************************
 ********************************************************************
 ********************************************************************
 ********************************************************************
 ********************************************************************
 ********************************************************************/

QueueHandle_t update_queue_ = NULL;

void kest_update_task(void *arg);

#define KEST_UPDATER_STATE_READY 0

typedef struct {
	int state;
	
	kest_preset *active_preset;
	kest_fpga_command_list cmds;
} kest_updater_state;

int kest_update_task_start()
{
	KEST_PRINTF("kest_update_task_start\n");
	update_queue_ = xQueueCreate(16, sizeof(kest_update));
	
	if (!update_queue_)
		return ERR_UNKNOWN_ERR;
	
	xTaskCreate(kest_update_task, "kest_update_task", 4096, NULL, 8, NULL);
	
	return NO_ERROR;
}

int kest_update_handle(kest_updater_state *state, kest_update update);

int kest_updater_state_init(kest_updater_state *state)
{
	if (!state)
		return ERR_NULL_PTR;
	
	state->state = KEST_UPDATER_STATE_READY;
	state->active_preset = global_cxt.active_preset;
	kest_fpga_command_list_init(&state->cmds);
	
	return NO_ERROR;
}

void kest_update_task(void *arg)
{
	kest_update update;
	TickType_t last_wake = xTaskGetTickCount();
	
	kest_updater_state state;
	kest_updater_state_init(&state);
	
	while (1)
	{
		while (xQueueReceive(update_queue_, &update, 0) == pdPASS)
		{
			kest_update_handle(&state, update);
		}
		
		xTaskDelayUntil(&last_wake, update_period_ticks);
	}
	
	vTaskDelete(NULL);
}

int kest_update_queue(kest_update update)
{
	if (xQueueSend(update_queue_, &update, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}


int kest_update_handle(kest_updater_state *state, kest_update update)
{
	if (!state)
		return ERR_NULL_PTR;
	
	kest_parameter *param = NULL;
	kest_scope_entry *entry = NULL;
	kest_string str;
	kest_string_init(&str);
	
	switch (update.type)
	{
		case KEST_UPDATE_PARAM:
			param = update.data.param;
			
			if (!param)
				break;
			
			KEST_PRINTF("Handling update of parameter \"%s\"...\n", param->name);
			entry = param->scope_entry;
			
			if (!entry)
				break;
			
			KEST_PRINTF("parameter has scope entry %p, with %d dependents\n", entry, entry->dependents.count);
			
			if (entry->dependents.count)
				KEST_PRINTF("Dependents:\n");
			
			for (size_t i = 0; i < entry->dependents.count; i++)
			{
				kest_string_append(&str, '\t');
				kest_string_append_dependent(&str, entry->dependents.entries[i]);
				kest_string_append(&str, '\n');
				KEST_PUTS(str);
				kest_string_drain(&str);
			}
			
			break;
	}
	
	return NO_ERROR;
}
