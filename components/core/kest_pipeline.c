#include "kest_int.h"

//#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
//#endif

static const char *FNAME = "kest_pipeline.c";

int init_m_pipeline(kest_pipeline *pipeline)
{
	if (!pipeline)
		return ERR_NULL_PTR;
	
	pipeline->effects = NULL;
	
	#ifdef KEST_USE_FREERTOS
	pipeline->mutex = xSemaphoreCreateMutex();
	#endif
	
	return NO_ERROR;
}

kest_effect *kest_pipeline_append_effect_eff(kest_pipeline *pipeline, kest_effect_desc *eff)
{
	KEST_PRINTF("kest_pipeline_append_effect_eff(pipeline = %p, eff = %p)\n", pipeline, eff);
	if (!pipeline || !eff)
		return NULL;
	
	kest_effect *effect = kest_allocator_alloc(&kest_effect_allocator, sizeof(kest_effect));
	
	if (!effect)
		return NULL;
	
	kest_effect_pll *node = kest_alloc(sizeof(kest_effect_pll));
	
	if (!node)
	{
		kest_allocator_free(&kest_effect_allocator, effect);
		return NULL;
	}
	
	node->data = effect;
	node->next = NULL;
	
	if (init_effect_from_effect_desc(effect, eff) != NO_ERROR)
	{
		kest_effect_free_retired(effect);
		kest_free(node);
		return NULL;
	}
	
	if (!pipeline->effects)
	{
		effect->id = 0;
		pipeline->effects = node;
	}
	else
	{
		int least_free_id = 0;
		kest_effect_pll *current = pipeline->effects;
		
		while (current)
		{
			if (current->data)
			{
				if (current->data->id >= least_free_id)
					least_free_id = current->data->id + 1;
			}
			
			if (current->next)
				current = current->next;
			else
				break;
		}
		
		effect->id = least_free_id;
		current->next = node;
	}
	
	KEST_PRINTF("kest_pipeline_append_effect_eff done\n");
	return effect;
}

int kest_pipeline_remove_effect(kest_pipeline *pipeline, uint16_t id)
{
	KEST_PRINTF("kest_pipeline_remove_effect\n");
	if (!pipeline)
		return ERR_NULL_PTR;
	
	kest_effect_pll *current = pipeline->effects;
	kest_effect_pll *prev = NULL;
	
	while (current)
	{
		if (current->data && current->data->id == id)
		{
			kest_effect_disable(current->data);
			free_effect(current->data);
			
			if (prev)
				prev->next = current->next;
			else
				pipeline->effects = current->next;
			
			kest_free(current);
			
			KEST_PRINTF("kest_pipeline_remove_effect found and vanquished the effect\n");
			return NO_ERROR;
		}
		
		prev = current;
		current = current->next;
	}
	
	
	KEST_PRINTF("kest_pipeline_remove_effect finished without finding the effect\n");
	return ERR_INVALID_EFFECT_ID;
}

int kest_pipeline_move_effect(kest_pipeline *pipeline, int new_pos, int old_pos)
{
	if (!pipeline)
		return ERR_NULL_PTR;
	
	if (!pipeline->effects)
		return ERR_BAD_ARGS;
	
	kest_effect_pll *target  = NULL;
	
	int i = 0;
	kest_effect_pll *current = pipeline->effects;
	kest_effect_pll *prev    = NULL;
	
	while (current && i < old_pos)
	{
		prev = current;
		current = current->next;
		i++;
	}
	
	if (!current)
		return ERR_BAD_ARGS;
	
	target = current;
	
	if (prev)
		prev->next = target->next;
	else
		pipeline->effects = target->next;

	i = 0;
	prev = NULL;
	current = pipeline->effects;
	
	while (current && i < new_pos)
	{
		prev = current;
		current = current->next;
		i++;
	}
	
	target->next = current;
	
	if (!prev)
		pipeline->effects = target;
	else
		prev->next = target;
	
	return NO_ERROR;
}

int kest_pipeline_get_n_effects(kest_pipeline *pipeline)
{
	if (!pipeline)
		return -ERR_NULL_PTR;
	
	int n = 0;
	
	kest_effect_pll *current = pipeline->effects;
	
	while (current)
	{
		if (current->data)
			n++;
		current = current->next;
	}
	
	return n;
}

