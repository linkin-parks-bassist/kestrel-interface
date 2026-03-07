#include "m_int.h"

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "m_profile_view.c";

m_ui_page *create_profile_view_for(m_profile *profile)
{
	M_PRINTF("create_profile_view_for(profile = %p)\n", profile);
	if (!profile)
		return NULL;
	
	m_ui_page *page = m_alloc(sizeof(m_ui_page));
	
	if (!page)
		return NULL;
	
	M_PRINTF("Calling init_ui_page...\n");
	init_ui_page(page);
	
	M_PRINTF("Calling init_profile_view...\n");
	int ret_val = init_profile_view(page);
	
	if (ret_val != NO_ERROR)
	{
		free_profile_view(page);
		return NULL;
	}
	
	M_PRINTF("Calling configure_profile_view...\n");
	ret_val = configure_profile_view(page, profile);
	
	if (ret_val != NO_ERROR)
	{
		free_profile_view(page);
		return NULL;
	}
	
	profile->view_page = page;
	
	M_PRINTF("create_profile_view_for done\n");
	return page;
}

int profile_view_transformer_click_cb(m_active_button *button)
{
	if (!button)
		return ERR_NULL_PTR;
	
	if (!button->data)
		return ERR_BAD_ARGS;
	
	m_transformer *trans = (m_transformer*)button->data;
	
	/*
	if (!trans->view_page)
	{
		M_PRINTF("Trnasformer's view page existn't; creating now\n");
		m_transformer_init_view_page(trans);
	}
	*/
	enter_ui_page_forwards(((m_transformer*)button->data)->view_page);
	
	return NO_ERROR;
}

int profile_view_transformer_moved_cb(m_active_button *button)
{
	if (!button)
		return ERR_NULL_PTR;
	
	m_transformer *trans = button->data;
	
	if (!trans)
		return ERR_BAD_ARGS;
	
	#ifdef USE_TEENSY
	queue_msg_to_teensy(create_m_message(M_MESSAGE_MOVE_TRANSFORMER, "sss", trans->profile->id, trans->id, button->index));
	#endif
	
	#ifdef USE_FPGA
	m_profile *profile = trans->profile;
	
	if (profile)
		m_profile_move_transformer(profile, button->index, button->prev_index);
	#endif
	
	return NO_ERROR;
}

int profile_view_transformer_delete_cb(m_active_button *button)
{
	M_PRINTF("profile_view_transformer_delete_cb\n");
	if (!button)
		return ERR_NULL_PTR;
	
	m_transformer *trans = button->data;
	
	if (!trans)
		return ERR_BAD_ARGS;
	
	int ret_val = m_profile_remove_transformer(trans->profile, trans->id);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	#ifdef USE_FPGA
	ret_val = m_profile_if_active_update_fpga(trans->profile);
	#endif
	M_PRINTF("profile_view_transformer_delete_cb done\n");
	return ret_val;
}

int init_profile_view(m_ui_page *page)
{
	//m_printf("init_profile_view...\n");
	if (!page)
		return ERR_NULL_PTR;
	
	init_ui_page(page);
	
	page->type = M_UI_PAGE_PROF_VIEW;
	
	m_profile_view_str *str = m_alloc(sizeof(m_profile_view_str));
	
	if (!str)
		return ERR_ALLOC_FAIL;
	
	str->settings_page = malloc(sizeof(m_ui_page));
	
	if (!str->settings_page)
	{
		m_free(str);
		return ERR_ALLOC_FAIL;
	}
	
	int ret_val = init_profile_settings_page(str->settings_page);
	
	if (ret_val != NO_ERROR)
	{
		return ret_val;
	}
	
	page->data_struct = (void*)str;
	
	str->profile 			= NULL;

	page->configure  			= configure_profile_view;
	page->create_ui  			= create_profile_view_ui;
	page->enter_page 			= enter_profile_view;
	page->free_all				= free_profile_view;
	
	str->save 		= NULL;
	str->plus		= NULL;
	str->play 		= NULL;
	
	str->menu_button 		= NULL;
	str->menu_button_label 	= NULL;
	
	page->panel = new_panel();
	
	str->array = m_active_button_array_new();
	
	m_active_button_array_set_length(str->array, 32);
	
	str->array->flags |= M_ACTIVE_BUTTON_ARRAY_FLAG_DELETEABLE;
	str->array->flags |= M_ACTIVE_BUTTON_ARRAY_FLAG_MOVEABLE;
	
	str->array->clicked_cb    = profile_view_transformer_click_cb;
	str->array->moved_cb      = profile_view_transformer_moved_cb;
	str->array->del_button_cb = profile_view_transformer_delete_cb;
	
	str->rep.representee = NULL;
	str->rep.representer = page;
	str->rep.update = profile_view_rep_update;
	
	return NO_ERROR;
}

static void save_button_cb(lv_event_t *e)
{
	#ifndef USE_SDCARD
	return;
	#endif
	
	m_ui_page *page = lv_event_get_user_data(e);
	
	if (!page)
		return;
	
	m_profile_view_str *str = (m_profile_view_str*)page->data_struct;
	
	if (!str)
		return;
	
	//m_printf("Saving profile...\n");
	int ret_val;
	
	if ((ret_val = m_profile_save(str->profile)) == NO_ERROR)
		m_button_disable(str->save);
}

