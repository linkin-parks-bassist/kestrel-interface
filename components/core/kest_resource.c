#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kest_int.h"

//#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 1
//#endif

static const char *FNAME = "kest_resource.c";

IMPLEMENT_LINKED_PTR_LIST(kest_dsp_resource);

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
	else if ((strcmp(type_str, "filter") == 0) || (strcmp(type_str, "biquad") == 0))
	{
		return KEST_DSP_RESOURCE_FILTER;
	}
	
	return KEST_DSP_RESOURCE_NOTHING;
}

int kest_resources_assign_handles(kest_dsp_resource_pll *list)
{
	int next_delay_handle = 0;
	int next_mem_handle = 0;
	int next_filter_handle = 0;
	
	kest_dsp_resource_pll *current = list;
	
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
	
	kest_expression_ptr_list_init_with_allocator(&filter->coefs, alloc);
	
	return filter;
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
