#ifndef KEST_INT_PRESET_SETTINGS_H_
#define KEST_INT_PRESET_SETTINGS_H_

typedef struct
{
	kest_parameter_widget volume_widget;
	kest_preset *preset;
	
	lv_obj_t *container;
		
	lv_obj_t *default_button;
	lv_obj_t *default_button_label;
	
	lv_obj_t *plus_button;
	lv_obj_t *plus_button_label;
	
	lv_obj_t *save_button;
	lv_obj_t *save_button_label;
	
	kest_representation rep;
} kest_preset_settings_str;

int init_preset_settings_page(kest_ui_page *page);

int configure_preset_settings_page(kest_ui_page *page, void *data);
int create_preset_settings_page_ui(kest_ui_page *page);
int free_preset_settings_page_ui(kest_ui_page *page);
int preset_settings_page_free_all(kest_ui_page *page);
int enter_preset_settings_page(kest_ui_page *page);
int enter_preset_settings_page_forward(kest_ui_page *page);
int enter_preset_settings_page_back(kest_ui_page *page);
int refresh_preset_settings_page(kest_ui_page *page);

#endif
