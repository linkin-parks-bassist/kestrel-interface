#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_preset_settings.c";

int init_preset_settings_page(kest_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	kest_preset_settings_str *str = malloc(sizeof(kest_preset_settings_str));
	
	if (!str)
		return ERR_ALLOC_FAIL;
	
	init_ui_page(page);
	page->panel = new_panel();
	
	if (!page->panel)
		return ERR_NULL_PTR;
	
	page->data_struct = str;
	
	str->preset = NULL;
	page->container = NULL;
	
	page->configure 			= configure_preset_settings_page;
	page->create_ui 			= create_preset_settings_page_ui;
	page->free_ui				= free_preset_settings_page_ui;
	page->free_all				= preset_settings_page_free_all;
	page->enter_page			= NULL;//enter_preset_settings_page;
	page->refresh				= refresh_preset_settings_page;
	
	page->panel = new_panel();
	
	if (!page->panel)
		return ERR_ALLOC_FAIL;
	
	return NO_ERROR;
}

int configure_preset_settings_page(kest_ui_page *page, void *data)
{
	if (!page)
		return ERR_NULL_PTR;
	
	ui_page_add_back_button(page);
	
	kest_preset *preset = (kest_preset*)data;
	
	if (!preset)
		return ERR_BAD_ARGS;
	
	if (!preset->name)
	{
		kest_preset_set_default_name_from_id(preset);
	}
	
	char buf[128];
	snprintf(buf, 128, "%s Settings", preset->name);
	
	page->panel->text = kest_strndup(buf, 128);
	
	kest_preset_settings_str *str = (kest_preset_settings_str*)page->data_struct;
	
	if (!str)
		return ERR_BAD_ARGS;
	
	str->volume_widget.preset = preset;
	str->volume_widget.param = &preset->volume;
	str->volume_widget.id = preset->volume.id;
	
	str->preset = preset;
	
	if (preset)
	{
		page->parent = preset->view_page;
	}
	
	page->configured = 1;
	
	return NO_ERROR;
}

void preset_settings_save_button_cb(lv_event_t *e)
{
	kest_ui_page *page = lv_event_get_user_data(e);
	
	if (!page)
		return;
	
	kest_preset_settings_str *str = (kest_preset_settings_str*)page->data_struct;
	
	if (!str)
		return;
	
	save_preset(str->preset);
	
	lv_obj_add_flag(str->save_button, LV_OBJ_FLAG_HIDDEN);
}

void default_preset_button_cb(lv_event_t *e)
{
	kest_ui_page *page = lv_event_get_user_data(e);
	
	if (!page)
		return;
	
	kest_preset_settings_str *str = (kest_preset_settings_str*)page->data_struct;
	
	if (!str)
		return;
	
	//set_preset_as_default(&global_cxt, str->preset);
	
	lv_obj_add_flag(str->default_button, LV_OBJ_FLAG_HIDDEN);
}

int create_preset_settings_page_ui(kest_ui_page *page)
{
	KEST_PRINTF("create_preset_settings_page_ui\n");
	if (!page)
		return ERR_NULL_PTR;
	
	if (page->ui_created)
	{
		KEST_PRINTF("Preset settings page: UI already created...\n");
		return NO_ERROR;
	}
	
	kest_preset_settings_str *str = (kest_preset_settings_str*)page->data_struct;
	
	if (!str)
	{
		KEST_PRINTF("Error! Preset settings page has no data struct!\n");
		return ERR_BAD_ARGS;
	}
		
	if (!str->preset)
	{
		KEST_PRINTF("Error! Preset settings page has no preset!\n");
		return ERR_BAD_ARGS;
	}
	
	ui_page_create_base_ui(page);
	
	lv_obj_set_flex_align(page->container,
		LV_FLEX_ALIGN_SPACE_EVENLY,
		LV_FLEX_ALIGN_CENTER,
		LV_FLEX_ALIGN_CENTER);
	
	parameter_widget_create_ui(&str->volume_widget, page->container);
	
	//str->default_button = lv_btn_create(page->screen);
    //lv_obj_set_size(str->default_button, KEST_BUTTON_WIDTH / 3, KEST_BUTTON_WIDTH);
	//lv_obj_align(str->default_button, LV_ALIGN_BOTTOM_MID, -STANDARD_CONTAINER_WIDTH / 3, -50);
    
	//str->default_button_label = lv_label_create(str->default_button);
	//lv_label_set_text(str->default_button_label, "Set Default");
	//lv_obj_center(str->default_button_label);
	
	//if (str->preset->default_preset)
	//{
	//	lv_obj_add_flag(str->default_button, LV_OBJ_FLAG_HIDDEN);
	//}
	
	//lv_obj_add_event_cb(str->default_button, default_preset_button_cb, LV_EVENT_CLICKED, page);
	
    /*str->plus_button = lv_btn_create(page->screen);
    lv_obj_set_size(str->plus_button, PRESET_VIEW_BUTTON_WIDTH / 3, PRESET_VIEW_BUTTON_HEIGHT);
	lv_obj_align(str->plus_button, LV_ALIGN_BOTTOM_MID, 0, -50);
    
	str->plus_button_label = lv_label_create(str->plus_button);
	lv_label_set_text(str->plus_button_label, "+");
	lv_obj_center(str->plus_button_label);
	
	lv_obj_add_event_cb(str->plus_button, enter_effect_selector_cb, LV_EVENT_CLICKED, page);
	
	str->save_button = lv_btn_create(page->screen);
    lv_obj_set_size(str->save_button, PRESET_VIEW_BUTTON_WIDTH / 3, PRESET_VIEW_BUTTON_HEIGHT);
	lv_obj_align(str->save_button, LV_ALIGN_BOTTOM_MID, PRESET_VIEW_EFFECT_LIST_WIDTH / 3, -50);
    
	str->save_button_label = lv_label_create(str->save_button);
	lv_label_set_text(str->save_button_label, "Save");
	lv_obj_center(str->save_button_label);
	
	lv_obj_add_event_cb(str->save_button, preset_settings_save_button_cb, LV_EVENT_CLICKED, page);*/
	
	page->ui_created = 1;
	
	return NO_ERROR;
}


int free_preset_settings_page_ui(kest_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	return NO_ERROR;
}

int preset_settings_page_free_all(kest_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	return NO_ERROR;
}

int enter_preset_settings_page(kest_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	return NO_ERROR;
}

int enter_preset_settings_page_forward(kest_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	return NO_ERROR;
}

int enter_preset_settings_page_back(kest_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	return NO_ERROR;
}

int refresh_preset_settings_page(kest_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	return NO_ERROR;
}
