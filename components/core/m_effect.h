#ifndef M_INT_TRANSFORMER_H_
#define M_INT_TRANSFORMER_H_

#define TRANSFORMER_MODE_FULL_SPECTRUM  0
#define TRANSFORMER_MODE_UPPER_SPECTRUM	1
#define TRANSFORMER_MODE_LOWER_SPECTRUM	2
#define TRANSFORMER_MODE_BAND			3

#define TRANSFORMER_WET_MIX_PID	0xFFFF

#define TRANSFORMER_BAND_LP_CUTOFF_PID 	0xFFFE
#define TRANSFORMER_BAND_HP_CUTOFF_PID 	0xFFFD
#define TRANSFORMER_BAND_MODE_SID 		0xFFFF

struct m_profile;
struct m_ui_page;

typedef struct m_effect
{
	uint16_t type;
	uint16_t id;
	
	m_parameter wet_mix;
	
	m_setting band_mode;
	m_parameter band_lp_cutoff;
	m_parameter band_hp_cutoff;
	m_parameter band_center;
	m_parameter band_width;
	
	int position;
	int block_position;
	
	m_parameter_pll *parameters;
	m_setting_pll *settings;
	
	struct m_profile *profile;
	
	#ifdef M_ENABLE_UI
	struct m_ui_page *view_page;
	#endif
	
	m_effect_desc *eff;
	m_expr_scope *scope;
	
	#ifdef M_USE_FREERTOS
	SemaphoreHandle_t mutex;
	#endif
	
	#ifdef M_ENABLE_REPRESENTATIONS
	m_representation_pll *reps;
	m_representation profile_rep;
	#endif
} m_effect;

const char *m_effect_name(m_effect *effect);

DECLARE_LINKED_PTR_LIST(m_effect);

int init_effect(m_effect *effect);

int effect_set_id(m_effect *effect, uint16_t profile_id, uint16_t effect_id);
int effect_rectify_param_ids(m_effect *effect);

m_parameter *effect_add_parameter(m_effect *effect);
m_setting *effect_add_setting(m_effect *effect);

int init_default_effect_by_type(m_effect *effect, uint16_t type, uint16_t profile_id, uint16_t effect_id);
int init_effect_from_effect_desc(m_effect *effect, m_effect_desc *eff);

#ifdef M_ENABLE_UI
int effect_init_ui_page(m_effect *effect, struct m_ui_page *parent);

void add_effect_from_menu(lv_event_t *e);
#endif

int request_append_effect(uint16_t type, m_effect *local);
#ifdef USE_TEENSY
void effect_receive_id(m_message message, m_response response);
#endif

int clone_effect(m_effect *dest, m_effect *src);
void free_effect(m_effect *effect);

m_parameter *effect_get_parameter(m_effect *effect, int n);
m_setting *effect_get_setting(m_effect *effect, int n);

int m_fpga_transfer_batch_append_effect(
		m_effect *effect,
		const m_eff_resource_report *cxt,
		m_eff_resource_report *report,
		m_fpga_transfer_batch *batch
	);


int m_effect_update_fpga_registers(m_effect *effect);

m_expr_scope *m_effect_create_scope(m_effect *effect);

int m_effect_set_parameter(m_effect *effect, const char *name, float value);
int m_effect_set_setting(m_effect *effect, const char *name, int value);

int m_effect_update_reps(m_effect *effect);
void m_effect_profile_rep_update(void *representer, void *representee);

struct m_ui_page;
int m_effect_init_view_page(m_effect *effect, struct m_ui_page *parent);

#endif
