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

typedef struct kest_update_ {
	int type;
	uint8_t data[UPDATE_DATA_SIZE];
} kest_update_;

QueueHandle_t update_queue = NULL;
static int initialised = 0;

#define UPDATE_RATE_HZ 50
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

IMPLEMENT_LIST(kest_fpga_write);
IMPLEMENT_LIST(kest_fpga_alloc);
IMPLEMENT_LIST(kest_fpga_mem_read);

int kest_updater_state_init(kest_updater_state *state)
{
	if (!state)
		return ERR_NULL_PTR;
	
	state->state = KEST_UPDATER_STATE_READY;
	state->active_preset = global_cxt.active_preset;
	kest_fpga_command_list_init(&state->cmds);
	
	kest_fpga_alloc_list_init(&state->allocs);
	
	kest_fpga_write_list_init(&state->instr_writes);
	kest_fpga_write_list_init(&state->reg_writes);
	kest_fpga_write_list_init(&state->filter_writes);
	
	kest_fpga_mem_read_list_init(&state->reads);
	
	kest_fpga_transfer_batch_init(&state->batch);
	state->batch.buffer_owned = 1;
	
	state->tick_ctr = 0;
	
	return NO_ERROR;
}

void kest_updater_state_destroy(kest_updater_state *state)
{
	if (!state)
		return;
	
	kest_fpga_command_list_destroy(&state->cmds);
	
	kest_fpga_alloc_list_destroy(&state->allocs);
	
	kest_fpga_write_list_destroy(&state->instr_writes);
	kest_fpga_write_list_destroy(&state->reg_writes);
	kest_fpga_write_list_destroy(&state->filter_writes);
	
	return;
}

int kest_fpga_write_generate_update(kest_fpga_write *write, kest_dependent *dep, kest_effect *effect)
{
	KEST_PRINTF("kest_fpga_write_generate_update(write = %p, dep = %p, effect = %p)\n",
		write, dep, effect);
	
	if (!write || !dep || !effect)
		return ERR_NULL_PTR;
	
	kest_scope *scope = effect->scope;
	kest_expression *expr = NULL;
	kest_block_reg_val *reg_val = NULL;
	kest_block *block = NULL;
	kest_dsp_resource *res = NULL;
	kest_filter *filter = NULL;
	int res_found = 0;
	
	int addr_1_local = 0;
	
	if (dep->type == KEST_DEPENDENT_BLOCK_REG)
	{
		KEST_PRINTF("dep->type == KEST_DEPENDENT_BLOCK_REG\n");
		write->type = KEST_BLOCK_REG_UPDATE;
		
		addr_1_local = dep->data.block_reg.block;
		
		if (addr_1_local < 0 || addr_1_local >= effect->blocks.count || !(dep->data.block_reg.reg == 0 || dep->data.block_reg.reg == 1))
			return ERR_BAD_ARGS;
		
		if (dep->data.block_reg.reg == 0)
			reg_val = &effect->blocks.entries[addr_1_local].reg_0;
		else if (dep->data.block_reg.reg == 1)
			reg_val = &effect->blocks.entries[addr_1_local].reg_1;
		
		if (!reg_val || !reg_val->expr)
			return ERR_BAD_ARGS;
		
		write->addr_1 = kest_effect_fpga_position_resolve_block(&effect->position_, addr_1_local);
		write->addr_2 = dep->data.block_reg.reg;
		write->format = reg_val->format;
		write->expr = reg_val->expr;
		write->scope = scope;
		
	}
	else if (dep->type == KEST_DEPENDENT_FILTER_COEF)
	{
		KEST_PRINTF("dep->type == KEST_DEPENDENT_FILTER_COEF\n");
		
		write->type = KEST_FILTER_COEF_UPDATE;
		
		addr_1_local = dep->data.filter_coef.filter;
		
		KEST_PRINTF("Searching for filter with handle %d. Effect has %d resources\n", addr_1_local, effect->resources.count);
		
		res_found = 0;
		for (int i = 0; !res_found && i < effect->resources.count; i++)
		{
			res = effect->resources.entries[i];
			
			KEST_PRINTF("effect->resources.entries[%d] = %p\n", i, effect->resources.entries[i]);
			
			if (!res)
			{
				continue;
			}
			
			KEST_PRINTF("Resource %d has type %s, handle %d\n", i, kest_dsp_resource_type_to_string(res->type), res->handle);
			
			if (res->type == KEST_DSP_RESOURCE_FILTER && res->handle == addr_1_local)
				res_found = 1;
		}
		
		if (!res || !res_found)
		{
			KEST_PRINTF("res = %p, res_found = %d\n", res, res_found);
			return ERR_BAD_ARGS;
		}
		
		filter = res->data;
		
		if (!filter)
			return ERR_BAD_ARGS;
		
		if (dep->data.filter_coef.coef < 0 || dep->data.filter_coef.coef >= filter->coefs.count)
			return ERR_BAD_ARGS;
		
		expr = filter->coefs.entries[dep->data.filter_coef.coef];
		
		if (!expr)
			return ERR_BAD_ARGS;
		
		write->addr_1 = kest_effect_fpga_position_resolve_filter(&effect->position_, addr_1_local);
		write->addr_2 = dep->data.filter_coef.coef;
		
		write->format = filter->format;
		write->expr   = expr;
		write->scope  = scope;
	}
	else
	{
		return ERR_BAD_ARGS;
	}
	
	KEST_PRINTF("kest_fpga_write_generate_update done with no error\n");
	return NO_ERROR;
}

