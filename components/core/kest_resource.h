#ifndef KEST_RESOURCES_H_
#define KEST_RESOURCES_H_

#define KEST_DSP_RESOURCE_NOTHING	0
#define KEST_DSP_RESOURCE_LUT		1
#define KEST_DSP_RESOURCE_MEM		2
#define KEST_DSP_RESOURCE_DELAY		3
#define KEST_DSP_RESOURCE_FILTER	4

struct kest_expression;
struct kest_effect;

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

int kest_dsp_resource_clone(kest_dsp_resource *dest, kest_dsp_resource *src);
kest_dsp_resource *kest_dsp_resource_make_clone(kest_dsp_resource *src);
kest_dsp_resource *kest_dsp_resource_make_clone_for_effect(kest_dsp_resource *src, struct kest_effect *effect);

int string_to_resource_type(const char *type_str);

DECLARE_LINKED_PTR_LIST(kest_dsp_resource);
DECLARE_PTR_LIST(kest_dsp_resource);
DECLARE_LIST(kest_dsp_resource);

struct kest_expression_ptr_list;

typedef struct kest_filter {
	int feed_forward;
	int feed_back;
	int format;
	
	int updated;
	
	struct kest_expression_ptr_list coefs;
} kest_filter;

int kest_filter_init(kest_filter *filter);
kest_filter *kest_filter_create(kest_allocator *alloc);

int kest_filter_clone(kest_filter *dest, kest_filter *src);
kest_filter *kest_filter_make_clone(kest_filter *src);

int kest_resources_assign_handles(kest_dsp_resource_pll *list);

typedef struct kest_mem_slot {
	int addr;
	int effective_addr;
	kest_fpga_sample_t value;
	
	int read_enable;
	kest_fpga_periodic_read read;
	
	struct kest_effect *effect;
} kest_mem_slot;

kest_mem_slot *kest_mem_slot_create(kest_allocator *alloc);

int kest_mem_slot_set_addr(kest_mem_slot *mem, int addr);
int kest_mem_slot_set_effective_addr(kest_mem_slot *mem, int addr);

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
