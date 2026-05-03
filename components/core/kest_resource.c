#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kest_int.h"

//#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
//#endif

static const char *FNAME = "kest_resource.c";

IMPLEMENT_LINKED_PTR_LIST(kest_dsp_resource);
IMPLEMENT_PTR_LIST(kest_dsp_resource);
IMPLEMENT_LIST(kest_dsp_resource);

kest_dsp_resource sin_lut = {
	.type   = KEST_DSP_RESOURCE_LUT,
	.name   = NULL,
	.handle = 0,
	.size   = NULL,
	.delay  = NULL,
	.data   = NULL
};

kest_dsp_resource tanh_lut = {
	.type   = KEST_DSP_RESOURCE_LUT,
	.name   = NULL,
	.handle = 1,
	.size   = NULL,
	.delay  = NULL,
	.data   = NULL
};

int kest_init_dsp_resource(kest_dsp_resource *res)
{
	if (!res)
		return ERR_NULL_PTR;
	
	memset(res, 0, sizeof(kest_dsp_resource));
	
	res->name = NULL;
	res->type = KEST_DSP_RESOURCE_NOTHING;
	res->size = NULL;
	res->delay = NULL;
	res->handle = 0;
	res->data = NULL;
	res->mem_size = 0;
	
	return NO_ERROR;
}

int kest_dsp_resource_create_filter(kest_dsp_resource *res)
{
	if (!res)
		return ERR_NULL_PTR;
	
	res->name = NULL;
	res->type = KEST_DSP_RESOURCE_FILTER;
	res->size = NULL;
	res->delay = NULL;
	res->handle = 0;
	res->data = kest_filter_create(NULL);
	
	if (!res->data)
		return ERR_ALLOC_FAIL;
	
	res->mem_size = 0;
	
	return NO_ERROR;
}

int string_to_resource_type(const char *type_str)
{
	if ((strcmp(type_str, "delay_buffer") == 0) || (strcmp(type_str, "delay") == 0))
	{
		return KEST_DSP_RESOURCE_DELAY;
	}
	else if ((strcmp(type_str, "mem") == 0) || (strcmp(type_str, "memory") == 0))
	{
		return KEST_DSP_RESOURCE_MEM;
	}
	else if ((strcmp(type_str, "filter"			) == 0)
		 || (strcmp(type_str, "biquad"			) == 0)
		 || (strcmp(type_str, "lpf"				) == 0)
		 || (strcmp(type_str, "hpf"				) == 0)
		 || (strcmp(type_str, "bpf"				) == 0)
		 || (strcmp(type_str, "polynomial"		) == 0))
	{
		return KEST_DSP_RESOURCE_FILTER;
	}
	
	return KEST_DSP_RESOURCE_NOTHING;
}

int kest_mem_slot_set_addr(kest_mem_slot *mem, int addr)
{
	if (!mem) return ERR_NULL_PTR;
	
	mem->addr = addr;
	
	return NO_ERROR;
}

int kest_mem_slot_set_effective_addr(kest_mem_slot *mem, int addr)
{
	KEST_PRINTF("kest_mem_slot_set_effective_addr(mem = %p, addr = %d)\n", mem, addr);
	if (!mem) return ERR_NULL_PTR;
	
	mem->effective_addr = addr;
	mem->read.spec.addr[0] = (addr & 0xFF00) >> 8;
	mem->read.spec.addr[1] = (addr & 0x00FF) >> 0;
	
	return NO_ERROR;
}

int kest_resources_assign_handles(kest_dsp_resource_pll *list)
{
	int next_filter_handle = 0;
	int next_delay_handle = 0;
	int next_mem_handle = 0;
	
	kest_dsp_resource_pll *current = list;
	
	kest_mem_slot *mem = NULL;
	
	int i = 0;
	while (current)
	{
		if (current->data)
		{
			switch (current->data->type)
			{
				case KEST_DSP_RESOURCE_DELAY:
					KEST_PRINTF("Assigning \"%s\" handle %d...\n", current->data->name, next_delay_handle);
					current->data->handle = next_delay_handle;
					next_delay_handle += 1;
					break;
				
				case KEST_DSP_RESOURCE_MEM:
					current->data->handle = next_mem_handle;
					mem = current->data->data;
					if (mem)
						mem->addr = next_mem_handle;
					
					next_mem_handle += current->data->mem_size;
					break;
				
				case KEST_DSP_RESOURCE_FILTER:
					current->data->handle = next_filter_handle;
					next_filter_handle += 1;
					break;
			}
		}
		
		current = current->next;
	}
	
	return NO_ERROR;
}

kest_eff_resource_report empty_m_eff_resource_report()
{
	kest_eff_resource_report result;
	
	memset(&result, 0, sizeof(result));
	
	return result;
}

int kest_filter_init(kest_filter *filter)
{
	if (!filter)
		return ERR_NULL_PTR;
	
	filter->feed_forward = 0;
	filter->feed_back = 0;
	filter->format = 0;
	
	kest_expression_ptr_list_init(&filter->coefs);
	
	return NO_ERROR;
}

