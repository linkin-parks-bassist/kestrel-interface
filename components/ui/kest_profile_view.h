#ifndef KEST_INT_PROFILE_VIEW_H_
#define KEST_INT_PROFILE_VIEW_H_

typedef struct kest_profile_view_str
{
	kest_profile *profile;
	
	kest_button *play;
	kest_button *plus;
	kest_button *save;
	
	lv_obj_t *menu_button;
	lv_obj_t *menu_button_label;
	
	char *name_saved;
	
	kest_ui_page *settings_page;
	
	kest_active_button_array *array;
	
	kest_representation rep;
} kest_profile_view_str;

kest_ui_page *create_profile_view_for(kest_profile *profile);

int init_profile_view		(kest_ui_page *page);
int configure_profile_view	(kest_ui_page *page, void *data);
int create_profile_view_ui	(kest_ui_page *page);
int free_profile_view_ui	(kest_ui_page *page);
int free_profile_view		(kest_ui_page *page);
int enter_profile_view		(kest_ui_page *page);
int enter_profile_view_from	(kest_ui_page *page, kest_ui_page *prev);
int refresh_profile_view	(kest_ui_page *page);

int profile_view_recalculate_indices(kest_ui_page *page);

int profile_view_append_effect(kest_ui_page *page, kest_effect *effect);
int profile_view_populate_index_pos_array(kest_ui_page *page);

int profile_view_index_y_position(int index);

int profile_view_refresh_play_button(kest_ui_page *page);
int profile_view_refresh_save_button(kest_ui_page *page);

int profile_view_change_name(kest_ui_page *page, char *name);

int profile_view_set_left_button_mode(kest_ui_page *page, int mode);

void profile_view_rep_update(void *representer, void *representee);

#endif