int kest_update_task_start()
{
	KEST_PRINTF("kest_update_task_start\n");
	update_queue_ = xQueueCreate(16, sizeof(kest_update));
	
	if (!update_queue_)
		return ERR_UNKNOWN_ERR;
	
	xTaskCreate(kest_update_task, "kest_update_task", 4096, NULL, 8, NULL);
	
	return NO_ERROR;
}

int kest_updater_handle_update(kest_updater_state *state, kest_update update);

int kest_updater_add_reg_update(kest_updater_state *state, kest_dependent *dep, kest_effect *effect)
{
	if (!state || !dep || !effect)
		return ERR_NULL_PTR;
	
	kest_fpga_write write;
	
	int ret_val = kest_fpga_write_generate_update(&write, dep, effect);
	
	if (ret_val != NO_ERROR)
	{
		KEST_PRINTF_FORCE("Error: %s\n", kest_error_code_to_string(ret_val));
		return ret_val;
	}
	
	for (int i = 0; i < state->reg_writes.count; i++)
	{
		if (state->reg_writes.entries[i].addr_1 == write.addr_1 && state->reg_writes.entries[i].addr_2 == write.addr_2)
			return NO_ERROR;
	}
	
	ret_val = kest_fpga_write_list_append(&state->reg_writes, write);
	
	return ret_val;
}

int kest_updater_add_filter_coef_update(kest_updater_state *state, kest_dependent *dep, kest_effect *effect)
{
	if (!state || !dep || !effect)
		return ERR_NULL_PTR;
	
	kest_fpga_write write;
	
	int ret_val = kest_fpga_write_generate_update(&write, dep, effect);
	
	if (ret_val != NO_ERROR)
	{
		KEST_PRINTF_FORCE("Error: %s\n", kest_error_code_to_string(ret_val));
		return ret_val;
	}
	
	for (int i = 0; i < state->filter_writes.count; i++)
	{
		if (state->filter_writes.entries[i].addr_1 == write.addr_1 && state->filter_writes.entries[i].addr_2 == write.addr_2)
			return NO_ERROR;
	}
	
	ret_val = kest_fpga_write_list_append(&state->filter_writes, write);
	
	return ret_val;
}

