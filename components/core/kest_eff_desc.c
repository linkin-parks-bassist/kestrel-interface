#include "kest_int.h"

#define PRINTLINES_ALLOWED 0

IMPLEMENT_LINKED_PTR_LIST(kest_effect_desc);

IMPLEMENT_POOL(kest_effect_desc);
kest_allocator kest_effect_desc_allocator;
kest_effect_desc_pool kest_effect_desc_mem_pool;

int kest_init_effect_desc(kest_effect_desc *eff)
{
	if (!eff) return ERR_NULL_PTR;
	
	*eff = (kest_effect_desc){0};
	
	return NO_ERROR;
}

static void kest_effect_desc_destroy(kest_effect_desc *eff)
{
	kest_block_pll_free(eff->blocks);
	for (kest_parameter_pll *p = eff->parameters; p; p = p->next)
	{
		kest_free((void *)p->data->name_internal);
		kest_free((void *)p->data->name);
		kest_free((void *)p->data->units);
	}
	kest_parameter_pll_destroy(eff->parameters, kest_parameter_free);
	for (kest_setting_pll *s = eff->settings; s; s = s->next)
	{
		kest_free((void *)s->data->name_internal);
		kest_free((void *)s->data->name);
		kest_free((void *)s->data->units);
	}
	kest_setting_pll_destroy(eff->settings, kest_setting_free);
	for (kest_dsp_resource_pll *r = eff->resources; r; r = r->next)
	{
		kest_free(r->data->name);
	}
	kest_dsp_resource_pll_destroy(eff->resources, kest_dsp_resource_free);
	for (kest_named_expression_pll *d = eff->def_exprs; d; d = d->next)
		kest_free((void *)d->data->name);
	kest_named_expression_pll_free(eff->def_exprs);
	for (int i = 0; i < eff->drivers.count; i++) kest_free(eff->drivers.entries[i].data);
	kest_driver_list_destroy(&eff->drivers);
	if (eff->scope)
	{
		kest_scope_entry_dict_destroy(&eff->scope->dict, NULL);
		kest_free(eff->scope);
	}
	kest_expression_capture_destroy(&eff->expressions);
	kest_free((void *)eff->name);
	kest_free((void *)eff->cname);
	kest_free((void *)eff->description);
	char_ptr_list_destroy_all(&eff->keywords, NULL);
	char_ptr_list_destroy_all(&eff->instruments, NULL);
	char_ptr_list_destroy_all(&eff->types, NULL);
	char_ptr_list_destroy_all(&eff->genres, NULL);
	kest_allocator_free(&kest_effect_desc_allocator, eff);
}

void kest_effect_desc_retain(kest_effect_desc *eff)
{
	if (eff) eff->instance_refs++;
}

void kest_effect_desc_release(kest_effect_desc *eff)
{
	if (!eff || !eff->instance_refs) return;
	if (!--eff->instance_refs && eff->retired) kest_effect_desc_destroy(eff);
}

void kest_effect_desc_retire(kest_effect_desc *eff)
{
	if (!eff) return;
	eff->retired = 1;
	if (!eff->instance_refs) kest_effect_desc_destroy(eff);
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

kest_scope *kest_eff_desc_create_scope(kest_effect_desc *eff)
{
	if (!eff)
		return NULL;
	
	kest_scope *scope = kest_scope_new();
	
	if (!scope)
		return NULL;
	
	kest_parameter_pll *current = eff->parameters;
	
	while (current)
	{
		if (current->data)
			kest_scope_add_param(scope, current->data);
		
		current = current->next;
	}
	
	return scope;
}
