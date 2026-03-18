#ifndef M_RESOURCES_H_
#define M_RESOURCES_H_

#define M_DSP_RESOURCE_NOTHING	0
#define M_DSP_RESOURCE_LUT		1
#define M_DSP_RESOURCE_MEM		2
#define M_DSP_RESOURCE_DELAY	3
#define M_DSP_RESOURCE_FILTER	4

struct m_expression;

typedef struct m_dsp_resource {
	char *name;
	int type;
	int handle;
	int mem_size;
	struct m_expression *size;
	struct m_expression *delay;
	void *data;
} m_dsp_resource;

int m_init_dsp_resource(m_dsp_resource *res);

int string_to_resource_type(const char *type_str);

DECLARE_LINKED_PTR_LIST(m_dsp_resource);

struct m_expression_ptr_list;

typedef struct m_filter {
	int feed_forward;
	int feed_back;
	int format;
	
	struct m_expression_ptr_list coefs;
} m_filter;

int m_filter_init(m_filter *filter);
m_filter *m_filter_create(m_allocator *alloc);

int m_resources_assign_handles(m_dsp_resource_pll *list);

typedef struct
{
	unsigned int blocks;
	unsigned int memory;
	unsigned int delays;
	unsigned int filters;
} m_eff_resource_report;

m_eff_resource_report empty_m_eff_resource_report();

int m_resource_report_integrate(m_eff_resource_report *a, const m_eff_resource_report *b);

extern m_dsp_resource sin_lut;
extern m_dsp_resource tanh_lut;

#endif
