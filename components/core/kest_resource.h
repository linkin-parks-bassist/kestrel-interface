#ifndef KEST_RESOURCES_H_
#define KEST_RESOURCES_H_

#define KEST_DSP_RESOURCE_NOTHING	0
#define KEST_DSP_RESOURCE_LUT		1
#define KEST_DSP_RESOURCE_MEM		2
#define KEST_DSP_RESOURCE_DELAY	3
#define KEST_DSP_RESOURCE_FILTER	4

struct kest_expression;

typedef struct kest_dsp_resource {
	char *name;
	int type;
	int handle;
	int mem_size;
	struct kest_expression *size;
	struct kest_expression *delay;
	void *data;
} kest_dsp_resource;

int kest_init_dsp_resource(kest_dsp_resource *res);

int string_to_resource_type(const char *type_str);

DECLARE_LINKED_PTR_LIST(kest_dsp_resource);

struct kest_expression_ptr_list;

typedef struct kest_filter {
	int feed_forward;
	int feed_back;
	int format;
	
	struct kest_expression_ptr_list coefs;
} kest_filter;

int kest_filter_init(kest_filter *filter);
kest_filter *kest_filter_create(kest_allocator *alloc);

int kest_resources_assign_handles(kest_dsp_resource_pll *list);

typedef struct
{
	unsigned int blocks;
	unsigned int memory;
	unsigned int delays;
	unsigned int filters;
} kest_eff_resource_report;

kest_eff_resource_report empty_m_eff_resource_report();

int kest_resource_report_integrate(kest_eff_resource_report *a, const kest_eff_resource_report *b);

extern kest_dsp_resource sin_lut;
extern kest_dsp_resource tanh_lut;

#endif
