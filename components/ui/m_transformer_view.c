#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "m_transformer_view.c";

m_ui_page *create_transformer_view_for(m_transformer *trans)
{
	M_PRINTF("create_transformer_view_for(trans = %p)\n", trans);
	
	if (!trans)
		return NULL;
	
	m_ui_page *page = m_alloc(sizeof(m_ui_page));
	
	if (!page)
		return NULL;
	
	init_ui_page(page);
	
	int ret_val = init_transformer_view(page);
	
	if (ret_val != NO_ERROR)
	{
		free_transformer_view(page);
		return NULL;
	}
	
	ret_val = configure_transformer_view(page, trans);
	
	if (ret_val != NO_ERROR)
	{
		free_transformer_view(page);
		return NULL;
	}
	
	M_PRINTF("create_transformer_view_for done\n");
	return page;
}

int init_transformer_view(m_ui_page *page)
{
	//m_printf("Init transformer view...\n");
	if (!page)
		return ERR_NULL_PTR;
	
	init_ui_page(page);
	
	page->type = M_UI_PAGE_TRANS_VIEW;
	
	page->panel = new_panel();
	
	if (!page->panel)
		return ERR_ALLOC_FAIL;
	
	m_transformer_view_str *str = m_alloc(sizeof(m_transformer_view_str));
	
	page->data_struct = str;
	
	if (!str)
		return ERR_ALLOC_FAIL;
	
	str->trans 			   = NULL;
	str->parameter_widgets = NULL;
	str->setting_widgets   = NULL;
	
	str->container 		   = NULL;
	
	page->configure  		 = configure_transformer_view;
	page->create_ui  		 = create_transformer_view_ui;
	page->enter_page 		 = enter_transformer_view;
	
	for (int i = 0; i < TRANSFORMER_VIEW_MAX_GROUPS; i++)
	{
		str->group_containers[i] = NULL;
		str->group_inhabited[i] = 0;
	}
	
	str->settings_page = m_alloc(sizeof(m_ui_page));
	
	if (!str->settings_page)
		return ERR_ALLOC_FAIL;
	
	init_transformer_settings_page(str->settings_page);
	
	return NO_ERROR;
}

void transformer_view_enter_settings_cb(lv_event_t *e)
{
	m_ui_page *page = lv_event_get_user_data(e);
	
	if (!page)
		return;
	
	m_transformer_view_str *str = (m_transformer_view_str*)page->data_struct;
	
	if (!str)
		return;
	
	enter_ui_page_forwards(str->settings_page);
}

int configure_transformer_view(m_ui_page *page, void *data)
{
	//m_printf("Conpfigure transformer view... page = %p, data = %p\n", page, data);
	if (!page || !data)
	{
		if (page)
			page->data_struct = NULL;
		return ERR_NULL_PTR;
	}
	
	if (page->configured)
		return NO_ERROR;
	
	ui_page_add_parent_button(page);
	ui_page_add_right_panel_button(page, LV_SYMBOL_SETTINGS, transformer_view_enter_settings_cb);
	
	m_transformer *trans = (m_transformer*)data;
	
	page->panel->text = m_transformer_name(trans);
	
	m_transformer_view_str *str = page->data_struct;
	
	if (!str)
		return ERR_BAD_ARGS;
	
	str->trans = trans;
	
	m_parameter_widget *pw;
	m_setting_widget *sw;
	int ret_val;
	
	m_parameter_pll *current_param = trans->parameters;
	
	int i = 0;
	int group;
	while (current_param)
	{
		if (current_param->data)
		{
			pw = m_alloc(sizeof(m_parameter_widget));
		
			if (!pw)
				return ERR_ALLOC_FAIL;
			
			nullify_parameter_widget(pw);
			ret_val = configure_parameter_widget(pw, current_param->data, trans->profile, page);
			
			str->parameter_widgets = m_parameter_widget_pll_append(str->parameter_widgets, pw);
			
			group = current_param->data->group;
			
			if (0 <= group && group < TRANSFORMER_VIEW_MAX_GROUPS)
				str->group_inhabited[group] = 1;
		}
		
		current_param = current_param->next;
	}
	
	m_setting_pll *current_setting = trans->settings;
	
	while (current_setting)
	{
		if (current_setting->data && current_setting->data->page == TRANSFORMER_SETTING_PAGE_MAIN)
		{
			sw = m_alloc(sizeof(m_setting_widget));
		
			if (!sw)
				return ERR_ALLOC_FAIL;
			
			nullify_setting_widget(sw);
			ret_val = configure_setting_widget(sw, current_setting->data, trans->profile, page);
			
			str->setting_widgets = m_setting_widget_pll_append(str->setting_widgets, sw);
		}
		
		current_setting = current_setting->next;
	}
	
	configure_transformer_settings_page(str->settings_page, trans);
	str->settings_page->parent = page;
	
	page->configured = 1;
	
	return NO_ERROR;
}

