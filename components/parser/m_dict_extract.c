#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 1
#endif

static const char *FNAME = "m_dict_extract.c";

m_parameter *m_extract_parameter_from_dict(m_eff_parsing_state *ps, m_ast_node *dict_node, m_dictionary *dict)
{
	if (!dict)
		return NULL;
	
	int ret_val;
	m_parameter *param = m_alloc(sizeof(m_parameter));
	
	if (!param) return NULL;
	
	init_parameter_str(param);
	
	param->name_internal = m_strndup(dict->name, 128);
	
	if (!param->name_internal)
		goto parameter_extract_abort;
	
	char *str;
	float v;
	int i;
	m_expression *expr;
	
	if ((ret_val = m_dictionary_lookup_str(dict, "name", (void*)&str)) == NO_ERROR)
	{
		param->name = m_strndup(str, 128);
		M_PRINTF("Obtained parameter name; \"%s\"\n", param->name);
	}
	else
	{
		m_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"name\" for parameter \"%s\"", param->name_internal);
		goto parameter_extract_abort;
	}
	
	if ((ret_val = m_dictionary_lookup_expr(dict, "default", &expr)) == NO_ERROR)
	{
		if (!m_expression_is_constant(expr))
		{
			m_parser_error_at_node(ps, dict_node, "Default value must be constant; \"%s\" (type \"%s\") is not\n",
				m_expression_to_string(expr), m_expression_type_to_str(expr->type));
			goto parameter_extract_abort;
		}
		
		param->value = m_expression_evaluate(expr, NULL);
	}
	else
	{
		m_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"default\" for parameter \"%s\"", param->name_internal);
		goto parameter_extract_abort;
	}
	
	if ((ret_val = m_dictionary_lookup_expr(dict, "min", &expr)) == NO_ERROR)
	{
		param->min_expr = expr;
	}
	else
	{
		m_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"min\" for parameter \"%s\"", param->name_internal);
		goto parameter_extract_abort;
	}
	
	if ((ret_val = m_dictionary_lookup_expr(dict, "max", &expr)) == NO_ERROR)
	{
		param->max_expr = expr;
	}
	else
	{
		m_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"max\" for parameter \"%s\"", param->name_internal);
		goto parameter_extract_abort;
	}
	
	if ((ret_val = m_dictionary_lookup_str(dict, "scale", (void*)&str)) == NO_ERROR)
	{
		if (strcmp(str, "linear") == 0 || strcmp(str, "flat") == 0)
		{
			param->scale = PARAMETER_SCALE_LINEAR;
		}
		else if (strcmp(str, "log")  == 0 || strcmp(str, "logarithmic") == 0
		      || strcmp(str, "exp")  == 0 || strcmp(str, "exponential") == 0
		      || strcmp(str, "freq") == 0 || strcmp(str, "octave") == 0)
		{
			param->scale = PARAMETER_SCALE_LOGARITHMIC;
		}
		else
		{
			m_parser_warn_at_node(ps, dict_node, "Unknown scale \"%s\" given to parameter \"%s\". Defaulting to linear.", str, param->name_internal);
		}
	}
	
	if ((ret_val = m_dictionary_lookup_str(dict, "units", (void*)&str)) == NO_ERROR)
	{
		param->units = m_strndup(str, 128);
	}
	
	if ((ret_val = m_dictionary_lookup_expr(dict, "max_velocity", &expr)) == NO_ERROR)
	{
		if (!m_expression_is_constant(expr))
		{
			m_parser_error_at_node(ps, dict_node, "max_velocity must be constant");
			goto parameter_extract_abort;
		}
		
		param->max_velocity = fabsf(m_expression_evaluate(expr, NULL));
	}
	
	if ((ret_val = m_dictionary_lookup_str(dict, "widget_type", (void*)&str)) == NO_ERROR)
	{
		if (strcmp(str, "dial") == 0 || strcmp(str, "potentiometer") == 0)
		{
			param->widget_type = PARAM_WIDGET_VIRTUAL_POT;
		}
		else if (strcmp(str, "slider" ) == 0 || strcmp(str, "slider_horizontal") == 0
		      || strcmp(str, "hslider") == 0)
		{
			param->widget_type = PARAM_WIDGET_HSLIDER;
		}
		else if (strcmp(str, "slider_vertical") == 0
		      || strcmp(str, "vslider")  == 0)
		{
			param->widget_type = PARAM_WIDGET_VSLIDER;
		}
		else if (strcmp(str, "slider_tall_vertical")  == 0 || strcmp(str, "slider_vertical_tall") == 0
		      || strcmp(str, "vslider_tall")  == 0)
		{
			param->widget_type = PARAM_WIDGET_VSLIDER_TALL;
		}
		else
		{
			m_parser_warn_at_node(ps, dict_node, "Unknown widget type \"%s\" given to parameter \"%s\". Defaulting to dial.", str, param->name_internal);
		}
	}
	
	if ((ret_val = m_dictionary_lookup_expr(dict, "group", &expr)) == NO_ERROR)
	{
		if (!m_expression_is_constant(expr))
		{
			m_parser_error_at_node(ps, dict_node, "Group value must be constant");
			goto parameter_extract_abort;
		}
		
		param->group = (int)(roundf(m_expression_evaluate(expr, NULL)));
	}
	
	M_PRINTF("Extracted a parameter;\n");
	M_PRINTF("\tname: \"%s\"\n", param->name);
	M_PRINTF("\tname_internal: \"%s\"\n", param->name_internal);
	M_PRINTF("\tvalue: %f\n", param->value);
	M_PRINTF("\tmin_expr: %s\n", m_expression_to_string(param->min_expr));
	M_PRINTF("\tmax_expr: %s\n",  m_expression_to_string(param->max_expr));
	M_PRINTF("\tscale: %d\n", param->scale);
	M_PRINTF("\tmax_velocity: %f\n", param->max_velocity);
	M_PRINTF("\twidget_type: %d\n", param->widget_type);
	M_PRINTF("\tgroup: %d\n", param->group);
	
	return param;
	