kest_filter *kest_filter_create(kest_allocator *alloc)
{
	kest_filter *filter = kest_allocator_alloc(alloc, sizeof(kest_filter));
	
	if (!filter)
		return NULL;
	
	filter->feed_forward = 0;
	filter->feed_back = 0;
	filter->format = 0;
	filter->updated = 0;
	
	kest_expression_ptr_list_init_with_allocator(&filter->coefs, alloc);
	
	return filter;
}

int kest_filter_clone(kest_filter *dest, kest_filter *src)
{
	if (!dest || !src)
		return ERR_NULL_PTR;
	
	dest->feed_forward = src->feed_forward;
	dest->feed_back = src->feed_back;
	dest->format = src->format;
	dest->updated = src->updated;
	
	int ret_val = kest_expression_ptr_list_init_with_allocator(&dest->coefs, NULL);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	for (int i = 0; i < src->coefs.count; i++)
	{
		ret_val = kest_expression_ptr_list_append(&dest->coefs, src->coefs.entries[i]);
		if (ret_val != NO_ERROR) break;
	}
	
	return ret_val;
}

kest_filter *kest_filter_make_clone(kest_filter *src)
{
	kest_filter *filter = kest_alloc(sizeof(kest_filter));
	
	if (!filter)
		return NULL;
	
	memset(filter, 0, sizeof(kest_filter));
	
	int ret_val = kest_filter_clone(filter, src);
	
	if (ret_val != NO_ERROR)
	{
		if (filter->coefs.entries)
			kest_expression_ptr_list_destroy(&filter->coefs);
		
		kest_free(filter);
		return NULL;
	}
	
	return filter;
}

kest_mem_slot *kest_mem_slot_create(kest_allocator *alloc)
{
	kest_mem_slot *mem_slot = kest_allocator_alloc(alloc, sizeof(kest_mem_slot));
	
	if (!mem_slot)
		return NULL;
	
	memset(mem_slot, 0, sizeof(kest_mem_slot));
	
	kest_fpga_periodic_read_init_mem(&mem_slot->read);
	
	return mem_slot;
}

int kest_resource_report_integrate(kest_eff_resource_report *a, const kest_eff_resource_report *b)
{
	if (!a) return ERR_NULL_PTR;
	if (!b) return ERR_BAD_ARGS;
	
	KEST_PRINTF("kest_resource_report_integrate\n");
	
	KEST_PRINTF("a->blocks = %d + %d = %d\n", a->blocks, b->blocks, a->blocks + b->blocks);
	KEST_PRINTF("a->delays = %d + %d = %d\n", a->delays, b->delays, a->delays + b->delays);
	KEST_PRINTF("a->memory = %d + %d = %d\n", a->memory, b->memory, a->memory + b->memory);
	KEST_PRINTF("a->filters = %d + %d = %d\n", a->filters, b->filters, a->filters + b->filters);
	a->blocks  += b->blocks;
	a->memory  += b->memory;
	a->delays  += b->delays;
	a->filters += b->filters;
	
	return NO_ERROR;
}

int kest_dsp_resource_clone(kest_dsp_resource *dest, kest_dsp_resource *src)
{
	KEST_PRINTF("kest_dsp_resource_clone\n");
	if (!dest || !src)
		return ERR_NULL_PTR;
	
	memcpy(dest, src, sizeof(kest_dsp_resource));
	
	dest->data = NULL;
	
	KEST_PRINTF("dest->type = %d\n", dest->type);
	if (dest->type == KEST_DSP_RESOURCE_MEM)
	{
		dest->data = kest_mem_slot_create(NULL);
		
		if (!dest->data)
			return ERR_ALLOC_FAIL;
		
		KEST_PRINTF("src->data = %p\n", src->data);
		
		if (src->data)
		{
			kest_mem_slot *mem = (kest_mem_slot*)src->data;
			memcpy(dest->data, src->data, sizeof(kest_mem_slot));
			KEST_PRINTF("mem->read_enable = %d, mem->read.period_ms = %d\n", mem->read_enable, mem->read.period_ms);
		}
	}
	else if (dest->type == KEST_DSP_RESOURCE_FILTER)
	{
		dest->data = kest_filter_make_clone(src->data);
		
		if (!dest->data) return ERR_ALLOC_FAIL;
	}
	
	return NO_ERROR;
}

kest_dsp_resource *kest_dsp_resource_make_clone(kest_dsp_resource *src)
{
	kest_dsp_resource *result = kest_alloc(sizeof(kest_dsp_resource));
	
	if (!result)
		return NULL;
	
	int ret_val = kest_dsp_resource_clone(result, src);
	
	if (ret_val != NO_ERROR)
	{
		/* The only way it can fail (see above) is an alloc fail
		 * so, no need to free any owned memory, just free the 
		 * whole thing and bail */
		kest_free(result);
		return NULL;
	}
	
	return result;
}

kest_dsp_resource *kest_dsp_resource_make_clone_for_effect(kest_dsp_resource *src, kest_effect *effect)
{
	kest_dsp_resource *result = kest_dsp_resource_make_clone(src);
	
	if (result && result->type == KEST_DSP_RESOURCE_MEM && result->data)
	{
		((kest_mem_slot*)result->data)->effect = effect;
	}
	
	return result;
}
