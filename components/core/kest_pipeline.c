#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_pipeline.c";

int init_m_pipeline(kest_pipeline *pipeline)
{
	if (!pipeline)
		return ERR_NULL_PTR;
	
	pipeline->effects = NULL;
	
	return NO_ERROR;
}

kest_effect *kest_pipeline_append_effect_eff(kest_pipeline *pipeline, kest_effect_desc *eff)
{
	if (!pipeline || !eff)
		return NULL;
	
	kest_effect *effect = kest_alloc(sizeof(kest_effect));
	
	if (!effect)
		return NULL;
	
	kest_effect_pll *node = kest_alloc(sizeof(kest_effect_pll));
	
	if (!node)
		return NULL;
	
	node->data = effect;
	node->next = NULL;
	
	init_effect_from_effect_desc(effect, eff);
	
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
			if (current->data)
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
			effect = kest_alloc(sizeof(kest_effect));
			
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
	
	kest_fpga_transfer_batch result = kest_new_fpga_transfer_batch();
	
	kest_eff_resource_report rpt = empty_m_eff_resource_report();
	
	int pos = 0;
	if (pipeline->effects)
		ret_val = kest_fpga_batch_append_effects(&result, pipeline->effects, &rpt, &pos);
	
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
