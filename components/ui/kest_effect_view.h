#ifndef KEST_INT_EFFECT_VIEW_H_
#define KEST_INT_EFFECT_VIEW_H_

#define EFFECT_VIEW_GRID_CELL_HSIZE 250
#define EFFECT_VIEW_GRID_CELL_VSIZE 200

#define EFFECT_VIEW_MAX_GROUPS 5

typedef struct
{
	lv_obj_t *container;
	kest_parameter_widget_pll *parameter_widgets;
} kest_tv_grouping;

typedef struct
{
	kest_effect *effect;
	
	lv_obj_t *container;
	
	int group_inhabited[EFFECT_VIEW_MAX_GROUPS];
	lv_obj_t *group_containers[EFFECT_VIEW_MAX_GROUPS];
	
	kest_parameter_widget_pll *parameter_widgets;
	kest_setting_widget_pll   *setting_widgets;
	
	kest_ui_page *settings_page;
} kest_effect_view_str;


kest_ui_page *create_effect_view_for(kest_effect *effect);

int effect_view_configure(kest_ui_page *page, void *effect);

int init_effect_view(kest_ui_page *page);
int configure_effect_view(kest_ui_page *page, void *data);
int create_effect_view_ui(kest_ui_page *page);
int free_effect_view_ui(kest_ui_page *page);
int free_effect_view(kest_ui_page *page);
int enter_effect_view(kest_ui_page *page);
int enter_effect_view_forward(kest_ui_page *page);
int enter_effect_view_back(kest_ui_page *page);
int refresh_effect_view(kest_ui_page *page);

int effect_view_request_parameter_values(kest_ui_page *page);

#endif