static void menu_button_cb(lv_event_t *e)
{
	M_PRINTF("menu_button_cb\n");
	m_ui_page *page = lv_event_get_user_data(e);
	
	if (!page)
		return;
	
	m_profile_view_str *str = (m_profile_view_str*)page->data_struct;
	
	if (!str)
		return;
	
	m_profile *profile = str->profile;
	
	if (!profile)
		return;
	
	if (profile->sequence)
	{
		M_PRINTF("profile has a sequence; enter sequence view %p\n", profile->sequence->view_page);
		enter_ui_page_backwards(profile->sequence->view_page);
	}
	else
	{
		M_PRINTF("profile has no sequence. enter ... whatevery. %p\n", page->parent);
		enter_ui_page_backwards(page->parent);
	}
}

int profile_view_save_name(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	m_profile_view_str *str = (m_profile_view_str*)page->data_struct;
	
	const char *new_name = lv_textarea_get_text(page->panel->title);
	
	if (str->profile->name)
		m_free(str->profile->name);
	
	str->profile->name = m_strndup(new_name, PROFILE_NAME_MAX_LEN);
	
	lv_obj_clear_state(page->panel->title, LV_STATE_FOCUSED);
	lv_obj_add_state(page->container, LV_STATE_FOCUSED);
	
	hide_keyboard();
	
	str->profile->unsaved_changes = 1;
	
	#ifdef USE_SDCARD
	m_button_enable(str->save);
	#endif
	
	m_profile_update_representations(str->profile);
	
	return NO_ERROR;
}


void profile_view_save_name_cb(lv_event_t *e)
{
	m_ui_page *page = (m_ui_page*)lv_event_get_user_data(e);
	profile_view_save_name(page);
}

void profile_view_revert_name(lv_event_t *e)
{
	m_ui_page *page = (m_ui_page*)lv_event_get_user_data(e);
	m_profile_view_str *str = (m_profile_view_str*)page->data_struct;
	
	lv_textarea_set_text(page->panel->title, str->profile->name);
	
	lv_obj_clear_state(page->panel->title, LV_STATE_FOCUSED);
	lv_obj_add_state(page->container, LV_STATE_FOCUSED);
	
	hide_keyboard();
}

void profile_view_enter_settings_page_cb(lv_event_t *e)
{
	m_ui_page *page = (m_ui_page*)lv_event_get_user_data(e);
	
	if (!page)
		return;
	
	m_profile_view_str *str = (m_profile_view_str*)page->data_struct;
	
	if (!str)
		return;
	
	enter_ui_page_forwards(str->settings_page);
}

void profile_view_enter_main_menu_cb(lv_event_t *e)
{
	enter_ui_page_backwards(&global_cxt.pages.main_menu);
}

void profile_view_play_button_cb(lv_event_t *e)
{
	M_PRINTF("profile_view_play_button_cb\n");
	m_ui_page *page = (m_ui_page*)lv_event_get_user_data(e);
	
	if (!page)
		return;
	
	m_profile_view_str *str = (m_profile_view_str*)page->data_struct;
	
	if (!str)
		return;
	
	if (!str->profile)
	{
		M_PRINTF("Profile is NULL\n");
		set_active_profile(NULL);
		return;
	}
	
	if (str->profile->sequence)
	{
		m_sequence_begin_at(str->profile->sequence, str->profile);
	}
	else
	{
		set_active_profile(str->profile);
	}
}

int configure_profile_view(m_ui_page *page, void *data)
{
	M_PRINTF("configure_profile_view(page = %p, data = %p)\n", page, data);
	if (!page || !data)
		return ERR_NULL_PTR;
	
	if (page->configured)
		return NO_ERROR;
	
	m_profile *profile = (m_profile*)data;
	
	m_profile_view_str *str = page->data_struct;
	
	if (!str)
		return ERR_ALLOC_FAIL;
	
	page->data_struct = str;
	
	str->profile = profile;
	
	profile->view_page = page;
	
	int ret_val;
	int alloc_fail = 0;
	
	m_transformer *trans;
	
	int i = 0;
	m_transformer_pll *current = profile->pipeline.transformers;
	
	M_PRINTF("Entering while(current)\n");
	while (current)
	{
		trans = current->data;
		
		m_active_button_array_append_new(str->array, trans, m_transformer_name(trans));
		if (trans && !trans->view_page)
		{
			ret_val = m_transformer_init_view_page(trans, page);
			if (ret_val != NO_ERROR)
			{
				// kill self
			}
			else
			{
				trans->view_page->create_ui(trans->view_page);
			}
		}
		
		current = current->next;
	}
	
	configure_profile_settings_page(str->settings_page, profile);
	
	str->play = ui_page_add_bottom_button(page, LV_SYMBOL_PLAY, profile_view_play_button_cb);
	str->plus = ui_page_add_bottom_button(page, LV_SYMBOL_PLUS, enter_transformer_selector_cb);
	str->save = ui_page_add_bottom_button(page, LV_SYMBOL_SAVE, save_button_cb);
	
	#ifndef USE_SDCARD
	m_button_disable(str->save);
	#endif
	
	if (!profile->unsaved_changes)
		m_button_disable(str->save);
	
	ui_page_set_title_rw(page, profile_view_save_name_cb, profile_view_revert_name);
	
	ui_page_add_left_panel_button(page, LV_SYMBOL_LIST, menu_button_cb);
	ui_page_add_right_panel_button(page, LV_SYMBOL_SETTINGS, profile_view_enter_settings_page_cb);
	
	str->rep.representee = profile;
	
	m_profile_add_representation(profile, &str->rep);
	
	ret_val = NO_ERROR;
	if (alloc_fail)	ret_val = ERR_ALLOC_FAIL;
	else page->configured = 1;
	
	return ret_val;
}

