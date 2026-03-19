#ifndef KEST_INT_TRANSFORMER_H_
#define KEST_INT_TRANSFORMER_H_

#define TRANSFORMER_MODE_FULL_SPECTRUM  0
#define TRANSFORMER_MODE_UPPER_SPECTRUM	1
#define TRANSFORMER_MODE_LOWER_SPECTRUM	2
#define TRANSFORMER_MODE_BAND			3

#define TRANSFORMER_WET_MIX_PID	0xFFFF

#define TRANSFORMER_BAND_LP_CUTOFF_PID 	0xFFFE
#define TRANSFORMER_BAND_HP_CUTOFF_PID 	0xFFFD
#define TRANSFORMER_BAND_MODE_SID 		0xFFFF

struct kest_profile;
struct kest_ui_page;

typedef struct kest_effect
{
	uint16_t type;
	uint16_t id;
	
	kest_parameter wet_mix;
	
	kest_setting band_mode;
	kest_parameter band_lp_cutoff;
	kest_parameter band_hp_cutoff;
	kest_parameter band_center;
	kest_parameter band_width;
	
	int position;
	int block_position;
	
	kest_parameter_pll *parameters;
	kest_setting_pll *settings;
	
	struct kest_profile *profile;
	
	#ifdef KEST_ENABLE_UI
	struct kest_ui_page *view_page;
	#endif
	
	kest_effect_desc *eff;
	kest_expr_scope *scope;
	
	#ifdef KEST_USE_FREERTOS
	SemaphoreHandle_t mutex;
	#endif
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	kest_representation_pll *reps;
	kest_representation profile_rep;
	#endif
} kest_effect;

const char *kest_effect_name(kest_effect *effect);

DECLARE_LINKED_PTR_LIST(kest_effect);

int init_effect(kest_effect *effect);

int effect_set_id(kest_effect *effect, uint16_t profile_id, uint16_t effect_id);
int effect_rectify_param_ids(kest_effect *effect);

kest_parameter *effect_add_parameter(kest_effect *effect);
kest_setting *effect_add_setting(kest_effect *effect);

int init_default_effect_by_type(kest_effect *effect, uint16_t type, uint16_t profile_id, uint16_t effect_id);
int init_effect_from_effect_desc(kest_effect *effect, kest_effect_desc *eff);

#ifdef KEST_ENABLE_UI
int effect_init_ui_page(kest_effect *effect, struct kest_ui_page *parent);

void add_effect_from_menu(lv_event_t *e);
#endif

int request_append_effect(uint16_t type, kest_effect *local);
#ifdef USE_TEENSY
void effect_receive_id(kest_message message, kest_response response);
#endif

int clone_effect(kest_effect *dest, kest_effect *src);
void free_effect(kest_effect *effect);

kest_parameter *effect_get_parameter(kest_effect *effect, int n);
kest_setting *effect_get_setting(kest_effect *effect, int n);

int kest_fpga_transfer_batch_append_effect(
		kest_effect *effect,
		const kest_eff_resource_report *cxt,
		kest_eff_resource_report *report,
		kest_fpga_transfer_batch *batch
	);


int kest_effect_update_fpga_registers(kest_effect *effect);

kest_expr_scope *kest_effect_create_scope(kest_effect *effect);

int kest_effect_set_parameter(kest_effect *effect, const char *name, float value);
int kest_effect_set_setting(kest_effect *effect, const char *name, int value);

int kest_effect_update_reps(kest_effect *effect);
void kest_effect_profile_rep_update(void *representer, void *representee);

struct kest_ui_page;
int kest_effect_init_view_page(kest_effect *effect, struct kest_ui_page *parent);

#endif
