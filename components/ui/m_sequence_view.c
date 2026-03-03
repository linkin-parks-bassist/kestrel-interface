#include "m_int.h"

static const char *FNAME = "m_sequence_view.c";

int create_sequence_view_for(m_sequence *sequence)
{
	if (!sequence)
		return ERR_NULL_PTR;
	
	sequence->view_page = m_alloc(sizeof(m_ui_page));
	
	if (!sequence->view_page)
		return ERR_ALLOC_FAIL;
	
	init_sequence_view(sequence->view_page);
	configure_sequence_view(sequence->view_page, sequence);
	
	return NO_ERROR;
}

int seq_view_clicked_cb(m_active_button *button)
{
	if (!button)
		return ERR_NULL_PTR;
	
	m_profile *profile = button->data;
	
	if (!profile)
		return ERR_BAD_ARGS;
	
	if (!profile->view_page)
	{
		if (!create_profile_view_for(profile))
			return ERR_ALLOC_FAIL;
	}
	
	enter_ui_page_forwards(profile->view_page);
	
	return NO_ERROR;
}

int seq_view_moved_cb(m_active_button *button)
{
	if (!button)
		return ERR_NULL_PTR;
	
	m_profile *profile = (m_profile*)button->data;
	
	if (!button->array || !button->array->parent || !button->array->parent->data_struct)
		return ERR_BAD_ARGS;
	
	m_sequence_view_str *str = (m_sequence_view_str*)button->array->parent->data_struct;
	
	m_sequence *sequence = str->sequence;
	
	m_sequence_move_profile(sequence, button->index, button->prev_index);
	
	return NO_ERROR;
}

int init_sequence_view(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	init_ui_page(page);
	
	page->type = M_UI_PAGE_SEQ_VIEW;
	
	m_sequence_view_str *str = m_alloc(sizeof(m_sequence_view_str));
	
	if (!str)
		return ERR_ALLOC_FAIL;
	
	str->sequence = NULL;
	
	str->play = NULL;
	str->plus = NULL;
	str->save = NULL;
	
	page->data_struct = str;
	
	page->configure = configure_sequence_view;
	page->create_ui = create_sequence_view_ui;
	
	str->array = m_active_button_array_new();
	
	m_active_button_array_set_length(str->array, 32);
	
	str->array->flags |= M_ACTIVE_BUTTON_ARRAY_FLAG_DELETEABLE;
	str->array->flags |= M_ACTIVE_BUTTON_ARRAY_FLAG_MOVEABLE;
	
	str->array->clicked_cb = seq_view_clicked_cb;
	str->array->moved_cb   = seq_view_moved_cb;
	
	str->rep.representee = NULL;
	str->rep.representer = page;
	str->rep.update = sequence_view_rep_update;
	
	return NO_ERROR;
}

void seq_play_cb(lv_event_t *e)
{
	m_ui_page *page = lv_event_get_user_data(e);
	
	if (!page)
		return;
	
	m_sequence_view_str *str = (m_sequence_view_str*)page->data_struct;
	
	if (!str)
		return;
	
	if (!str->sequence)
	{
		m_printf("ERROR: no sequence\n");
		return;
	}
	
	if (!str->sequence->active)
		m_sequence_begin(str->sequence);
	
	return;
}

void seq_plus_cb(lv_event_t *e)
{
	m_printf("seq_plus_cb, line %d\n", __LINE__);
	
	m_ui_page *page = lv_event_get_user_data(e);
	
	if (!page) return;
	
	m_printf("seq_plus_cb, line %d\n", __LINE__);
	
	m_sequence_view_str *str = (m_sequence_view_str*)page->data_struct;
	
	if (!str) return;
	
	m_printf("seq_plus_cb, line %d\n", __LINE__);
	
	m_profile *new_profile = create_new_profile();
	
	if (!new_profile) return;
	
	m_printf("seq_plus_cb, line %d\n", __LINE__);
	
	new_profile->sequence = str->sequence;
	
	seq_profile_ll *node = sequence_append_profile_rp(str->sequence, new_profile);
	
	if (!node) return;
	
	m_printf("seq_plus_cb, line %d\n", __LINE__);
	
	m_active_button *button = m_active_button_array_append_new(str->array, node->data, node->data->name);
	
	
	m_printf("seq_plus_cb, line %d\n", __LINE__);
	m_active_button_set_representation(button, button, new_profile, sequence_view_profile_button_rep_update);
	
	
	m_printf("seq_plus_cb, line %d\n", __LINE__);
	if (page->ui_created)
		m_active_button_create_ui(button, page->container);
	
	
	m_printf("seq_plus_cb, line %d\n", __LINE__);
	m_profile_add_representation(new_profile, &button->rep);
	
	return;
}