int clone_pipeline(kest_pipeline *dest, kest_pipeline *src)
{
	if (!src || !dest)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("Cloning pipeline...\n");
	
	kest_effect_pll *current = src->effects;
	kest_effect_pll *nl;
	kest_effect *effect = NULL;
	
	int i = 0;
	while (current)
	{
		KEST_PRINTF("Cloning effect %d... current = %p, current->next = %p\n", i, current, current->next);
		if (current->data)
		{
			effect = kest_allocator_alloc(&kest_effect_allocator, sizeof(kest_effect));
			
			if (!effect)
				return ERR_ALLOC_FAIL;
			
			clone_effect(effect, current->data);
			
			nl = kest_effect_pll_append(dest->effects, effect);
		
			if (nl)
				dest->effects = nl;
		}
		
		current = current->next;
		i++;
	}
	
	return NO_ERROR;
}

void gut_pipeline(kest_pipeline *pipeline)
{
	if (!pipeline)
		return;
	
	kest_effect_pll_destroy(pipeline->effects, free_effect);
	pipeline->effects = NULL;
}

int kest_pipeline_check_capacity(const kest_pipeline *pipeline, unsigned int extra_blocks)
{
	if (!pipeline) return ERR_NULL_PTR;
	unsigned int remaining = KEST_FPGA_N_BLOCKS;
	for (kest_effect_pll *node = pipeline->effects; node; node = node->next)
	{
		kest_effect *effect = node->data;
		if (!effect || !effect->eff) return ERR_BAD_ARGS;
		if (effect->blocks.count > remaining || effect->eff->res_rpt.blocks > remaining)
			return ERR_PIPELINE_FULL;
		remaining -= effect->eff->res_rpt.blocks;
	}
	return extra_blocks > remaining ? ERR_PIPELINE_FULL : NO_ERROR;
}

int kest_pipeline_create_fpga_transfer_batch(kest_pipeline *pipeline, kest_fpga_transfer_batch *batch)
{
	KEST_PRINTF("kest_pipeline_create_fpga_transfer_batch(pipeline = %p, batch = %p)\n", pipeline, batch);
	if (!batch)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	if (!pipeline)
	{
		ret_val = ERR_BAD_ARGS;
		goto return_nothing;
	}

	/* Check the whole chain before encoding changes any effect's position. */
	ret_val = kest_pipeline_check_capacity(pipeline, 0);
	if (ret_val != NO_ERROR) goto return_nothing;
	
	kest_fpga_transfer_batch result = kest_new_fpga_transfer_batch();
	
	kest_eff_resource_report rpt = empty_m_eff_resource_report();
	
	int pos = 0;
	if (pipeline->effects)
		ret_val = kest_fpga_batch_append_effects(&result, pipeline->effects, &rpt, &pos);
	
	kest_fpga_batch_append(&result, COMMAND_ENABLE_TAIL);
	
	if (ret_val != NO_ERROR)
	{
		kest_free_fpga_transfer_batch(result);
		goto return_nothing;
	}
	
	*batch = result;
	
	KEST_PRINTF("kest_pipeline_create_fpga_transfer_batch done (%s)\n", kest_error_code_to_string(ret_val));
	return ret_val;
	
return_nothing:
	batch->buf = NULL;
	batch->buf_len = 0;
	batch->len = 0;
	batch->buffer_owned = 1;
	
	KEST_PRINTF("kest_pipeline_create_fpga_transfer_batch failed (%s)\n", kest_error_code_to_string(ret_val));
	return ret_val;
}


kest_effect *kest_pipeline_get_effect_by_id(kest_pipeline *pipeline, int id)
{
	if (!pipeline)
		return NULL;
	
	KEST_PRINTF("searching pipelime %p for a effect with ID %d.\n", pipeline, id);
	
	kest_effect_pll *current = pipeline->effects;
	int i = 0;
	KEST_PRINTF("Beginning on the list%s\n", (!current) ? "..... which is empty! :0\n" : "");
	
	while (current)
	{
		KEST_PRINTF("Effect %d", i);
		
		if (current->data)
		{
			KEST_PRINTF(" had ID %d\n", current->data->id);
		}
		else
		{
			KEST_PRINTF("... doesn't exist!!!!! :(\n");
		}
		if (current->data && current->data->id == id)
		{
			KEST_PRINTF("This is the desired effect! Great. Return it\n");
			return current->data;
		}
		current = current->next;
		i++;
	}
	
	KEST_PRINTF("The desired effect was not found :(\n");
	
	return NULL;
}

