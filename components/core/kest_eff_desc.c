#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

IMPLEMENT_LINKED_PTR_LIST(kest_effect_desc);

int kest_init_effect_desc(kest_effect_desc *eff)
{
	if (!eff) return ERR_NULL_PTR;
	
	eff->parameters = NULL;
	eff->resources = NULL;
	eff->settings = NULL;
	eff->blocks  = NULL;
	eff->scope  = NULL;
	eff->cname = NULL;
	eff->name = NULL;
	
	eff->def_exprs = NULL;
	
	return NO_ERROR;
}

int kest_effect_desc_generate_res_rpt(kest_effect_desc *eff)
{
	if (!eff)
		return ERR_NULL_PTR;
	
	unsigned int blocks = 0;
	unsigned int memory = 0;
	unsigned int delays = 0;
	unsigned int filters = 0;
	
	kest_block_pll *cb = eff->blocks;
	
	while (cb)
	{
		blocks++;
		cb = cb->next;
	}
	
	kest_dsp_resource_pll *cr = eff->resources;
	
	while (cr)
	{
		if (cr->data)
		{
			switch (cr->data->type)
			{
				case KEST_DSP_RESOURCE_MEM:
					memory += cr->data->mem_size;
					break;
				case KEST_DSP_RESOURCE_DELAY:
					delays += 1;
					break;
				case KEST_DSP_RESOURCE_FILTER:
					filters += 1;
					break;
			}
		}
		cr = cr->next;
	}
	
	eff->res_rpt.blocks = blocks;
	eff->res_rpt.memory = memory;
	eff->res_rpt.delays = delays;
	eff->res_rpt.filters = filters;
	
	return NO_ERROR;
}

kest_expr_scope *kest_eff_desc_create_scope(kest_effect_desc *eff)
{
	if (!eff)
		return NULL;
	
	kest_expr_scope *scope = kest_new_expr_scope();
	
	if (!scope)
		return NULL;
	
	kest_parameter_pll *current = eff->parameters;
	
	while (current)
	{
		if (current->data)
			kest_expr_scope_add_param(scope, current->data);
		
		current = current->next;
	}
	
	return scope;
}