void kest_updater_print_reg_writes(kest_updater_state *state)
{
	if (!state)
		return;
	
	if (state->reg_writes.count == 0)
		return;
	
	kest_fpga_write_list *writes = &state->reg_writes;
	kest_fpga_write write;
	
	KEST_PRINTF_FORCE("Updater tick %d register writes (n = %d):\n", state->tick_ctr, state->reg_writes.count);
	
	float val;
	
	for (size_t i = 0; i < writes->count; i++)
	{
		write = writes->entries[i];
		
		val = kest_expression_evaluate(write.expr, write.scope);
		KEST_PRINTF_FORCE_("\tBlock %d reg %d, format %d, scope %p, value %s%.04f = %s\n", write.addr_1, write.addr_2, write.format,
			write.scope, val < 0 ? "" : " ", val, kest_expression_to_string(write.expr));
	}
}

void kest_updater_print_filter_writes(kest_updater_state *state)
{
	if (!state)
		return;
		
	if (state->filter_writes.count == 0)
		return;
	
	kest_fpga_write_list *writes = &state->filter_writes;
	kest_fpga_write write;
	
	KEST_PRINTF_FORCE("Updater tick %d filter writes (n = %d):\n", state->tick_ctr, state->filter_writes.count);
	
	float val;
	
	for (size_t i = 0; i < writes->count; i++)
	{
		write = writes->entries[i];
		
		val = kest_expression_evaluate(write.expr, write.scope);
		KEST_PRINTF_FORCE_("\tFilter %d coef %d, format %d, scope %p, value %s%.04f = %s\n", write.addr_1, write.addr_2, write.format,
			write.scope, val < 0 ? "" : " ", val, kest_expression_to_string(write.expr));
	}
}

void kest_updater_print_command_list(kest_updater_state *state)
{
	if (!state)
		return;
	
	if (state->cmds.count == 0)
		return;
	
	kest_fpga_command cmd;
	
	kest_string str;
	kest_string_init(&str);
	
	KEST_PRINTF_FORCE("Updater tick %d commands (n = %d):\n", state->tick_ctr, state->cmds.count);
	
	for (size_t i = 0; i < state->cmds.count; i++)
	{
		cmd = state->cmds.entries[i];
		
		KEST_PRINTF_FORCE_("\tCommand %d: ", i);
		kest_fpga_command_to_string_(cmd, &str);
		KEST_PUTS_FORCE(str);
		kest_string_drain(&str);
		KEST_PRINTF_FORCE_("\n");
	}
}

void kest_updater_print_instr_writes(kest_updater_state *state)
{
	
}

void kest_updater_print_allocs(kest_updater_state *state)
{
	if (!state)
		return;
	
	if (state->allocs.count == 0)
		return;
	
	kest_fpga_alloc alloc;
	
	KEST_PRINTF("Updater tick %d commands (n = %d):\n", state->tick_ctr, state->allocs.count);
	
	for (size_t i = 0; i < state->allocs.count; i++)
	{
		alloc = state->allocs.entries[i];
		
		KEST_PRINTF_("\tAlloc %d: ", i);
		
		switch (alloc.type)
		{
			case KEST_ALLOC_TYPE_DELAY:
				KEST_PRINTF_("Delay (size: %d samples (%.01fms), delay: %d samples (%.01fms))\n",
					alloc.size_1, ((float)alloc.size_1 / (float)KEST_FPGA_SAMPLE_RATE) * 1000.0f,
					alloc.size_2, ((float)alloc.size_2 / (float)KEST_FPGA_SAMPLE_RATE) * 1000.0f);
				break;
			case KEST_ALLOC_TYPE_FILTER:
				KEST_PRINTF_("Filter (order: %d, feed_forward: %d, feed_back: %d, format %d)\n",
					alloc.size_1 + alloc.size_2, alloc.size_1, alloc.size_2, alloc.format);
				break;
		}
	}
}

