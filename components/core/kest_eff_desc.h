#ifndef KEST_EFFECT_H_
#define KEST_EFFECT_H_

typedef struct {
	const char *name;
	const char *cname;
	
	kest_block_pll *blocks;
	kest_setting_pll *settings;
	kest_parameter_pll *parameters;
	kest_dsp_resource_pll *resources;
	kest_named_expression_pll *def_exprs;
	
	kest_driver_list drivers;
	
	kest_eff_resource_report res_rpt;
	
	kest_scope *scope;
} kest_effect_desc;

DECLARE_LINKED_PTR_LIST(kest_effect_desc);

int kest_init_effect_desc(kest_effect_desc *eff);
int kest_effect_desc_generate_res_rpt(kest_effect_desc *eff);

kest_scope *kest_eff_desc_create_scope(kest_effect_desc *eff);

DECLARE_POOL(kest_effect_desc);
extern kest_allocator kest_effect_desc_allocator;
extern kest_effect_desc_pool kest_effect_desc_mem_pool;

#endif
