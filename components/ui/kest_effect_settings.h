#ifndef KEST_INT_EFFECT_SETTINGS_H_
#define KEST_INT_EFFECT_SETTINGS_H_

typedef struct
{
	lv_obj_t *text;
	
	kest_effect *effect;
	
	kest_setting_widget band_mode;
	
	kest_parameter_widget band_lp_cutoff;
	kest_parameter_widget band_hp_cutoff;
	
	lv_obj_t *band_control_cont;
} effect_settings_page_str;

int init_effect_settings_page	   (kest_ui_page *page);
int configure_effect_settings_page(kest_ui_page *page, void *data);
int create_effect_settings_page_ui(kest_ui_page *page);
int refresh_effect_settings_page  (kest_ui_page *page);
int free_effect_settings_page_ui  (kest_ui_page *page);
int effect_settings_page_free_all (kest_ui_page *page);

#endif
