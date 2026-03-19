#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_effect_select.c";

IMPLEMENT_LINKED_PTR_LIST(kest_effect_selector_button);

void enter_effect_selector_cb(lv_event_t *e)
{
	enter_ui_page_forwards(&global_cxt.pages.effect_selector);
}

int init_effect_selector_eff(kest_ui_page *page)
{
	KEST_PRINTF("init_effect_selector_eff\n");
	if (!page)
		return ERR_NULL_PTR;
	
	init_ui_page(page);
	
	page->create_ui  = create_effect_selector_ui_eff;
	page->enter_page = enter_effect_selector;
	
	kest_effect_selector_str *str = kest_alloc(sizeof(kest_effect_selector_str));
	
	page->data_struct = str;
	
	if (!str) return ERR_ALLOC_FAIL;
	
	str->buttons 	 = NULL;
	str->button_list = NULL;
	str->page_offset = 0;
	
	kest_effect_desc_pll *current = global_cxt.effects;
	
	int i = 0;
	kest_effect_selector_button *button;
	while (current)
	{
		button = kest_alloc(sizeof(kest_effect_selector_button));
		
		if (!button)
			return ERR_ALLOC_FAIL;
		
		init_effect_selector_button_from_effect(button, current->data);
		str->buttons = kest_effect_selector_button_pll_append(str->buttons, button);
		
		current = current->next;
		i++;
	}
	
	KEST_PRINTF("Created %d buttons\n", i);
	
	page->configured = 1;
	
	return NO_ERROR;
}

int configure_effect_selector(kest_ui_page *page, void *data)
{
	//kest_printf("configure_effect_selector...\n");
	if (!page || !data)
		return ERR_NULL_PTR;
	
	if (page->configured)
		return NO_ERROR;
	
	kest_effect_selector_str *ts = page->data_struct;
	
	if (!ts)
		return ERR_BAD_ARGS;
	
	KEST_PRINTF("success\n");
	return NO_ERROR;
}

int refresh_effect_selector(kest_ui_page *page)
{
	return ERR_UNIMPLEMENTED;
}

void add_effect_from_menu_eff(lv_event_t *e)
{
	kest_effect_selector_button *button = lv_event_get_user_data(e);
	
	// Should never happen
	if (!button)
	{
		KEST_PRINTF("User tried to add effect from menu, but the pointer to the page struct is NULL!\n");
		return;
	}
	
	kest_ui_page *pv = NULL;
	kest_effect *effect = NULL;
	
	kest_profile *profile = global_cxt.working_profile;
	
	kest_effect_desc *eff = button->eff;
	
	KEST_PRINTF("User wishes to add a \"%s\"\n", button->name);
	
	if (!profile->view_page)
	{
		KEST_PRINTF("Profile does not have a view page!\n");
	}
	else
	{
		enter_ui_page_backwards(profile->view_page);
		pv = profile->view_page;
	}
	
	effect = kest_profile_append_effect_eff(profile, eff);
	
	if (pv) profile_view_append_effect(pv, effect);
	
	profile->unsaved_changes = 1;
	
	kest_profile_update_representations(profile);
	
	#ifdef USE_FPGA
	kest_profile_if_active_update_fpga(profile);
	#endif
	
	kest_effect_init_view_page(effect, profile->view_page);
	create_effect_view_ui(effect->view_page);
}

int init_effect_selector_button_from_effect(kest_effect_selector_button *button, kest_effect_desc *eff)
{
	//kest_printf("Init effect selector button. Button = %p, index = %d, profile = %p\n", button, index, profile);
	if (!button || !eff)
		return ERR_NULL_PTR;
	
	button->type = 0;
	button->name = eff->name;
	
	button->button = NULL;
	button->label  = NULL;
	
	button->eff = eff;
	
	return NO_ERROR;
}

int create_effect_selector_button_ui_eff(kest_effect_selector_button *button, lv_obj_t *parent)
{
	if (!button)
		return ERR_NULL_PTR;
	
	create_standard_button_click_short(&button->button, &button->label, parent, button->name, add_effect_from_menu_eff, button);

	return NO_ERROR;
}

int create_effect_selector_ui_eff(kest_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	if (page->ui_created)
		return NO_ERROR;
	
	kest_effect_selector_str *ts = (kest_effect_selector_str*)page->data_struct;
	
	if (!ts)
		return ERR_BAD_ARGS;
	
	page->screen = lv_obj_create(NULL);
	
	create_panel_with_back_button(page);
	set_panel_text(page, "Add Effect");
	create_standard_button_list_tall(&ts->button_list, page->screen);
    
    kest_effect_selector_button_pll *current = ts->buttons;
    
    while (current)
    {
		create_effect_selector_button_ui_eff(current->data, ts->button_list);
		current = current->next;
	}
	
	page->ui_created = 1;
	
	return NO_ERROR;
}

int enter_effect_selector(kest_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	return NO_ERROR;
}