parameter_extract_abort:
	if (param)
	{
		gut_parameter(param);
		m_free(param);
	}
	
	return NULL;
}

int m_extract_enum_setting_from_dict(m_eff_parsing_state *ps, m_ast_node *dict_node, m_dictionary *dict, m_setting *setting)
{
	if (!dict || !setting)
		return ERR_NULL_PTR;
	
	int ret_val;
	setting->widget_type = SETTING_WIDGET_DROPDOWN;
	
	return NO_ERROR;
}

int m_extract_bool_setting_from_dict(m_eff_parsing_state *ps, m_ast_node *dict_node, m_dictionary *dict, m_setting *setting)
{
	if (!dict || !setting)
		return ERR_NULL_PTR;
	
	int ret_val;
	setting->widget_type = SETTING_WIDGET_SWITCH;
	
	const char *str;
	
	setting->value = 0;
	if ((ret_val = m_dictionary_lookup_str(dict, "default", &str)) == NO_ERROR)
	{
		if (strcmp(str, "true") == 0)
		{
			setting->value = 1;
		}
		else if (strcmp(str, "false") != 0)
		{
			m_parser_warn_at_node(ps, dict_node, "Unknown default value \"%s\" given to bool setting \"%s\". Defaulting to false", str, setting->name_internal);
		}
	}
	else
	{
		m_parser_warn_at_node(ps, dict_node, "No default value given to bool setting \"%s\". Defaulting to false", setting->name_internal);
	}
	
	return NO_ERROR;
}

int m_extract_int_setting_from_dict(m_eff_parsing_state *ps, m_ast_node *dict_node, m_dictionary *dict, m_setting *setting)
{
	if (!dict || !setting)
		return ERR_NULL_PTR;
	
	int ret_val;
	setting->widget_type = SETTING_WIDGET_FIELD;
	
	m_expression *expr;
	
	setting->value = 0;
	if ((ret_val = m_dictionary_lookup_expr(dict, "default", &expr)) == NO_ERROR)
	{
		if (!m_expression_is_constant(expr))
		{
			m_parser_error_at_node(ps, dict_node, "Default value must be constant");
			return ERR_BAD_ARGS;
		}
		
		setting->value = (int)(roundf(m_expression_evaluate(expr, NULL)));
	}
	else
	{
		m_parser_warn_at_node(ps, dict_node, "Could not find mandatory attribute \"default\" for setting \"%s\"; defulating to 0.", setting->name_internal);
	}
	
	setting->min = 0;
	if ((ret_val = m_dictionary_lookup_expr(dict, "min", &expr)) == NO_ERROR)
	{
		if (!m_expression_is_constant(expr))
		{
			m_parser_error_at_node(ps, dict_node, "Min must be constant");
			return ERR_BAD_ARGS;
		}
		
		setting->min = (int)(roundf(m_expression_evaluate(expr, NULL)));
	}
	else
	{
		m_parser_warn_at_node(ps, dict_node, "Could not find mandatory attribute \"min\" for setting \"%s\"; defulating to 0.", setting->name_internal);
	}
	
	setting->max = 0;
	if ((ret_val = m_dictionary_lookup_expr(dict, "max", &expr)) == NO_ERROR)
	{
		if (!m_expression_is_constant(expr))
		{
			m_parser_error_at_node(ps, dict_node, "Max must be constant");
			return ERR_BAD_ARGS;
		}
		
		setting->max = (int)(roundf(m_expression_evaluate(expr, NULL)));
	}
	else
	{
		m_parser_warn_at_node(ps, dict_node, "Could not find mandatory attribute \"max\" for setting \"%s\"; defulating to 0.", setting->name_internal);
	}
	
	return NO_ERROR;
}