int kest_pipeline_rectify_ids(kest_pipeline *pipeline, int preset_id)
{
	if (!pipeline)
		return ERR_NULL_PTR;
	
	kest_effect_pll *current = pipeline->effects;
	
	while (current)
	{
		if (current->data)
		{
			effect_rectify_param_ids(current->data);
		}
		
		current = current->next;
	}
	
	return NO_ERROR;
}

int kest_pipeline_deactivate_lfos(kest_pipeline *pipeline)
{
	if (!pipeline)
		return ERR_NULL_PTR;
	
	kest_effect_pll *current = pipeline->effects;
	
	while (current)
	{
		if (current->data)
			kest_effect_deactivate_lfos_async(current->data);
		
		current = current->next;
	}
	
	return NO_ERROR;
}

int kest_pipeline_activate_lfos(kest_pipeline *pipeline)
{
	KEST_PRINTF("kest_pipeline_activate_lfos_async\n");
	if (!pipeline)
		return ERR_NULL_PTR;
	
	kest_effect_pll *current = pipeline->effects;
	
	while (current)
	{
		if (current->data)
			kest_effect_activate_lfos_async(current->data);
		
		current = current->next;
	}
	
	return NO_ERROR;
}

int kest_pipeline_update_fpga(kest_pipeline *pipeline)
{
	#ifdef KEST_LIBRARY
	return NO_ERROR;
	#else
	KEST_PRINTF("kest_pipeline_update_fpga\n");
	if (!pipeline)
		return ERR_NULL_PTR;
	
	#ifdef KEST_USE_FREERTOS
	if (xSemaphoreTake(pipeline->mutex, portMAX_DELAY) != pdTRUE)
		return ERR_MUTEX_UNAVAILABLE;
	#endif
	
	kest_fpga_transfer_batch batch = kest_new_fpga_transfer_batch();
	
	if (!batch.buf)
		return ERR_ALLOC_FAIL;
	
	kest_effect_pll *current = pipeline->effects;
	
	while (current)
	{
		kest_fpga_transfer_batch_append_effect_updates(&batch, current->data);
		current = current->next;
	}
	
	kest_fpga_batch_append(&batch, COMMAND_COMMIT_REG_UPDATES);
	kest_fpga_queue_transfer_batch(batch);
	
	#ifdef KEST_USE_FREERTOS
	xSemaphoreGive(pipeline->mutex);
	#endif
	#endif
	
	return NO_ERROR;
}

int kest_pipeline_update_positions(kest_pipeline *pipeline)
{
	if (!pipeline)
		return ERR_NULL_PTR;
	
	kest_effect_fpga_position pos = kest_fpga_position_start();
	
	kest_effect_pll *current = pipeline->effects;
	kest_effect *effect = NULL;
	
	kest_dsp_resource *res = NULL;
	
	while (current)
	{
		effect = current->data;
		
		effect->position_ = pos;
		
		pos.block_start += effect->blocks.count;
		
		for (size_t j = 0; j < effect->resources.count; j++)
		{
			res = effect->resources.entries[j];
			
			if (!res)
				continue;
			
			switch (res->type)
			{
				case KEST_DSP_RESOURCE_MEM:
					pos.mem_start += res->mem_size;
					break;

				case KEST_DSP_RESOURCE_DELAY:
					pos.delay_start += 1;
					break;

				case KEST_DSP_RESOURCE_FILTER:
					pos.filter_start += 1;
					break;
			}
		}
		
		current = current->next;
	}
	
	return NO_ERROR;
}

int kest_effect_ptr_list_update_positions(kest_effect_ptr_list *effects)
{
	if (!effects)
		return ERR_NULL_PTR;
	
	kest_effect_fpga_position pos = kest_fpga_position_start();
	
	kest_effect *effect = NULL;
	
	kest_dsp_resource *res = NULL;
	
	int mem_increment = 0;
	
	for (size_t i = 0; i < effects->count; i++)
	{
		effect = effects->entries[i];
		
		effect->position_ = pos;
		
		pos.block_start += effect->blocks.count;
		
		mem_increment = 0;
		
		for (size_t j = 0; j < effect->resources.count; j++)
		{
			res = effect->resources.entries[j];
			
			if (!res)
				continue;
			
			switch (res->type)
			{
				case KEST_DSP_RESOURCE_MEM:
					mem_increment += res->mem_size;
					kest_mem_slot_set_effective_addr(res->data, res->handle + pos.mem_start);
					break;

				case KEST_DSP_RESOURCE_DELAY:
					pos.delay_start += 1;
					break;

				case KEST_DSP_RESOURCE_FILTER:
					pos.filter_start += 1;
					break;
			}
		}
		
		pos.mem_start += mem_increment;
	}
	
	return NO_ERROR;
}