void seq_save_cb(lv_event_t *e)
{
	m_ui_page *page = lv_event_get_user_data(e);
	
	if (!page)
		return;
	
	m_sequence_view_str *str = (m_sequence_view_str*)page->data_struct;
	
	if (!str)
		return;
	
	m_sequence *sequence = str->sequence;
	
	if (sequence == &global_cxt.main_sequence)
	{
		cxt_save_all_profiles(&global_cxt);
	}
	else
	{
		save_sequence(sequence);
	}
	
	return;
}

int seq_view_free_cb(m_active_button *button)
{
	if (!button)
		return ERR_NULL_PTR;
	
	
	
	return NO_ERROR;
}

int seq_view_delete_cb(m_active_button *button)
{
	if (!button) return ERR_NULL_PTR;
	if (!button->data) return ERR_BAD_ARGS;
	
	if (!button->array) return ERR_BAD_ARGS;
	if (!button->array->parent) return ERR_BAD_ARGS;
	if (!button->array->parent->data_struct) return ERR_BAD_ARGS;
	
	m_sequence_view_str *str = (m_sequence_view_str*)button->array->parent->data_struct;
	m_sequence *sequence = str->sequence;
	
	if (!sequence) return ERR_NULL_PTR;
	
	m_profile *profile = (m_profile*)button->data;
	
	m_sequence_delete_profile(sequence, profile);
	
	// no dangling pointers tyvm
	button->data = NULL;
	
	return NO_ERROR;
}

void sequence_view_set_name(lv_event_t *e)
{
	m_ui_page *page = (m_ui_page*)lv_event_get_user_data(e);
	m_sequence_view_str *str = (m_sequence_view_str*)page->data_struct;
	
	const char *new_name = lv_textarea_get_text(page->panel->title);
	
	if (str->sequence->name)
		m_free(str->sequence->name);
	
	str->sequence->name = m_strndup(new_name, PROFILE_NAME_MAX_LEN);
	
	lv_obj_clear_state(page->panel->title, LV_STATE_FOCUSED);
	lv_obj_add_state(page->container, LV_STATE_FOCUSED);
	
	hide_keyboard();
	
	str->sequence->unsaved_changes = 1;
	m_button_enable(str->save);
	
	m_sequence_update_representations(str->sequence);
	
	return;
}

void sequence_view_revert_name(lv_event_t *e)
{
	m_ui_page *page = (m_ui_page*)lv_event_get_user_data(e);
	m_sequence_view_str *str = (m_sequence_view_str*)page->data_struct;
	
	lv_textarea_set_text(page->panel->title, str->sequence->name);
	
	lv_obj_clear_state(page->panel->title, LV_STATE_FOCUSED);
	lv_obj_add_state(page->container, LV_STATE_FOCUSED);
	
	hide_keyboard();
	
	return;
}