m_setting *m_extract_setting_from_dict(m_eff_parsing_state *ps, m_ast_node *dict_node, m_dictionary *dict)
{
	if (!dict)
		return NULL;
	
	int ret_val;
	char *str;
	float v;
	int i;
	m_expression *expr;
	
	m_setting *setting = m_alloc(sizeof(m_setting));
	if (!setting) return NULL;
	
	init_setting_str(setting);
	
	setting->name_internal = m_strndup(dict->name, 128);
	
	if (!setting->name_internal)
		goto setting_extract_abort;
	
	if ((ret_val = m_dictionary_lookup_str(dict, "name", (void*)&str)) == NO_ERROR)
	{
		setting->name = m_strndup(str, 128);
		M_PRINTF("Obtained setting name; \"%s\"\n", setting->name);
	}
	else
	{
		m_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"name\" for setting \"%s\"", setting->name_internal);
		goto setting_extract_abort;
	}
	
	setting->type = TRANSFORMER_SETTING_ENUM;
	if ((ret_val = m_dictionary_lookup_str(dict, "type", (void*)&str)) == NO_ERROR)
	{
		M_PRINTF("Obtained setting type; \"%s\"\n", str);
		
		if (strcmp(str, "enum") == 0)
		{
			setting->type = TRANSFORMER_SETTING_ENUM;
		}
		else if (strcmp(str, "bool") == 0)
		{
			setting->type = TRANSFORMER_SETTING_BOOL;
		}
		else if (strcmp(str, "int") == 0)
		{
			setting->type = TRANSFORMER_SETTING_INT;
		}
		else
		{
			m_parser_warn_at_node(ps, dict_node, "Unknown scale \"%s\" given to setting \"%s\". Defaulting to int.", str, setting->name_internal);
		}
	}
	else
	{
		m_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"type\" for setting \"%s\"", setting->name_internal);
		goto setting_extract_abort;
	}
	
	switch (setting->type)
	{
		case TRANSFORMER_SETTING_ENUM:
			ret_val = m_extract_enum_setting_from_dict(ps, dict_node, dict, setting);
			
			if (ret_val != NO_ERROR)
				goto setting_extract_abort;
			break;
		
		case TRANSFORMER_SETTING_BOOL:
			ret_val = m_extract_bool_setting_from_dict(ps, dict_node, dict, setting);
			
			if (ret_val != NO_ERROR)
				goto setting_extract_abort;
			break;
		
		case TRANSFORMER_SETTING_INT:
			ret_val = m_extract_int_setting_from_dict(ps, dict_node, dict, setting);
			
			if (ret_val != NO_ERROR)
				goto setting_extract_abort;
			break;
	}
	
	if ((ret_val = m_dictionary_lookup_str(dict, "units", (void*)&str)) == NO_ERROR)
		setting->units = m_strndup(str, 128);
	
	setting->page = TRANSFORMER_SETTING_PAGE_SETTINGS;
	
	if ((ret_val = m_dictionary_lookup_str(dict, "page", (void*)&str)) == NO_ERROR)
	{
		M_PRINTF("Obtained setting page; \"%s\"\n", str);
		
		if (strcmp(str, "main") == 0)
			setting->page = TRANSFORMER_SETTING_PAGE_MAIN;
		else if (strcmp(str, "settings") == 0)
			setting->page = TRANSFORMER_SETTING_PAGE_SETTINGS;
		else
			m_parser_warn_at_node(ps, dict_node, "Unknown page \"%s\" given for setting \"%s\". Defaulting to settings page.", str, setting->name_internal);
	}
	
	if ((ret_val = m_dictionary_lookup_expr(dict, "group", &expr)) == NO_ERROR)
	{
		if (!m_expression_is_constant(expr))
		{
			m_parser_error_at_node(ps, dict_node, "Group value must be constant");
			goto setting_extract_abort;
		}
		
		setting->group = (int)(roundf(m_expression_evaluate(expr, NULL)));
	}
	
	M_PRINTF("Extracted a setting;\n");
	M_PRINTF("\tname: \"%s\"\n", setting->name);
	M_PRINTF("\tname_internal: \"%s\"\n", setting->name_internal);
	M_PRINTF("\tpage: %s\n", (setting->page == TRANSFORMER_SETTING_PAGE_MAIN) ? "main" : "settings");
	
	return setting;
	