/* Private staging only: no programming, UI publication or retirement queues. */
void kest_pipeline_discard_staged(kest_pipeline *pipeline)
{
	if (!pipeline) return;
	kest_effect_pll *node = pipeline->effects;
	while (node)
	{
		kest_effect_pll *next = node->next;
		kest_effect_free_retired(node->data);
		kest_free(node);
		node = next;
	}
	pipeline->effects = NULL;
}

static int reload_same_text(const char *a, const char *b)
{
	return (!a && !b) || (a && b && !strcmp(a, b));
}

int kest_pipeline_stage_reload(kest_pipeline *dest, const kest_pipeline *src,
                              kest_effect_desc *previous, kest_effect_desc *replacement)
{
	if (!dest || !src || !previous || !replacement || dest == src || dest->effects)
		return ERR_BAD_ARGS;
	if (!reload_same_text(previous->cname, replacement->cname)) return ERR_BAD_ARGS;
	for (kest_effect_pll *node = src->effects; node; node = node->next)
	{
		kest_effect *old = node->data;
		if (!old) goto rejected;
		kest_effect *fresh = kest_pipeline_append_effect_eff(dest,
			old->eff == previous ? replacement : old->eff);
		if (!fresh) goto rejected;
		fresh->preset = old->preset;
		effect_set_id(fresh, old->wet_mix.id.preset_id, old->id);
		fresh->wet_mix.value = old->wet_mix.value;
		fresh->band_mode.value = old->band_mode.value;
		fresh->band_lp_cutoff.value = old->band_lp_cutoff.value;
		fresh->band_hp_cutoff.value = old->band_hp_cutoff.value;
		for (kest_parameter_pll *p = fresh->parameters; p; p = p->next)
		{
			kest_parameter *match = NULL;
			for (kest_parameter_pll *q = old->parameters; q; q = q->next)
				if (p->data->name_internal && reload_same_text(p->data->name_internal, q->data->name_internal))
				{
					if (match) goto rejected;
					match = q->data;
				}
			if (match && reload_same_text(p->data->units, match->units) && isfinite(match->value))
				p->data->value = match->value;
		}
		for (kest_setting_pll *p = fresh->settings; p; p = p->next)
		{
			kest_setting *match = NULL;
			for (kest_setting_pll *q = old->settings; q; q = q->next)
				if (p->data->name_internal && reload_same_text(p->data->name_internal, q->data->name_internal))
				{
					if (match) goto rejected;
					match = q->data;
				}
			if (!match || match->type != p->data->type || !reload_same_text(match->units, p->data->units)) continue;
			if (match->value < p->data->min || match->value > p->data->max) goto rejected;
			if (p->data->type == EFFECT_SETTING_ENUM)
			{
				int compatible = 0;
				for (int i = 0; i < match->n_options; i++)
					for (int j = 0; j < p->data->n_options; j++)
						if (match->options[i].value == match->value && p->data->options[j].value == match->value &&
							reload_same_text(match->options[i].name, p->data->options[j].name)) compatible = 1;
				if (!compatible) continue;
			}
			p->data->value = match->value;
		}
		/* All values are staged before dependent bounds are evaluated. */
		for (kest_parameter_pll *p = fresh->parameters; p; p = p->next)
		{
			float lo = p->data->min_expr ? kest_expression_evaluate(p->data->min_expr, fresh->scope) : p->data->min;
			float hi = p->data->max_expr ? kest_expression_evaluate(p->data->max_expr, fresh->scope) : p->data->max;
			/* Reject incompatible values rather than order-dependent clamping. */
			if (!isfinite(lo) || !isfinite(hi) || lo > hi || !isfinite(p->data->value) ||
				p->data->value < lo || p->data->value > hi) goto rejected;
		}
	}
	return NO_ERROR;
rejected:
	kest_pipeline_discard_staged(dest);
	return ERR_BAD_ARGS;
}
