#ifndef M_INT_TRANSFORMER_SETTINGS_H_
#define M_INT_TRANSFORMER_SETTINGS_H_

typedef struct
{
	lv_obj_t *text;
	
	m_effect *effect;
	
	m_setting_widget band_mode;
	
	m_parameter_widget band_lp_cutoff;
	m_parameter_widget band_hp_cutoff;
	
	lv_obj_t *band_control_cont;
} effect_settings_page_str;

int init_effect_settings_page	   (m_ui_page *page);
int configure_effect_settings_page(m_ui_page *page, void *data);
int create_effect_settings_page_ui(m_ui_page *page);
int refresh_effect_settings_page  (m_ui_page *page);
int free_effect_settings_page_ui  (m_ui_page *page);
int effect_settings_page_free_all (m_ui_page *page);

#endif