setting_extract_abort:
	
	if (setting)
	{
		gut_setting(setting);
		m_free(setting);
	}
	
	return NULL;
}

int m_extract_delay_buffer_from_dict(m_eff_parsing_state *ps, m_ast_node *dict_node, m_dictionary *dict, m_dsp_resource *res)
{
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	int ret_val;
	
	m_expression *expr;
	
	ret_val = m_dictionary_lookup_expr(dict, "size", &expr);
	
	if (ret_val == NO_ERROR)
	{
		res->size = expr;
	}
	else
	{
		res->size = NULL;
	}
	
	ret_val = m_dictionary_lookup_expr(dict, "delay", &expr);
		
	if (ret_val == NO_ERROR)
	{
		res->delay = expr;
	}
	else
	{
		m_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"delay\" for delay buffer \"%s\"", res->name);
		return ERR_BAD_ARGS;
	}

	M_PRINTF("Found delay \"%s\", size %p, delay %p\n", res->name, res->size, res->delay);
	
	return NO_ERROR;
}

int m_extract_mem_from_dict(m_eff_parsing_state *ps, m_ast_node *dict_node, m_dictionary *dict, m_dsp_resource *res)
{
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	int ret_val;
	
	int size = 1;
	
	m_expression *expr;
	
	M_PRINTF("Extracting filter \"%s\"...\n", res->name);
	
	ret_val = m_dictionary_lookup_expr(dict, "size", &expr);
	
	if (ret_val == NO_ERROR)
	{
		size = (int)(roundf(fabs(m_expression_evaluate(expr, NULL))));
		
		if (size == 0)
			size = 1;
		
		res->mem_size = size;
	}
	else
	{
		res->mem_size = 1;
	}
	
	return NO_ERROR;
}

int m_extract_filter_coefs_from_dict(m_eff_parsing_state *ps, m_ast_node *dict_node, m_dictionary *dict, m_dsp_resource *res)
{
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	M_PRINTF("Extracting coefficients for filter \"%s\"...\n", res->name);
	
	m_filter *filter = (m_filter*)res->data;
	
	if (!filter)
		return ERR_ALLOC_FAIL;
	
	m_dictionary_entry_list *coefs = NULL;
	
	int ret_val = m_dictionary_lookup_list(dict, "coefs", &coefs);
	
	if (ret_val != NO_ERROR)
	{
		m_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"coefs\" for filter \"%s\"", res->name);
		return ERR_BAD_ARGS;
	}
	
	M_PRINTF("Found list; pointer %p.\n", coefs);
	
	M_PRINTF("%d entries. begin traversal\n", coefs->count);
	
	m_string *string;
	char *str;
	
	for (int i = 0; i < coefs->count; i++)
	{
		string = m_dict_entry_to_string(&coefs->entries[i]);
		if (!string)
		{
			M_PRINTF("Alloc fail!!\n");
			return ERR_ALLOC_FAIL;
		}
		str = m_string_to_native(string);
		if (!str)
		{
			M_PRINTF("Alloc fail 2!!\n");
			return ERR_ALLOC_FAIL;
		}
		M_PRINTF("Entry %d; %s\n", i, str);
		m_free(str);
		m_string_destroy(string);
		string = NULL;
		
		if (coefs->entries[i].type != DICT_ENTRY_TYPE_EXPR)
		{
			m_parser_error_at_node(ps, dict_node, "Filter coefficients must be expressions, but coefficient %d of filter \"%s\" is a %s (%d).",
				i, res->name, m_dict_entry_type_to_string_nice(coefs->entries[i].type), coefs->entries[i].type);
			return ERR_BAD_ARGS;
		}
		
		m_expression_ptr_list_append(&filter->coefs, coefs->entries[i].value.val_expr);
	}
	
	return NO_ERROR;
}

