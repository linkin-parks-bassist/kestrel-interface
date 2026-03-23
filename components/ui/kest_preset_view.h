#ifndef KEST_INT_PRESET_VIEW_H_
#define KEST_INT_PRESET_VIEW_H_

typedef struct kest_preset_view_str
{
	kest_preset *preset;
	
	kest_button *play;
	kest_button *plus;
	kest_button *save;
	
	lv_obj_t *menu_button;
	lv_obj_t *menu_button_label;
	
	char *name_saved;
	
	kest_ui_page *settings_page;
	
	kest_active_button_array *array;
	
	kest_representation rep;
} kest_preset_view_str;

kest_ui_page *create_preset_view_for(kest_preset *preset);

int init_preset_view		(kest_ui_page *page);
int configure_preset_view	(kest_ui_page *page, void *data);
int create_preset_view_ui	(kest_ui_page *page);
int free_preset_view_ui	(kest_ui_page *page);
int free_preset_view		(kest_ui_page *page);
int enter_preset_view		(kest_ui_page *page);
int enter_preset_view_from	(kest_ui_page *page, kest_ui_page *prev);
int refresh_preset_view	(kest_ui_page *page);

int preset_view_recalculate_indices(kest_ui_page *page);

int preset_view_append_effect(kest_ui_page *page, kest_effect *effect);
int preset_view_populate_index_pos_array(kest_ui_page *page);

int preset_view_index_y_position(int index);

int preset_view_refresh_play_button(kest_ui_page *page);
int preset_view_refresh_save_button(kest_ui_page *page);

int preset_view_change_name(kest_ui_page *page, char *name);

int preset_view_set_left_button_mode(kest_ui_page *page, int mode);

void preset_view_rep_update(void *representer, void *representee);

#endif