int create_profile_view_ui(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	if (page->ui_created)
		return NO_ERROR;
	
	page->parent = &global_cxt.pages.main_menu;
	
	m_profile_view_str *str = (m_profile_view_str*)page->data_struct;
	
	if (!str)
		return ERR_BAD_ARGS;
	
	m_profile *profile = str->profile;
	
	if (!profile)
		return ERR_BAD_ARGS;
	
	if (page->panel->text)
	{
		// Prevent a minor memory leak
		// After I figure out ownership/nullification
	}
	
	if (!profile->name)
	{
		M_PRINTF("create_profile_view_ui. profile->name = NULL\n");
		m_profile_set_default_name_from_id(profile);
	}
	
	M_PRINTF("create_profile_view_ui. profile->name = %s\n", profile->name);
	
	page->panel->text = profile->name;
	
	ui_page_create_base_ui(page);
	m_active_button_array_create_ui(str->array, page->container);
	
	M_PRINTF("profile->active = %d\n", profile->active);
	
	if (profile->active)
	{
		m_button_disable(str->play);
	}
	
	if (!str->profile->unsaved_changes)
	{
		m_button_disable(str->save);
	}
	
	page->ui_created = 1;
	
	return NO_ERROR;
}

int enter_profile_view(m_ui_page *page)
{
	M_PRINTF("enter_profile_view\n");
	if (!page)
		return ERR_NULL_PTR;
	
	m_profile_view_str *str = (m_profile_view_str*)page->data_struct;
	
	if (str)
		global_cxt.working_profile = str->profile;
	M_PRINTF("set working profile\n");
	global_cxt.pages.transformer_selector.parent = page;
	M_PRINTF("enter_profile_view done\n");
	return NO_ERROR;
}

int profile_view_append_transformer(m_ui_page *page, m_transformer *trans)
{
	M_PRINTF("profile_view_append_transformer\n");
	if (!page)
		return ERR_NULL_PTR;
	
	m_profile_view_str *str = page->data_struct;
	
	if (!str)
		return ERR_BAD_ARGS;
	
	m_active_button *button = m_active_button_array_append_new(str->array, trans, m_transformer_name(trans));
	
	if (page->ui_created)
	{
		m_active_button_create_ui(button, page->container);
	}
	
	M_PRINTF("profile_view_append_transformer done; trans->view_page = %p\n", trans->view_page);
	return NO_ERROR;
}

int profile_view_recalculate_indices(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	
	
	return NO_ERROR;
}


int free_profile_view(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	m_profile_view_str *str = page->data_struct;
	
	return ERR_UNIMPLEMENTED;
	
	return NO_ERROR;
}

int profile_view_refresh_play_button(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	
	
	return NO_ERROR;
}


int profile_view_refresh_save_button(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	return NO_ERROR;
}

int profile_view_change_name(m_ui_page *page, char *name)
{
	if (!page)
		return ERR_NULL_PTR;
	
	if (!page->panel)
		return ERR_BAD_ARGS;
	

	
	return NO_ERROR;
}

int profile_view_set_left_button_mode(m_ui_page *page, int mode)
{
	if (!page)
		return ERR_NULL_PTR;
	
	m_profile_view_str *str = (m_profile_view_str*)page->data_struct;
	
	
	
	return NO_ERROR;
}

void profile_view_rep_update(void *representer, void *representee)
{
	M_PRINTF("profile_view_rep_update. representer = %p, representee = %p\n", representer, representee);
	
	m_ui_page *page = (m_ui_page*)representer;
	m_profile *profile = (m_profile*)representee;
	
	if (!page || !profile)
		return;
	
	ui_page_set_title(page, profile->name);
	
	m_profile_view_str *str = (m_profile_view_str*)page->data_struct;
	M_PRINTF("profile->active = %d,  str = %p, \n", profile->active, str);
	if (!str)
		return;
	
	if (profile->active)
	{
		m_button_disable(str->play);
	}
	else
	{
		m_button_enable(str->play);
	}
	
	#ifdef USE_SDCARD
	if (profile->unsaved_changes)
	{
		m_button_enable(str->save);
	}
	else
	{
		m_button_disable(str->save);
	}
	#endif
	
	M_PRINTF("profile_view_rep_update done\n");
}