int m_extract_biquad_from_dict(m_eff_parsing_state *ps, m_ast_node *dict_node, m_dictionary *dict, m_dsp_resource *res)
{
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	m_filter *filter = m_filter_create(NULL);
	
	if (!filter)
		return ERR_ALLOC_FAIL;
	
	res->data = (void*)filter;
	
	filter->feed_forward = 3;
	filter->feed_back = 2;
	
	int ret_val = m_extract_filter_coefs_from_dict(ps, dict_node, dict, res);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	m_expression_ptr_list *coefs = &filter->coefs;
	
	if (filter->feed_forward + filter->feed_back != coefs->count)
	{
		m_parser_error_at_node(ps, dict_node, "Filter \"%s\" is declared to be a biquad, but supplies %d coefficients, not 5.", res->name, coefs->count);
		return ERR_BAD_ARGS;
	}
	
	M_PRINTF("Extracted (%d, %d) filter \"%s\", with coefficients\n", filter->feed_forward, filter->feed_back, res->name);
	
	for (int i = 0; i < coefs->count; i++)
	{
		M_PRINTF("\t%s\n", m_expression_to_string(coefs->entries[i]));
	}
	
	return NO_ERROR;
}

int m_extract_filter_from_dict(m_eff_parsing_state *ps, m_ast_node *dict_node, m_dictionary *dict, m_dsp_resource *res)
{
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	m_filter *filter = m_filter_create(NULL);
	
	if (!filter)
		return ERR_ALLOC_FAIL;
	
	res->data = (void*)filter;
	
	int ret_val = m_extract_filter_coefs_from_dict(ps, dict_node, dict, res);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	m_expression *expr;
	
	ret_val = m_dictionary_lookup_expr(dict, "feed_back", &expr);
	
	if (ret_val == NO_ERROR)
	{
		filter->feed_back = (int)(roundf(m_expression_evaluate(expr, NULL)));
		
		if (filter->feed_back < 0)
		{
			m_parser_error_at_node(ps, dict_node, "Filter \"%s\" has negative feed-back degree %d, which doesn't make sense.",
				res->name, filter->feed_back);
			return ERR_BAD_ARGS;
		}
		if (filter->feed_back > filter->coefs.count)
		{
			m_parser_error_at_node(ps, dict_node, "Filter \"%s\" has feed-back degree %d, which is higher than its coefficient count %d!",
				res->name, filter->feed_back, filter->coefs.count);
			return ERR_BAD_ARGS;
		}
		
		filter->feed_forward = filter->coefs.count - filter->feed_back;
	}
	else
	{
		filter->feed_back = 0;
		filter->feed_forward = filter->coefs.count;
	}
	
	M_PRINTF("Extracted (%d, %d) filter \"%s\", with coefficients\n", filter->feed_forward, filter->feed_back, res->name);
	
	m_expression_ptr_list *coefs = &filter->coefs;
	
	for (int i = 0; i < coefs->count; i++)
	{
		M_PRINTF("\t%s\n", m_expression_to_string(coefs->entries[i]));
	}
	
	return NO_ERROR;
}

m_dsp_resource *m_extract_resource_from_dict(m_eff_parsing_state *ps, m_ast_node *dict_node, m_dictionary *dict)
{
	if (!dict)
		return NULL;
	
	int ret_val;
	float delay_len;
	char *type_str = NULL;
	m_dsp_resource *res = m_alloc(sizeof(m_dsp_resource));
	
	if (!res) return NULL;
	
	m_init_dsp_resource(res);
	
	res->name = m_strndup(dict->name, 128);
	
	if (!res->name)
		goto resource_extract_abort;
	
	ret_val = m_dictionary_lookup_str(dict, "type", (void*)&type_str);
	
	if (ret_val != NO_ERROR || !type_str)
	{
		m_parser_error_at_node(ps, dict_node, "Could not find attribute \"type\" for resource \"%s\"", dict->name);
		goto resource_extract_abort;
	}
	
	res->type = string_to_resource_type(type_str);
	
	if (res->type == M_DSP_RESOURCE_NOTHING)
	{
		m_parser_error_at_node(ps, dict_node, "Resource type \"%s\" unrecognised", type_str);
		goto resource_extract_abort;
	}
	else if (res->type == M_DSP_RESOURCE_DELAY)
	{
		m_extract_delay_buffer_from_dict(ps, dict_node, dict, res);
	}
	else if (res->type == M_DSP_RESOURCE_MEM)
	{
		m_extract_mem_from_dict(ps, dict_node, dict, res);
	}
	else if (res->type == M_DSP_RESOURCE_FILTER)
	{
		if (strcmp(type_str, "biquad") == 0)
			m_extract_biquad_from_dict(ps, dict_node, dict, res);
		else
			m_extract_filter_from_dict(ps, dict_node, dict, res);
	}
	
	return res;
	
resource_extract_abort:
	if (res)
	{
		if (res->name)
			m_free(res->name);
		
		m_free(res);
	}
	
	return NULL;
}