int kest_updater_drain_lists(kest_updater_state *state)
{
	if (!state)
		return ERR_NULL_PTR;
	
	kest_fpga_batch_drain(&state->batch);
	kest_fpga_command_list_drain(&state->cmds);
	
	kest_fpga_alloc_list_drain(&state->allocs);
	kest_fpga_write_list_drain(&state->instr_writes);
	kest_fpga_write_list_drain(&state->reg_writes);
	kest_fpga_write_list_drain(&state->filter_writes);
	
	return NO_ERROR;
}

int kest_updater_generate_command_list(kest_updater_state *state);
int kest_updater_generate_tx_batch(kest_updater_state *state);
int kest_updater_send(kest_updater_state *state);

void kest_update_task(void *arg)
{
	kest_update update;
	TickType_t last_wake = xTaskGetTickCount();
	
	kest_updater_state state;
	kest_updater_state_init(&state);
	
	while (1)
	{
		while (xQueueReceive(update_queue_, &update, 0) == pdPASS)
			kest_updater_handle_update(&state, update);
		
		#ifdef PRINT_ALLOCS
		kest_updater_print_allocs(&state);
		#endif
		#ifdef PRINT_INSTR_WRITES
		kest_updater_print_instr_writes(&state);
		#endif
		#ifdef PRINT_REG_WRITES
		kest_updater_print_reg_writes(&state);
		#endif
		#ifdef PRINT_FILTER_WRITES
		kest_updater_print_filter_writes(&state);
		#endif
		
		kest_updater_generate_command_list(&state);
		
		#ifdef PRINT_COMMAND_LIST
		kest_updater_print_command_list(&state);
		#endif
		
		kest_updater_generate_tx_batch(&state);
		kest_updater_send(&state);
		
		kest_updater_drain_lists(&state);
		
		state.tick_ctr++;
		
		for (size_t i = 0; i < state.reads.count; i++)
			kest_fpga_queue_read(&state.reads.entries[i].read);
		
		xTaskDelayUntil(&last_wake, update_period_ticks);
	}
	
	vTaskDelete(NULL);
}