int configure_sequence_view(m_ui_page *page, void *data)
{
	if (!page)
		return ERR_NULL_PTR;
	
	m_sequence *sequence = (m_sequence*)data;
	
	if (!sequence)
		return ERR_BAD_ARGS;
	
	m_sequence_view_str *str = (m_sequence_view_str*)page->data_struct;
	
	if (!str)
		return ERR_ALLOC_FAIL;
	
	str->sequence = sequence;
	
	sequence->view_page = page;
	
	page->panel = new_panel();
	
	ui_page_add_back_button(page);
	
	str->array->parent = page;
	
	//str->array->free_cb 	 = seq_view_free_cb;
	str->array->delete_cb  	 = seq_view_delete_cb;
	str->array->clicked_cb   = seq_view_clicked_cb;
	str->array->moved_cb 	 = seq_view_moved_cb;
	
	str->play = ui_page_add_bottom_button(page, LV_SYMBOL_PLAY, seq_play_cb);
	str->plus = ui_page_add_bottom_button(page, LV_SYMBOL_PLUS, seq_plus_cb);
	str->save = ui_page_add_bottom_button(page, LV_SYMBOL_SAVE, seq_save_cb);
	
	page->panel->text = sequence->name;
	
	if (sequence != &global_cxt.main_sequence)
		ui_page_set_title_rw(page, sequence_view_set_name, sequence_view_revert_name);
	
	seq_profile_ll *current = sequence->profiles;
	
	m_active_button *button;
	
	while (current)
	{
		if (current->data)
		{
			button = m_active_button_array_append_new(str->array, current->data, current->data->name);
			
			if (!button)
			{
				// die
				return ERR_ALLOC_FAIL;
			}
			
			m_active_button_set_representation(button, button, current->data, sequence_view_profile_button_rep_update);
			
			
			//button = append_new_glide_button_to_array(str->buttons, current->data, current->data->name);
			//current->button = button;
		}
		
		current = current->next;
	}
	
	str->rep.representee = sequence;
	
	m_sequence_add_representation(sequence, &str->rep);
	
	page->configured = 1;
	
	return NO_ERROR;
}

int create_sequence_view_ui(m_ui_page *page)
{
	m_printf("create_sequence_view_ui\n");
	if (!page)
		return ERR_NULL_PTR;
	
	m_sequence_view_str *str = (m_sequence_view_str*)page->data_struct;
	
	if (!str)
		return ERR_BAD_ARGS;
	
	ui_page_create_base_ui(page);
	
	#ifndef USE_SDCARD
	m_button_disable(str->save);
	#endif
	
	m_active_button_array_create_ui(str->array, page->container);
	
	//create_glide_button_array_ui(str->buttons, page->container);
	
	page->ui_created = 1;
	
	m_printf("create_sequence_view_ui done\n");
	return NO_ERROR;
}

int refresh_sequence_view(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	
	
	return NO_ERROR;
}

int free_sequence_view_ui(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	
	
	return NO_ERROR;
}

int sequence_view_free_all(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	
	
	return NO_ERROR;
	
}

void sequence_view_rep_update(void *representer, void *representee)
{
	m_ui_page *page = (m_ui_page*)representer;
	m_sequence *sequence = (m_sequence*)representee;
	
	if (!page || !sequence)
		return;
	
	ui_page_set_title(page, sequence->name);
	
	m_sequence_view_str *str = page->data_struct;
	
	if (!str)
		return;
	
	if (sequence->active)
	{
		m_button_disable(str->play);
	}
	else
	{
		m_button_enable(str->play);
	}
	
	#ifdef USE_SDCARD
	if (sequence->unsaved_changes)
	{
		m_button_enable(str->save);
	}
	else
	{
		m_button_disable(str->save);
	}
	#endif
	
	return;
}


void sequence_view_profile_button_rep_update(void *representer, void *representee)
{
	m_printf("sequence_view_profile_button_rep_update\n");
	m_active_button *button  = (m_active_button*)representer;
	m_profile *profile = (m_profile*)representee;
	
	m_printf("button = %p, profile = %s\n", button, profile ? (profile->name ? profile->name : "Unnamed Profile") : "NULL");
	
	if (!button || !profile)
	{
		m_printf("sequence_view_profile_button_rep_update bailing...\n");
		return;
	}
	
	m_active_button_change_label(button, profile->name);
	
	if (profile->active)
	{
		m_printf("profile is active; activating active symbol\n");
		m_active_button_swap_del_button_for_persistent_unclickable(button, LV_SYMBOL_PLAY);
	}
	else
	{
		m_printf("profile is inactive; disabling active symbol\n");
		m_active_button_reset_del_button(button);
	}
	
	m_printf("sequence_view_profile_button_rep_update done\n");
	
	return;
}
