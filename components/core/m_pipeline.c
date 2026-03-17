#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "m_pipeline.c";

int init_m_pipeline(m_pipeline *pipeline)
{
	if (!pipeline)
		return ERR_NULL_PTR;
	
	pipeline->effects = NULL;
	
	return NO_ERROR;
}

m_effect *m_pipeline_append_effect_eff(m_pipeline *pipeline, m_effect_desc *eff)
{
	if (!pipeline || !eff)
		return NULL;
	
	m_effect *effect = m_alloc(sizeof(m_effect));
	
	if (!effect)
		return NULL;
	
	m_effect_pll *node = m_alloc(sizeof(m_effect_pll));
	
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
		m_effect_pll *current = pipeline->effects;
		
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

int m_pipeline_remove_effect(m_pipeline *pipeline, uint16_t id)
{
	M_PRINTF("m_pipeline_remove_effect\n");
	if (!pipeline)
		return ERR_NULL_PTR;
	
	m_effect_pll *current = pipeline->effects;
	m_effect_pll *prev = NULL;
	
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
			
			m_free(current);
			
			M_PRINTF("m_pipeline_remove_effect found and vanquished the effect\n");
			return NO_ERROR;
		}
		
		prev = current;
		current = current->next;
	}
	
	
	M_PRINTF("m_pipeline_remove_effect finished without finding the effect\n");
	return ERR_INVALID_TRANSFORMER_ID;
}

int m_pipeline_move_effect(m_pipeline *pipeline, int new_pos, int old_pos)
{
	if (!pipeline)
		return ERR_NULL_PTR;
	
	if (!pipeline->effects)
		return ERR_BAD_ARGS;
	
	m_effect_pll *target  = NULL;
	
	int i = 0;
	m_effect_pll *current = pipeline->effects;
	m_effect_pll *prev    = NULL;
	
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

int m_pipeline_get_n_effects(m_pipeline *pipeline)
{
	if (!pipeline)
		return -ERR_NULL_PTR;
	
	int n = 0;
	
	m_effect_pll *current = pipeline->effects;
	
	while (current)
	{
		if (current->data)
			n++;
		current = current->next;
	}
	
	return n;
}

int clone_pipeline(m_pipeline *dest, m_pipeline *src)
{
	if (!src || !dest)
		return ERR_NULL_PTR;
	
	M_PRINTF("Cloning pipeline...\n");
	
	m_effect_pll *current = src->effects;
	m_effect_pll *nl;
	m_effect *effect = NULL;
	
	int i = 0;
	while (current)
	{
		M_PRINTF("Cloning effect %d... current = %p, current->next = %p\n", i, current, current->next);
		if (current->data)
		{
			effect = m_alloc(sizeof(m_effect));
			
			if (!effect)
				return ERR_ALLOC_FAIL;
			
			clone_effect(effect, current->data);
			
			nl = m_effect_pll_append(dest->effects, effect);
		
			if (nl)
				dest->effects = nl;
		}
		
		current = current->next;
		i++;
	}
	
	return NO_ERROR;
}

void gut_pipeline(m_pipeline *pipeline)
{
	if (!pipeline)
		return;
	
	destructor_free_m_effect_pll(pipeline->effects, free_effect);
	pipeline->effects = NULL;
}

int m_pipeline_create_fpga_transfer_batch(m_pipeline *pipeline, m_fpga_transfer_batch *batch)
{
	M_PRINTF("m_pipeline_create_fpga_transfer_batch(pipeline = %p, batch = %p)\n", pipeline, batch);
	if (!batch)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	if (!pipeline)
	{
		ret_val = ERR_BAD_ARGS;
		goto return_nothing;
	}
	
	m_fpga_transfer_batch result = m_new_fpga_transfer_batch();
	
	m_fpga_batch_append(&result, COMMAND_BEGIN_PROGRAM);
	
	m_eff_resource_report rpt = empty_m_eff_resource_report();
	
	int pos = 0;
	if (pipeline->effects)
		ret_val = m_fpga_batch_append_effects(&result, pipeline->effects, &rpt, &pos);
	
	if (ret_val != NO_ERROR)
	{
		m_free_fpga_transfer_batch(result);
		goto return_nothing;
	}
	
	m_fpga_batch_append(&result, COMMAND_END_PROGRAM);
	
	*batch = result;
	
	M_PRINTF("m_pipeline_create_fpga_transfer_batch done (%s)\n", m_error_code_to_string(ret_val));
	return ret_val;
	
return_nothing:
	batch->buf = NULL;
	batch->buf_len = 0;
	batch->len = 0;
	batch->buffer_owned = 1;
	
	M_PRINTF("m_pipeline_create_fpga_transfer_batch failed (%s)\n", m_error_code_to_string(ret_val));
	return ret_val;
}


m_effect *m_pipeline_get_effect_by_id(m_pipeline *pipeline, int id)
{
	if (!pipeline)
		return NULL;
	
	M_PRINTF("searching pipelime %p for a effect with ID %d.\n", pipeline, id);
	
	m_effect_pll *current = pipeline->effects;
	int i = 0;
	M_PRINTF("Beginning on the list%s\n", (!current) ? "..... which is empty! :0\n" : "");
	
	while (current)
	{
		M_PRINTF("Transformer %d", i);
		
		if (current->data)
		{
			M_PRINTF(" had ID %d\n", current->data->id);
		}
		else
		{
			M_PRINTF("... doesn't exist!!!!! :(\n");
		}
		if (current->data && current->data->id == id)
		{
			M_PRINTF("This is the desired effect! Great. Return it\n");
			return current->data;
		}
		current = current->next;
		i++;
	}
	
	M_PRINTF("The desired effect was not found :(\n");
	
	return NULL;
}