int kest_updater_notify_param(kest_parameter *param)
{
	if (!param)
		return ERR_BAD_ARGS;
	
	kest_update update;
	
	update.type = KEST_UPDATE_PARAM;
	update.data.param = param;
	
	if (xQueueSend(update_queue_, &update, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}

int kest_updater_notify_mem(kest_parameter *param)
{
	if (!param)
		return ERR_BAD_ARGS;
	
	kest_update update;
	
	update.type = KEST_UPDATE_PARAM;
	update.data.param = param;
	
	if (xQueueSend(update_queue_, &update, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}

int kest_updater_notify_preset(kest_preset *preset)
{
	if (!preset)
		return ERR_BAD_ARGS;
	
	kest_update update;
	
	update.type = KEST_UPDATE_PRESET;
	update.data.preset = preset;
	
	if (xQueueSend(update_queue_, &update, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}

int kest_update_queue(kest_update update)
{
	if (xQueueSend(update_queue_, &update, pdMS_TO_TICKS(1)) != pdPASS)
		return ERR_CURRENTLY_EXHAUSTED;
	
	return NO_ERROR;
}

int kest_updater_handle_preset_update(kest_updater_state *state, kest_preset *preset);

int kest_updater_handle_update(kest_updater_state *state, kest_update update)
{
	if (!state)
		return ERR_NULL_PTR;
	
	kest_parameter *param = NULL;
	kest_scope_entry *entry = NULL;
	kest_dependent *dep = NULL;
	kest_effect *effect = NULL;
	
	#ifdef PRINTLINES_ALLOWED
	kest_string str;
	kest_string_init(&str);
	#endif
	
	if (state->state == KEST_UPDATER_STATE_REPROGRAM)
		return NO_ERROR;
	
	switch (update.type)
	{
		case KEST_UPDATE_PARAM:
			param = update.data.param;
			
			if (!param)
				break;
			
			KEST_PRINTF("Handling update of parameter \"%s\"...\n", param->name);
			
			effect = param->effect;
			
			if (!effect || !effect->scope)
				break;
			
			entry = kest_scope_lookup(effect->scope, param->name_internal);
			
			if (!entry)
				break;
			
			KEST_PRINTF("Parameter has scope entry %p, with %d dependents\n", entry, entry->dependents.count);
			
			if (entry->dependents.count && PRINTLINES_ALLOWED)
			{
				KEST_PRINTF("Dependents:\n");
			}
			
			for (size_t i = 0; i < entry->dependents.count; i++)
			{
				dep = &entry->dependents.entries[i];
				
				#ifdef PRINTLINES_ALLOWED
				KEST_PRINTF_("\t");
				kest_string_append_dependent(&str, entry->dependents.entries[i]);
				#endif
				
				if (dep->type == KEST_DEPENDENT_BLOCK_REG)
					kest_updater_add_reg_update(state, dep, effect);
				else if (dep->type == KEST_DEPENDENT_FILTER_COEF)
					kest_updater_add_filter_coef_update(state, dep, effect);
				
				#ifdef PRINTLINES_ALLOWED
				KEST_PUTS_(str);
				KEST_PRINTF_("\n");
				kest_string_drain(&str);
				#endif
			}
			
			break;
		
		case KEST_UPDATE_PRESET:
			KEST_PRINTF("oooooogh\n");
			kest_updater_handle_preset_update(state, update.data.preset);
			break;
	}
	
	return NO_ERROR;
}

int kest_alloc_for_filter(kest_fpga_alloc *alloc, kest_dsp_resource *res)
{
	if (!alloc || !res)
		return ERR_NULL_PTR;
	
	kest_filter *filter = (kest_filter*)res->data;
	
	if (!filter)
		return ERR_BAD_ARGS;
	
	alloc->type = KEST_ALLOC_TYPE_FILTER;
	alloc->size_1 = filter->feed_forward;
	alloc->size_2 = filter->feed_back;
	alloc->format = filter->format;
	
	return NO_ERROR;
}

int kest_alloc_for_delay(kest_fpga_alloc *alloc, kest_dsp_resource *res, kest_scope *scope)
{
	if (!alloc || !res || !scope)
		return ERR_NULL_PTR;
	
	kest_delay *delay = (kest_delay*)res->data;
	
	if (!delay)
		return ERR_BAD_ARGS;
	
	float unit_c = 0.001 * KEST_FPGA_SAMPLE_RATE;
	uint32_t delay_samples, delay_size;
	
	switch (delay->units)
	{
		case KEST_DELAY_UNITS_MS:
			KEST_PRINTF("Units: ms\n");
			unit_c = 0.001 * KEST_FPGA_SAMPLE_RATE;
			break;
		case KEST_DELAY_UNITS_SECONDS:
			KEST_PRINTF("Units: s\n");
			unit_c = KEST_FPGA_SAMPLE_RATE;
			break;
		case KEST_DELAY_UNITS_SAMPLES:
			KEST_PRINTF("Units: samples\n");
			unit_c = 1.0;
			break;
	}
	
	delay_samples = (uint32_t)(ceilf(kest_expression_evaluate(delay->delay, scope)) * unit_c);
	
	if (delay->size)
	{
		delay_size = (uint32_t)(ceilf(kest_expression_evaluate(delay->size, scope)) * unit_c);
		
		KEST_PRINTF("delay->size = %s\n", kest_expression_to_string(delay->size));
		
		KEST_PRINTF("delay_size = (uint32_t)(ceilf(kest_expression_evaluate(delay->size, scope)) * unit_c) = ceilf(%f) * %f = %d\n",
			kest_expression_evaluate(delay->size, scope), unit_c, delay_size);
	}
	else
	{
		delay_size = delay_samples + 4;
	}


	if (delay_size < delay_samples + 4)
		delay_size = delay_samples + 4;

	delay_size += 4 - (delay_size % 4);
	
	alloc->type = KEST_ALLOC_TYPE_DELAY;
	alloc->size_1 = delay_size;
	alloc->size_2 = delay_samples;
	
	return NO_ERROR;
}

int kest_alloc_to_command(kest_fpga_command *cmd, kest_fpga_alloc *alloc)
{
	if (!cmd || !alloc)
		return ERR_NULL_PTR;
	
	switch (alloc->type)
	{
		case KEST_ALLOC_TYPE_DELAY:
			*cmd = kest_fpga_command_alloc_delay(alloc->size_1, alloc->size_2);
			break;
		
		case KEST_ALLOC_TYPE_FILTER:
			*cmd = kest_fpga_command_alloc_filter(alloc->format, alloc->size_1, alloc->size_2);
			break;
	}
	
	return NO_ERROR;
}

int kest_updater_handle_preset_update_add_filter_writes(kest_updater_state *state, int handle, kest_filter *filter, kest_scope *scope)
{
	if (!state || !filter)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	kest_fpga_write write;
	
	write.type = KEST_FILTER_COEF_WRITE;
	write.addr_1 = handle;
	write.format = filter->format;
	write.scope = scope;
	
	for (size_t i = 0; i < filter->coefs.count; i++)
	{
		write.addr_2 = i;
		write.expr = filter->coefs.entries[i];
		kest_fpga_write_list_append(&state->filter_writes, write);
	}
	
	return ret_val;
}

int kest_updater_handle_preset_update_add_effect(kest_updater_state *state, kest_effect *effect)
{
	if (!state || !effect)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	kest_dsp_resource *res = NULL;
	
	kest_fpga_alloc alloc;
	kest_fpga_write write;
	kest_fpga_mem_read read;
	kest_block *block;
	
	kest_mem_slot *mem = NULL;
	
	int block_pos = effect->position_.block_start;
	
	for (size_t i = 0; i < effect->resources.count; i++)
	{
		res = effect->resources.entries[i];
		
		if (!res)
			continue;
		
		switch (res->type)
		{
			case KEST_DSP_RESOURCE_DELAY:
				ret_val = kest_alloc_for_delay(&alloc, res, effect->scope);
				
				if (ret_val == NO_ERROR)
					kest_fpga_alloc_list_append(&state->allocs, alloc);
				else
					return ret_val;
				break;

			case KEST_DSP_RESOURCE_FILTER:
				ret_val = kest_alloc_for_filter(&alloc, res);
				
				if (ret_val == NO_ERROR)
					kest_fpga_alloc_list_append(&state->allocs, alloc);
				else
					return ret_val;
				
				kest_updater_handle_preset_update_add_filter_writes(state,
					effect->position_.filter_start + res->handle, (kest_filter*)res->data, effect->scope);
				break;
				
				
			case KEST_DSP_RESOURCE_MEM:
				mem = (kest_mem_slot*)res->data;
				
				if (!mem)
					break;
				
				if (mem->read_enable)
				{
					read.addr = mem->addr + effect->position_.mem_start;
					read.period_ms = mem->read.period_ms;
					read.last_t = 0;
					read.read = mem->read.spec;
				}
				
				kest_fpga_mem_read_list_append(&state->reads, read);
				break;
		}
	}
	
	write.scope = effect->scope;
	
	for (size_t i = 0; i < effect->blocks.count; i++)
	{
		block = &effect->blocks.entries[i];
		write.addr_1 = block_pos + i;
		
		write.type = KEST_BLOCK_INSTR_WRITE;
		
		write.instr = kest_block_instr_encode_positional(block, &effect->position_);
		kest_fpga_write_list_append(&state->instr_writes, write);
		
		write.type = KEST_BLOCK_REG_WRITE;
		if (block->reg_0.active)
		{
			write.addr_2 = 0;
			
			write.expr = block->reg_0.expr;
			write.format = block->reg_0.format;
			kest_fpga_write_list_append(&state->reg_writes, write);
		}
		
		if (block->reg_1.active)
		{
			write.addr_2 = 1;
			
			write.expr = block->reg_1.expr;
			write.format = block->reg_1.format;
			kest_fpga_write_list_append(&state->reg_writes, write);
		}
	}
	
	return ret_val;
}

int kest_updater_clear(kest_updater_state *state)
{
	if (!state)
		return ERR_NULL_PTR;
	
	kest_updater_drain_lists(state);
	kest_fpga_mem_read_list_drain(&state->reads);
	
	return NO_ERROR;
}

int kest_updater_handle_preset_update(kest_updater_state *state, kest_preset *preset)
{
	if (!state)
		return ERR_NULL_PTR;
	
	if (!preset)
		return ERR_BAD_ARGS;
	
	kest_updater_clear(state);
	
	state->state = KEST_UPDATER_STATE_REPROGRAM;
	
	state->active_preset = preset;
	
	kest_effect_pll *current = preset->pipeline.effects;
	
	while (current)
	{
		kest_updater_handle_preset_update_add_effect(state, current->data);
		current = current->next;
	}
	
	return NO_ERROR;
}

int kest_fpga_write_to_command(kest_fpga_command *dest, kest_fpga_write *src)
{
	if (!dest || !src)
		return ERR_NULL_PTR;
	
	int eval_expr = 0;
	float val = 0.0f;
	
	switch (src->type)
	{
		case KEST_BLOCK_REG_WRITE:
		case KEST_BLOCK_REG_UPDATE:
		case KEST_FILTER_COEF_WRITE:
		case KEST_FILTER_COEF_UPDATE:
			eval_expr = 1;
			break;
	}
	
	if (eval_expr)
		val = kest_expression_evaluate(src->expr, src->scope);
	
	switch (src->type)
	{
		case KEST_BLOCK_INSTR_WRITE:
			*dest = kest_fpga_command_write_block_instr(src->addr_1, src->instr);
			break;
			
		case KEST_BLOCK_REG_WRITE:
			if (src->addr_2 == 0)
				*dest = kest_fpga_command_write_block_reg_0(src->addr_1, val, src->format);
			else if (src->addr_2 == 1)
				*dest = kest_fpga_command_write_block_reg_1(src->addr_1, val, src->format);
			else
				return ERR_BAD_ARGS;
			break;

		case KEST_BLOCK_REG_UPDATE:
			if (src->addr_2 == 0)
				*dest = kest_fpga_command_update_block_reg_0(src->addr_1, val, src->format);
			else if (src->addr_2 == 1)
				*dest = kest_fpga_command_update_block_reg_1(src->addr_1, val, src->format);
			else
				return ERR_BAD_ARGS;
			break;

		case KEST_FILTER_COEF_WRITE:
			*dest = kest_fpga_command_write_filter_coef(src->addr_1, src->addr_2, val, src->format);
			break;

		case KEST_FILTER_COEF_UPDATE:
			*dest = kest_fpga_command_update_filter_coef(src->addr_1, src->addr_2, val, src->format);
			break;
		
		default:
			return ERR_BAD_ARGS;
	}
	
	return NO_ERROR;
}

int kest_updater_generate_command_list(kest_updater_state *state)
{
	if (!state)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	int any_reg_updates = (state->reg_writes.count != 0);
	int current_filter_found;
	
	int_list filter_updates;
	int_list_init(&filter_updates);
	
	kest_fpga_alloc *alloc;
	kest_fpga_write *write;
	kest_fpga_command cmd;
	
	for (size_t i = 0; i < state->allocs.count; i++)
	{
		alloc = &state->allocs.entries[i];
		ret_val = kest_alloc_to_command(&cmd, alloc);
		if (ret_val != NO_ERROR)
		{
			KEST_PRINTF_FORCE("Error: %s\n", kest_error_code_to_string(ret_val));
			return ret_val;
		}
		kest_fpga_command_list_append(&state->cmds, cmd);
	}
	
	for (size_t i = 0; i < state->instr_writes.count; i++)
	{
		write = &state->instr_writes.entries[i];
		ret_val = kest_fpga_write_to_command(&cmd, write);
		if (ret_val != NO_ERROR)
		{
			KEST_PRINTF_FORCE("Error: %s\n", kest_error_code_to_string(ret_val));
			return ret_val;
		}
		kest_fpga_command_list_append(&state->cmds, cmd);
	}
	
	for (size_t i = 0; i < state->reg_writes.count; i++)
	{
		write = &state->reg_writes.entries[i];
		ret_val = kest_fpga_write_to_command(&cmd, write);
		if (ret_val != NO_ERROR)
		{
			KEST_PRINTF_FORCE("Error: %s\n", kest_error_code_to_string(ret_val));
			return ret_val;
		}
		kest_fpga_command_list_append(&state->cmds, cmd);
	}
	
	for (size_t i = 0; i < state->filter_writes.count; i++)
	{
		write = &state->filter_writes.entries[i];
		ret_val = kest_fpga_write_to_command(&cmd, write);
		if (ret_val != NO_ERROR)
		{
			KEST_PRINTF_FORCE("Error: %s\n", kest_error_code_to_string(ret_val));
			return ret_val;
		}
		kest_fpga_command_list_append(&state->cmds, cmd);
		
		if (write->type == KEST_FILTER_COEF_UPDATE)
		{
			current_filter_found = 0;
			for (size_t j = 0; !current_filter_found && j < filter_updates.count; j++)
			{
				if (filter_updates.entries[j] == write->addr_1)
					current_filter_found = 1;
			}
			if (!current_filter_found)
				int_list_append(&filter_updates, write->addr_1);
		}
	}
	
	if (any_reg_updates)
		kest_fpga_command_list_append(&state->cmds, kest_fpga_command_commit_reg_updates());
	
	for (size_t i = 0; i < filter_updates.count; i++)
		kest_fpga_command_list_append(&state->cmds, kest_fpga_command_commit_filter_coefs(filter_updates.entries[i]));
	
	return ret_val;
}

int kest_updater_generate_tx_batch(kest_updater_state *state)
{
	if (!state)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	kest_fpga_batch_drain(&state->batch);
	
	kest_fpga_command_list_append_encoded(&state->cmds, &state->batch);
	
	return ret_val;
}


int kest_updater_send(kest_updater_state *state)
{
	if (!state)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	kest_fpga_transfer_batch send_batch;
	
	int len = state->batch.len;
	
	send_batch.buf = kest_alloc(len);
	
	if (!send_batch.buf)
		return ERR_ALLOC_FAIL;
	
	send_batch.buf_len = len;
	send_batch.len = len;
	send_batch.buffer_owned = 0;
	
	memcpy(send_batch.buf, state->batch.buf, len);
	
	switch (state->state)
	{
		case KEST_UPDATER_STATE_READY:
			kest_fpga_queue_transfer_batch(send_batch);
			break;
		
		case KEST_UPDATER_STATE_REPROGRAM:
			kest_fpga_queue_program_batch(send_batch);
			state->state = KEST_UPDATER_STATE_READY;
			break;
	}
	
	return ret_val;
}

kest_fpga_transfer_batch kest_standalone_generate_program_batch(kest_effect_ptr_list *effects)
{
	kest_fpga_transfer_batch batch;
	
	kest_updater_state state;
	kest_updater_state_init(&state);
	
	if (!effects)
		goto standalone_generate_finish;
	
	kest_effect_ptr_list_update_positions(effects);
	
	state.state = KEST_UPDATER_STATE_REPROGRAM;
	
	for (size_t i = 0; i < effects->count; i++)
		kest_updater_handle_preset_update_add_effect(&state, effects->entries[i]);
		
	kest_updater_generate_command_list(&state);
	kest_updater_generate_tx_batch(&state);
	
standalone_generate_finish:
	
	batch = state.batch;
	kest_updater_state_destroy(&state);
	batch.buffer_owned = 0;
	return batch;
}
