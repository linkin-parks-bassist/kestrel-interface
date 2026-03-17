#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "m_effect_select.c";

IMPLEMENT_LINKED_PTR_LIST(m_effect_selector_button);

void enter_effect_selector_cb(lv_event_t *e)
{
	enter_ui_page_forwards(&global_cxt.pages.effect_selector);
}

int init_effect_selector_eff(m_ui_page *page)
{
	M_PRINTF("init_effect_selector_eff\n");
	if (!page)
		return ERR_NULL_PTR;
	
	init_ui_page(page);
	
	page->create_ui  = create_effect_selector_ui_eff;
	page->enter_page = enter_effect_selector;
	
	m_effect_selector_str *str = m_alloc(sizeof(m_effect_selector_str));
	
	page->data_struct = str;
	
	if (!str) return ERR_ALLOC_FAIL;
	
	str->buttons 	 = NULL;
	str->button_list = NULL;
	str->page_offset = 0;
	
	m_effect_desc_pll *current = global_cxt.effects;
	
	int i = 0;
	m_effect_selector_button *button;
	while (current)
	{
		button = m_alloc(sizeof(m_effect_selector_button));
		
		if (!button)
			return ERR_ALLOC_FAIL;
		
		init_effect_selector_button_from_effect(button, current->data);
		str->buttons = m_effect_selector_button_pll_append(str->buttons, button);
		
		current = current->next;
		i++;
	}
	
	M_PRINTF("Created %d buttons\n", i);
	
	page->configured = 1;
	
	return NO_ERROR;
}

int configure_effect_selector(m_ui_page *page, void *data)
{
	//m_printf("configure_effect_selector...\n");
	if (!page || !data)
		return ERR_NULL_PTR;
	
	if (page->configured)
		return NO_ERROR;
	
	m_effect_selector_str *ts = page->data_struct;
	
	if (!ts)
		return ERR_BAD_ARGS;
	
	M_PRINTF("success\n");
	return NO_ERROR;
}

int refresh_effect_selector(m_ui_page *page)
{
	return ERR_UNIMPLEMENTED;
}

void add_effect_from_menu_eff(lv_event_t *e)
{
	m_effect_selector_button *button = lv_event_get_user_data(e);
	
	// Should never happen
	if (!button)
	{
		M_PRINTF("User tried to add effect from menu, but the pointer to the page struct is NULL!\n");
		return;
	}
	
	m_ui_page *pv = NULL;
	m_effect *effect = NULL;
	
	m_profile *profile = global_cxt.working_profile;
	
	m_effect_desc *eff = button->eff;
	
	M_PRINTF("User wishes to add a \"%s\"\n", button->name);
	
	if (!profile->view_page)
	{
		M_PRINTF("Profile does not have a view page!\n");
	}
	else
	{
		enter_ui_page_backwards(profile->view_page);
		pv = profile->view_page;
	}
	
	effect = m_profile_append_effect_eff(profile, eff);
	
	if (pv) profile_view_append_effect(pv, effect);
	
	profile->unsaved_changes = 1;
	
	m_profile_update_representations(profile);
	
	#ifdef USE_FPGA
	m_profile_if_active_update_fpga(profile);
	#endif
	
	m_effect_init_view_page(effect, profile->view_page);
	create_effect_view_ui(effect->view_page);
}

int init_effect_selector_button_from_effect(m_effect_selector_button *button, m_effect_desc *eff)
{
	//m_printf("Init effect selector button. Button = %p, index = %d, profile = %p\n", button, index, profile);
	if (!button || !eff)
		return ERR_NULL_PTR;
	
	button->type = 0;
	button->name = eff->name;
	
	button->button = NULL;
	button->label  = NULL;
	
	button->eff = eff;
	
	return NO_ERROR;
}

int create_effect_selector_button_ui_eff(m_effect_selector_button *button, lv_obj_t *parent)
{
	if (!button)
		return ERR_NULL_PTR;
	
	create_standard_button_click_short(&button->button, &button->label, parent, button->name, add_effect_from_menu_eff, button);

	return NO_ERROR;
}

int create_effect_selector_ui_eff(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	if (page->ui_created)
		return NO_ERROR;
	
	m_effect_selector_str *ts = (m_effect_selector_str*)page->data_struct;
	
	if (!ts)
		return ERR_BAD_ARGS;
	
	page->screen = lv_obj_create(NULL);
	
	create_panel_with_back_button(page);
	set_panel_text(page, "Add Effect");
	create_standard_button_list_tall(&ts->button_list, page->screen);
    
    m_effect_selector_button_pll *current = ts->buttons;
    
    while (current)
    {
		create_effect_selector_button_ui_eff(current->data, ts->button_list);
		current = current->next;
	}
	
	page->ui_created = 1;
	
	return NO_ERROR;
}

int enter_effect_selector(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	return NO_ERROR;
}
