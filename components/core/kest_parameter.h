#ifndef KEST_INT_PARAMETER_H_
#define KEST_INT_PARAMETER_H_

#define PARAM_NAM_ENG_MAX_LEN 254

#define PARAMETER_SCALE_LINEAR		0
#define PARAMETER_SCALE_LOGARITHMIC	1

#define KEST_STANDARD_GAIN_MIN -24
#define KEST_STANDARD_GAIN_MAX  24

#define KEST_PARAMETER_UNDRIVEN -1

typedef struct kest_parameter_id
{
	uint16_t preset_id;
	uint16_t effect_id;
	uint16_t parameter_id;
} kest_parameter_id;

struct kest_expression;
struct kest_effect;

struct kest_parameter_widget;

typedef struct kest_parameter
{
	float value;
	float min;
	float max;
	
	struct kest_expression *min_expr;
	struct kest_expression *max_expr;
	
	int scale;
	
	int updated;
	
	float old_value;
	float new_value;
	
	float max_velocity;
	
	struct kest_effect *effect;
	kest_parameter_id id;
	
	int widget_type;
	const char *name;
	const char *name_internal;
	const char *units;
	
	int group;
	
	int driver_override;
	int driver_index;
	
	#ifdef KEST_ENABLE_UI
	struct kest_parameter_widget *pw;
	#endif
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	kest_representation widget_rep;
	kest_representation effect_rep;
	
	kest_representation_ptr_list reps;
	#endif
} kest_parameter;

float kest_parameter_evaluate(kest_parameter *param);
int kest_parameter_if_driven_refresh(kest_parameter *param);
int kest_parameter_set(kest_parameter *param, float v);
int kest_parameter_driver_set(kest_parameter *param, float v);

int kest_parameter_clear_update(kest_parameter *param);

typedef struct kest_setting_option
{
	uint16_t value;
	const char *name;
} kest_setting_option;

typedef struct kest_setting_id
{
	uint16_t preset_id;
	uint16_t effect_id;
	uint16_t setting_id;
} kest_setting_id;

#define EFFECT_SETTING_ENUM 	0
#define EFFECT_SETTING_BOOL 	1
#define EFFECT_SETTING_INT 		2

#define EFFECT_SETTING_PAGE_SETTINGS 0
#define EFFECT_SETTING_PAGE_MAIN 	  1

typedef struct kest_setting
{
	int value;
	
	int updated;
	int old_value;
	int new_value;
	
	int type;
	int page;
	
	kest_setting_id id;
	
	int min;
	int max;
	
	int n_options;
	kest_setting_option *options;
	
	int widget_type;
	const char *name;
	const char *name_internal;
	const char *units;
	
	int group;
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	kest_representation_ptr_list reps;
	kest_representation effect_rep;
	#endif
} kest_setting;

DECLARE_LINKED_PTR_LIST(kest_parameter);
DECLARE_LINKED_PTR_LIST(kest_setting);

DECLARE_PTR_LIST(kest_parameter);
DECLARE_PTR_LIST(kest_setting);

DECLARE_LIST(kest_parameter);
DECLARE_LIST(kest_setting);

typedef kest_setting_pll setting_ll;

int init_parameter_str(kest_parameter *param);
int init_parameter(kest_parameter *param, const char *name, float level, float min, float max);
int init_parameter_wni(kest_parameter *param, const char *name, const char *name_internal, float level, float min, float max);

int kest_parameters_assign_ids(kest_parameter_pll *list);
int kest_settings_assign_ids(kest_setting_pll *list);

void kest_parameter_widget_rep_update(void *representer, void *representee);

kest_parameter *new_m_parameter_wni(const char *name, const char *name_internal, float level, float min, float max);

int init_setting_str(kest_setting *setting);
int init_setting(kest_setting *setting, const char *name, uint16_t level);

struct kest_effect;

void clone_parameter(kest_parameter *dest, kest_parameter *src);
kest_parameter *kest_parameter_make_clone(kest_parameter *src);
kest_parameter *kest_parameter_make_clone_for_effect(kest_parameter *src, struct kest_effect *effect);
void gut_parameter(kest_parameter *param);
void kest_parameter_free(kest_parameter *param);

int clone_setting(kest_setting *dest, kest_setting *src);
kest_setting *kest_setting_make_clone(kest_setting *src);
kest_setting *kest_setting_make_clone_for_effect(kest_setting *src, struct kest_effect *effect);
void gut_setting(kest_setting *setting);
void kest_setting_free(kest_setting *setting);

#ifdef KEST_ENABLE_GLOBAL_CONTEXT
struct kest_interval;
struct kest_interval kest_parameter_get_range(kest_parameter *param);
#endif

void kest_parameter_effect_rep_update(void *representer, void *representee);
void kest_setting_effect_rep_update(void *representer, void *representee);

void kest_parameter_if_updated_refresh_pw(void *param_);

DECLARE_POOL(kest_parameter);
extern kest_allocator kest_parameter_allocator;
extern kest_parameter_pool kest_parameter_mem_pool;
DECLARE_POOL(kest_setting);
extern kest_allocator kest_setting_allocator;
extern kest_setting_pool kest_setting_mem_pool;

#endif