int create_transformer_view_ui(m_ui_page *page)
{
	M_PRINTF("create_transformer_view_ui\n");
	if (!page)
		return ERR_NULL_PTR;
	
	if (page->ui_created)
		return NO_ERROR;
	
	ui_page_create_base_ui(page);
	
	m_transformer_view_str *str = (m_transformer_view_str*)page->data_struct;
	
	if (!str)
		return ERR_BAD_ARGS;
	
	if (!str->trans)
		return ERR_BAD_ARGS;
	
	page->panel->text = m_transformer_name(str->trans);
    
    lv_obj_set_layout(page->container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(page->container, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(page->container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY);
	
	for (int i = 0; i < TRANSFORMER_VIEW_MAX_GROUPS; i++)
	{
		if (str->group_inhabited[i])
		{
			str->group_containers[i] = lv_obj_create(page->container);
			
			lv_obj_remove_style_all(str->group_containers[i]);
			lv_obj_set_flex_flow (str->group_containers[i], LV_FLEX_FLOW_ROW_WRAP);
			lv_obj_set_flex_align(str->group_containers[i], LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY);
			lv_obj_set_size(str->group_containers[i], 0, 0);
		}
	}
	
	m_setting_widget_pll *current_setting = str->setting_widgets;
	
	int group;
	int i = 0;
	
	while (current_setting)
	{
		if (current_setting->data)
		{
			if (current_setting->data->setting)
			{
				group = current_setting->data->setting->group;
				if (0 <= group && group < TRANSFORMER_VIEW_MAX_GROUPS)
				{
					setting_widget_create_ui(current_setting->data, str->group_containers[group]);
				}
				else
				{
					setting_widget_create_ui(current_setting->data, page->container);
				}
			}
		}
		current_setting = current_setting->next;
		i++;
	}
	
	m_parameter_widget_pll *current_param = str->parameter_widgets;
	i = 0;
	
	while (current_param)
	{
		if (current_param->data)
		{
			if (current_param->data->param)
			{
				group = current_param->data->param->group;
				M_PRINTF("Creating parameter widget for parameter \"%s\" (%s). Group = %d\n",
					current_param->data->param->name, current_param->data->param->name_internal, group);
				if (0 <= group && group < TRANSFORMER_VIEW_MAX_GROUPS)
				{
					parameter_widget_create_ui(current_param->data, str->group_containers[group]);
				}
				else
				{
					parameter_widget_create_ui(current_param->data, page->container);
				}
			}
		}
		current_param = current_param->next;
		i++;
	}
	
	for (int i = 0; i < TRANSFORMER_VIEW_MAX_GROUPS; i++)
	{
		if (str->group_containers[i] && str->group_inhabited[i])
		{
			lv_obj_set_width(str->group_containers[i],  LV_SIZE_CONTENT);
			lv_obj_set_height(str->group_containers[i], LV_SIZE_CONTENT);
			
			lv_obj_set_style_max_width(str->group_containers[i], STANDARD_CONTAINER_WIDTH - 20, 0);
		}
	}
	
	create_transformer_settings_page_ui(str->settings_page);
	
	page->ui_created = 1;
	
	M_PRINTF("create_transformer_view_ui done\n");
	return NO_ERROR;
}

int enter_transformer_view(m_ui_page *page)
{
	#ifdef USE_TEENSY
	transformer_view_request_parameter_values(page);
	#endif
	return NO_ERROR;
}

int clear_transformer_view()
{
	return ERR_UNIMPLEMENTED;
}

int free_transformer_view_ui(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	lv_obj_del(page->screen);
	
	return NO_ERROR;
}

int free_transformer_view(m_ui_page *page)
{
	if (!page)
		return ERR_NULL_PTR;
	
	m_transformer_view_str *str = (m_transformer_view_str*)page->data_struct;
	
	if (str)
		destructor_free_m_parameter_widget_pll(str->parameter_widgets, free_parameter_widget);
	
	if (page->screen)
		lv_obj_del(page->screen);
	
	m_free(page);
	
	return NO_ERROR;
}
