#include "kest_int.h"
#include <limits.h>

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "kest_dict_extract.c";


#define CHECK_MANDATORY_ATTR(attr) do { \
	if (!kest_eff_entry_dict_lookup(dict, attr)) { \
		kest_parser_error(ps, "%s \"%s\": mandatory attribute \"%s\" not found", type, name, attr); \
		return NULL; \
	}\
} while (0)	

#define ASSERT_ATTR_EXPR() do { \
	if (entry->type != KEST_EFF_ENTRY_TYPE_EXPR) { \
		kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be a numerical expression", type, name, key); \
		ret_val = ERR_BAD_ARGS; \
		goto extract_finish; \
	} \
	else \
	{ \
		if (!entry->value.val_expr) { \
			KEST_PRINTF_FORCE("entry->value.val_expr = NULL. Bug ?\n"); \
			ret_val = ERR_BAD_ARGS; \
			goto extract_finish; \
		} \
	} \
} while (0)

#define ASSERT_ATTR_CONST_EXPR() do { \
	if (entry->type != KEST_EFF_ENTRY_TYPE_EXPR) { \
		kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be a numerical expression", type, name, key); \
		ret_val = ERR_BAD_ARGS; \
		goto extract_finish; \
	} \
	else \
	{ \
		if (!entry->value.val_expr) { \
			KEST_PRINTF_FORCE("entry->value.val_expr = NULL. Bug ?\n"); \
			ret_val = ERR_BAD_ARGS; \
			goto extract_finish; \
		} \
		if (!kest_expression_is_constant(entry->value.val_expr)) { \
			kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be constant", type, name, key); \
			ret_val = ERR_BAD_ARGS; \
			goto extract_finish; \
		} \
		 \
		value = kest_expression_evaluate(expr, NULL); \
	} \
} while (0)

#define ASSERT_ATTR_CONST_INT() do { \
	if (entry->type != KEST_EFF_ENTRY_TYPE_EXPR) { \
		kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be a numerical expression", type, name, key); \
		ret_val = ERR_BAD_ARGS; \
		goto extract_finish; \
	} \
	else \
	{ \
		if (!entry->value.val_expr) { \
			KEST_PRINTF_FORCE("entry->value.val_expr = NULL. Bug ?\n"); \
			ret_val = ERR_BAD_ARGS; \
			goto extract_finish; \
		} \
		if (!kest_expression_is_constant(entry->value.val_expr)) { \
			kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be constant", type, name, key); \
			ret_val = ERR_BAD_ARGS; \
			goto extract_finish; \
		} \
		 \
		value = kest_expression_evaluate(expr, NULL); \
		 \
		if (fabsf(value - roundf(value)) > 1e-6) \
		{ \
			kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be an integer", type, name, key); \
			ret_val = ERR_BAD_ARGS; \
			goto extract_finish; \
		} \
		\
		value = roundf(value); \
	} \
} while (0)


#define ASSERT_ATTR_STRING() do { \
	if (entry->type != KEST_EFF_ENTRY_TYPE_STR) { \
		kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be a string", type, name, key); \
		ret_val = ERR_BAD_ARGS; \
		goto extract_finish; \
	} \
	else \
	{ \
		if (!entry->value.val_string) { \
			KEST_PRINTF_FORCE("entry->value.val_string = NULL. Bug ?\n"); \
			ret_val = ERR_BAD_ARGS; \
			goto extract_finish; \
		} \
	} \
} while (0)

#define ASSERT_ATTR_LIST() do { \
	if (entry->type != KEST_EFF_ENTRY_TYPE_LIST) { \
		kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be a list", type, name, key); \
		ret_val = ERR_BAD_ARGS; \
		goto extract_finish; \
	} \
	else \
	{ \
		if (!entry->value.val_expr) { \
			KEST_PRINTF_FORCE("entry->value.val_list = NULL. Bug ?\n"); \
			ret_val = ERR_BAD_ARGS; \
			goto extract_finish; \
		} \
	} \
} while (0)

#define ASSERT_ATTR_DICT() do { \
	if (entry->type != KEST_EFF_ENTRY_TYPE_SUBDICT) { \
		kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be a dict", type, name, key); \
		ret_val = ERR_BAD_ARGS; \
		goto extract_finish; \
	} \
	else \
	{ \
		if (!entry->value.val_expr) { \
			KEST_PRINTF_FORCE("entry->value.val_dict = NULL. Bug ?\n"); \
			ret_val = ERR_BAD_ARGS; \
			goto extract_finish; \
		} \
	} \
} while (0)

// "name",  "default",  "min", "max", "scale",  "units", "max_velocity", "widget", "group", "driver"

kest_parameter *kest_extract_parameter(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name)
{
	KEST_PRINTF("kest_extract_parameter(ps = %p, dict = %p, name = \"%s\")\n",
		ps, dict, name ? name : "(NULL)");
	if (!dict || !name)
		return NULL;
	
	const char *type = "Parameter";
	
	// Before anything else, check for mandatory attributes
	CHECK_MANDATORY_ATTR("name");
	CHECK_MANDATORY_ATTR("default");
	CHECK_MANDATORY_ATTR("min");
	CHECK_MANDATORY_ATTR("max");
	
	kest_parameter *param = kest_allocator_alloc(&kest_parameter_allocator, sizeof(kest_parameter));
	
	if (!param)
		return NULL;
	
	int ret_val = NO_ERROR;
	size_t initial_driver_count = ps->drivers.count;
	int had_driver_storage = ps->drivers.entries != NULL;

	init_parameter_str(param);
	param->name_internal = kest_strndup(name, 128);
	if (!param->name_internal)
	{
		ret_val = ERR_ALLOC_FAIL;
		goto extract_finish;
	}
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	KEST_PRINTF("kest_eff_entry_dict_count(dict) = %d\n", n);
	
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	const char *string = NULL;
	kest_expression *expr = NULL;
	kest_eff_entry_list *list = NULL;
	kest_eff_entry_dict *subdict = NULL;
	
	float value = 0.0f;
	kest_driver driver;
	int j = 0;
	

	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		
		KEST_PRINTF("Entry %d = %p\n", i, entry);
		
		if (!entry)
			continue;
		
		key = entry->name;
		
		KEST_PRINTF("Type: %s. Key: \"%s\"\n", kest_eff_entry_type_to_string_nice(entry->type), key);
		
		expr    = entry->value.val_expr;
		list    = entry->value.val_list;
		subdict = entry->value.val_dict;
		string  = entry->value.val_string;
		
		if (strcmp(key, "name") == 0)
		{
			ASSERT_ATTR_STRING();
			param->name = kest_strndup(string, 128);
			if (!param->name)
			{
				ret_val = ERR_ALLOC_FAIL;
				goto extract_finish;
			}
		}
		else if (strcmp(key, "default") == 0)
		{
			ASSERT_ATTR_CONST_EXPR();
			param->value = value;
		}
		else if (strcmp(key, "min") == 0)
		{
			ASSERT_ATTR_EXPR();
			param->min_expr = expr;
		}
		else if (strcmp(key, "max") == 0)
		{
			ASSERT_ATTR_EXPR();
			param->max_expr = expr;
		}
		else if (strcmp(key, "scale") == 0)
		{
			ASSERT_ATTR_STRING();
			
			if (strcmp(string, "logarithmic") == 0
			 || strcmp(string, "log"		) == 0)
			{
				param->scale = PARAMETER_SCALE_LOGARITHMIC;
			}
			else if (strcmp(string, "linear") == 0)
			{
				param->scale = PARAMETER_SCALE_LINEAR;
			}
			else
			{
				kest_parser_error(ps, "%s \"%s\": unknown scale \"%s\"", type, name, string); \
				ret_val = ERR_BAD_ARGS;
			}
		}
		else if (strcmp(key, "units") == 0)
		{
			ASSERT_ATTR_STRING();
			param->units = kest_strndup(string, 32);
			if (!param->units)
			{
				ret_val = ERR_ALLOC_FAIL;
				goto extract_finish;
			}
		}
		else if (strcmp(key, "widget") == 0)
		{
			ASSERT_ATTR_STRING();
			
			if (strcmp(string, "dial") == 0 || strcmp(string, "pot") == 0)
			{
				param->widget_type = PARAM_WIDGET_VIRTUAL_POT;
			}
			else if (strcmp(string, "slider" ) == 0 || strcmp(string, "slider_horizontal") == 0 || 
				     strcmp(string, "hslider") == 0)
			{
				param->widget_type = PARAM_WIDGET_HSLIDER;
			}
			else if (strcmp(string, "slider_vertical") == 0
				  || strcmp(string, "vslider")  == 0)
			{
				param->widget_type = PARAM_WIDGET_VSLIDER;
			}
			else if (strcmp(string, "slider_tall_vertical")  == 0 || strcmp(string, "slider_vertical_tall") == 0
				  || strcmp(string, "vslider_tall")  == 0)
			{
				param->widget_type = PARAM_WIDGET_VSLIDER_TALL;
			}
			else
			{
				kest_parser_error(ps, "%s \"%s\": unknown widget \"%s\"", type, name, string);
				ret_val = ERR_BAD_ARGS;
			}
		}
		else if (strcmp(key, "group") == 0)
		{
			ASSERT_ATTR_CONST_INT();
			
			if (value < 0 || value > EFFECT_VIEW_MAX_GROUPS)
			{
				kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be in \{0, 1, ..., %d\}\n", type, name, key, EFFECT_VIEW_MAX_GROUPS - 1); \
				ret_val = ERR_BAD_ARGS;
			}
			
			param->group = (int)value;
		}
		else if (strcmp(key, "max_velocity") == 0)
		{
			ASSERT_ATTR_CONST_EXPR();
			
			if (value <= 0)
			{
				kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be positive", type, name, key); \
				ret_val = ERR_BAD_ARGS;
			}
			else
			{
				param->max_velocity = value;
			}
		}
		else if (strcmp(key, "driver") == 0)
		{
			ASSERT_ATTR_EXPR();
				
			switch (expr->type)
			{
				case KEST_EXPR_REF:
					j = ps->drivers.count;
					KEST_PRINTF("Creating driver \"%s\". ps->drivers.count = %d\n", expr->val.ref_name, j);
					int driver_ret = kest_driver_init_scope_entry(&driver, expr->val.ref_name);
					if (driver_ret != NO_ERROR)
					{
						ret_val = driver_ret;
						goto extract_finish;
					}
					driver_ret = kest_driver_list_append(&ps->drivers, driver);
					if (driver_ret != NO_ERROR)
					{
						ret_val = driver_ret;
						kest_free(driver.data);
						goto extract_finish;
					}
					param->driver_index = j;
					break;
				
				default:
					kest_parser_error(ps, "%s \"%s\": \"%s\" cannot be a driver.\n", type, name, kest_expression_to_string(expr));
					ret_val = ERR_BAD_ARGS;
			}
		}
		else
		{
			kest_parser_error(ps, "Parameter \"%s\": unrecognised attribute \"%s\"", name, key);
			ret_val = ERR_BAD_ARGS;
			goto extract_finish;
		}
	}
	
extract_finish:
	
	if (ret_val != NO_ERROR)
	{
		// These drivers borrow keys from this parameter's fresh parser expressions.
		while (ps->drivers.count > initial_driver_count)
			kest_free(ps->drivers.entries[--ps->drivers.count].data);
		if (!had_driver_storage)
			kest_driver_list_destroy(&ps->drivers);
		kest_free(param->name_internal);
		
		if (param->name)
			kest_free(param->name);
		
		if (param->units)
			kest_free(param->units);
		
		kest_parameter_free(param);
		param = NULL;
	}
	else
	{
		KEST_PRINTF("Extracted a parameter;\n");
		KEST_PRINTF("\tname: \"%s\"\n", param->name);
		KEST_PRINTF("\tname_internal: \"%s\"\n", param->name_internal);
		KEST_PRINTF("\tvalue: %f\n", param->value);
		KEST_PRINTF("\tmin_expr: %s\n", kest_expression_to_string(param->min_expr));
		KEST_PRINTF("\tmax_expr: %s\n",  kest_expression_to_string(param->max_expr));
		KEST_PRINTF("\tscale: %d\n", param->scale);
		KEST_PRINTF("\tmax_velocity: %f\n", param->max_velocity);
		KEST_PRINTF("\twidget_type: %d\n", param->widget_type);
		KEST_PRINTF("\tgroup: %d\n", param->group);
		KEST_PRINTF("\tdriver_index: %d\n", param->driver_index);
	}
	
	return param;
}

static int extract_setting_options(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, kest_setting *setting)
{
	size_t count = kest_eff_entry_dict_count(dict), bytes = 0;
	if (!count || count > 32768 || setting->min < 0 || setting->max > 32767 || setting->min > setting->max)
		goto invalid;
	for (size_t i = 0; i < count; i++)
	{
		kest_eff_entry *choice = kest_eff_entry_dict_index(dict, i);
		if (!choice || choice->type != KEST_EFF_ENTRY_TYPE_SUBDICT || !choice->value.val_dict)
			goto invalid;
		kest_eff_entry_dict *fields = choice->value.val_dict;
		kest_eff_entry *name = kest_eff_entry_dict_lookup(fields, "name");
		kest_eff_entry *value = kest_eff_entry_dict_lookup(fields, "value");
		if (kest_eff_entry_dict_count(fields) != 2 || !name || name->type != KEST_EFF_ENTRY_TYPE_STR ||
			!name->value.val_string || !value || value->type != KEST_EFF_ENTRY_TYPE_EXPR ||
			!value->value.val_expr || !kest_expression_is_constant(value->value.val_expr))
			goto invalid;
		size_t length = strlen(name->value.val_string);
		if (!length || length >= 128 || strpbrk(name->value.val_string, "\r\n")) goto invalid;
		bytes += length + 1;
	}
	/* Descriptor labels live with this array; clones borrow them while retaining the descriptor. */
	setting->options = kest_alloc(count * sizeof(kest_setting_option) + bytes);
	if (!setting->options) return ERR_ALLOC_FAIL;
	setting->n_options = (int)count;
	char *label = (char *)(setting->options + count);
	int has_default = 0;
	for (size_t i = 0; i < count; i++)
	{
		kest_eff_entry_dict *fields = kest_eff_entry_dict_index(dict, i)->value.val_dict;
		const char *name = kest_eff_entry_dict_lookup(fields, "name")->value.val_string;
		float value = kest_expression_evaluate(kest_eff_entry_dict_lookup(fields, "value")->value.val_expr, NULL);
		if (!isfinite(value) || value != floorf(value) || value < setting->min || value > setting->max)
			goto invalid;
		for (size_t j = 0; j < i; j++)
			if (setting->options[j].value == value || strcmp(setting->options[j].name, name) == 0)
				goto invalid;
		setting->options[i].value = (uint16_t)value;
		setting->options[i].name = label;
		size_t length = strlen(name) + 1;
		memcpy(label, name, length);
		label += length;
		if (value == setting->value) has_default = 1;
	}
	if (has_default) return NO_ERROR;
invalid:
	kest_parser_error(ps, "Setting \"%s\": invalid enum choices or default", setting->name_internal);
	return ERR_BAD_ARGS;
}

kest_setting *kest_extract_setting(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name)
{
	if (!dict || !name)
		return NULL;
	
	// Before anything else, check for mandatory attributes
	
	const char *type = "Setting";
	
	CHECK_MANDATORY_ATTR("name");
	CHECK_MANDATORY_ATTR("default");
	CHECK_MANDATORY_ATTR("min");
	CHECK_MANDATORY_ATTR("max");
	CHECK_MANDATORY_ATTR("type");
	
	kest_setting *setting = kest_allocator_alloc(&kest_setting_allocator, sizeof(kest_setting));
	
	if (!setting)
		return NULL;
	
	int ret_val = NO_ERROR;

	init_setting_str(setting);
	kest_eff_entry_dict *options_dict = NULL;
	setting->name_internal = kest_strndup(name, 128);
	if (!setting->name_internal)
	{
		ret_val = ERR_ALLOC_FAIL;
		goto extract_finish;
	}
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	const char *string = NULL;
	kest_expression *expr = NULL;
	kest_eff_entry_list *list = NULL;
	kest_eff_entry_dict *subdict = NULL;
	
	float value = 0.0f;
	

	
	int widget_set = 0;
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		
		if (!entry)
			continue;
		
		key = entry->name;
		
		expr    = entry->value.val_expr;
		list    = entry->value.val_list;
		subdict = entry->value.val_dict;
		string  = entry->value.val_string;
		
		if (strcmp(key, "name") == 0)
		{
			ASSERT_ATTR_STRING();
			setting->name = kest_strndup(string, 128);
			if (!setting->name)
			{
				ret_val = ERR_ALLOC_FAIL;
				goto extract_finish;
			}
		}
		else if (strcmp(key, "default") == 0 || strcmp(key, "min") == 0 || strcmp(key, "max") == 0)
		{
			ASSERT_ATTR_CONST_EXPR();
			if (!isfinite(value) || value != floorf(value) || (double)value < INT_MIN || (double)value > INT_MAX)
			{
				kest_parser_error(ps, "Setting \"%s\": %s must fit an integer", name, key);
				ret_val = ERR_BAD_ARGS;
				goto extract_finish;
			}
			if (strcmp(key, "default") == 0) setting->value = (int)value;
			else if (strcmp(key, "min") == 0) setting->min = (int)value;
			else setting->max = (int)value;
		}
		else if (strcmp(key, "type") == 0)
		{
			ASSERT_ATTR_STRING();
			
			if (strcmp(string, "enum") == 0)
			{
				setting->type = EFFECT_SETTING_ENUM;
				
				if (!widget_set)
				{
					setting->widget_type = SETTING_WIDGET_DROPDOWN;
				}
			}
			else if (strcmp(string, "bool") == 0)
			{
				setting->type = EFFECT_SETTING_BOOL;
				
				if (!widget_set)
				{
					setting->widget_type = SETTING_WIDGET_SWITCH;
				}
			}
			else if (strcmp(string, "int") == 0)
			{
				setting->type = EFFECT_SETTING_INT;
				
				if (!widget_set)
				{
					setting->widget_type = SETTING_WIDGET_FIELD;
				}
			}
			else
			{
				kest_parser_error(ps, "%s \"%s\": unknown setting type \"%s\"", type, name, string);
				ret_val = ERR_BAD_ARGS;
			}
		}
		else if (strcmp(key, "units") == 0)
		{
			ASSERT_ATTR_STRING();
			setting->units = kest_strndup(string, 32);
			if (!setting->units)
			{
				ret_val = ERR_ALLOC_FAIL;
				goto extract_finish;
			}
		}
		else if (strcmp(key, "widget") == 0)
		{
			ASSERT_ATTR_STRING();
			
			if (strcmp(string, "dropdown") == 0)
			{
				setting->page = SETTING_WIDGET_DROPDOWN;
				widget_set = 1;
			}
			else if (strcmp(string, "field") == 0)
			{
				setting->page = SETTING_WIDGET_FIELD;
				widget_set = 1;
			}
			else if (strcmp(string, "switch") == 0)
			{
				setting->page = SETTING_WIDGET_SWITCH;
				widget_set = 1;
			}
			else
			{
				kest_parser_error(ps, "%s \"%s\": unknown setting widget type \"%s\"", type, name, string);
				ret_val = ERR_BAD_ARGS;
			}
		}
		else if (strcmp(key, "page") == 0)
		{
			ASSERT_ATTR_STRING();
			
			if (strcmp(string, "main") == 0)
			{
				setting->page = EFFECT_SETTING_PAGE_MAIN;
			}
			else if (strcmp(string, "settings") == 0)
			{
				setting->page = EFFECT_SETTING_PAGE_SETTINGS;
			}
			else
			{
				kest_parser_error(ps, "%s \"%s\": unknown page \"%s\"", type, name, string);
				ret_val = ERR_BAD_ARGS;
			}
		}
		else if (strcmp(key, "group") == 0)
		{
			ASSERT_ATTR_CONST_INT();
			
			if (value < 0 || value > EFFECT_VIEW_MAX_GROUPS)
			{
				kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be in \{0, 1, ..., %d\}\n", type, name, key, EFFECT_VIEW_MAX_GROUPS - 1);
				ret_val = ERR_BAD_ARGS;
			}
			
			setting->group = (int)value;
		}
		else if (strcmp(key, "options") == 0)
		{
			ASSERT_ATTR_DICT();
			options_dict = subdict;
		}
		else
		{
			kest_parser_error(ps, "Setting \"%s\": unrecognised attribute \"%s\"", name, key);
			ret_val = ERR_BAD_ARGS;
			goto extract_finish;
		}
	}
	if (setting->min > setting->max || setting->value < setting->min || setting->value > setting->max)
	{
		kest_parser_error(ps, "Setting \"%s\": invalid bounds or default", name);
		ret_val = ERR_BAD_ARGS;
	}
	else if (setting->type == EFFECT_SETTING_ENUM)
		ret_val = extract_setting_options(ps, options_dict, setting);
	else if (options_dict)
	{
		kest_parser_error(ps, "Setting \"%s\": options require enum type", name);
		ret_val = ERR_BAD_ARGS;
	}
	
extract_finish:
	
	if (ret_val != NO_ERROR)
	{
		kest_free(setting->name_internal);
		
		if (setting->name)
			kest_free(setting->name);
		
		gut_setting(setting);
		kest_free(setting->units);
		kest_allocator_free(&kest_setting_allocator, setting);
		setting = NULL;
	}
	else
	{
		KEST_PRINTF("Extracted a setting;\n");
		KEST_PRINTF("\tname: \"%s\"\n", setting->name);
		KEST_PRINTF("\tname_internal: \"%s\"\n", setting->name_internal);
		KEST_PRINTF("\tpage: %s\n", (setting->page == EFFECT_SETTING_PAGE_MAIN) ? "main" : "settings");
	}
	
	return setting;
}

kest_dsp_resource *kest_extract_mem		  (kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name);
kest_dsp_resource *kest_extract_delay	  (kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name);
kest_dsp_resource *kest_extract_filter	  (kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name);
kest_dsp_resource *kest_extract_lpf		  (kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name);
kest_dsp_resource *kest_extract_hpf		  (kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name);
kest_dsp_resource *kest_extract_bpf		  (kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name);
kest_dsp_resource *kest_extract_lfo		  (kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name);
kest_dsp_resource *kest_extract_polynomial(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name);

kest_dsp_resource *kest_extract_resource(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name)
{
	if (!dict || !name)
		return NULL;
	
	kest_eff_entry *type_entry = kest_eff_entry_dict_lookup(dict, "type");
	
	if (!type_entry)
	{
		kest_parser_error(ps, "Resource \"%s\": mandatory attribute \"type\" not found", name);
		return NULL;
	}
	
	if (type_entry->type != KEST_EFF_ENTRY_TYPE_STR || !type_entry->value.val_string)
	{
		kest_parser_error(ps, "Resource \"%s\": attribute \"type\" must be a string", name);
		return NULL;
	}
	const char *type_string = type_entry->value.val_string;
	
	if (strcmp(type_string, "mem") == 0)
	{
		return kest_extract_mem(ps, dict, name);
	}
	else if (strcmp(type_string, "delay") == 0)
	{
		return kest_extract_delay(ps, dict, name);
	}
	else if (strcmp(type_string, "filter") == 0)
	{
		return kest_extract_filter(ps, dict, name);
	}
	else if (strcmp(type_string, "lpf") == 0)
	{
		return kest_extract_lpf(ps, dict, name);
	}
	else if (strcmp(type_string, "hpf") == 0)
	{
		return kest_extract_hpf(ps, dict, name);
	}
	else if (strcmp(type_string, "bpf") == 0)
	{
		return kest_extract_bpf(ps, dict, name);
	}
	else if (strcmp(type_string, "polynomial") == 0)
	{
		return kest_extract_polynomial(ps, dict, name);
	}
	else if (strcmp(type_string, "lfo") == 0)
	{
		return kest_extract_lfo(ps, dict, name);
	}
	else
	{
		kest_parser_error(ps, "%s: resource type \"%s\" unrecognised\n", name, type_string);
		return NULL;
	}
}

/*
 * char *name;
	int type;
	int handle;
	int mem_size;
	struct kest_expression *size;
	struct kest_expression *delay;
	void *data;
  */

kest_dsp_resource *kest_extract_mem(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name)
{
	if (!dict || !name)
		return NULL;
	
	const char *type = "Resource";
	
	kest_dsp_resource *resource = kest_allocator_alloc(&kest_dsp_resource_allocator, sizeof(kest_dsp_resource)); // Survives parser arena reset.
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_MEM;
	resource->mem_size = 1;
	
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	kest_mem_slot *mem = kest_mem_slot_create(NULL);
	
	if (!mem)
	{
		if (resource->name)
			kest_free(resource->name);
		
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		
		return NULL;
	}
	
	resource->data = mem;
	
	mem->read_enable = 1;
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	const char *string = NULL;
	kest_expression *expr = NULL;
	kest_eff_entry_list *list = NULL;
	kest_eff_entry_dict *subdict = NULL;
	
	float value = 0.0f;
	
	int ret_val = NO_ERROR;
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		
		if (!entry)
			continue;
		
		key = entry->name;
		
		expr    = entry->value.val_expr;
		list    = entry->value.val_list;
		subdict = entry->value.val_dict;
		string  = entry->value.val_string;
		
		if (strcmp(key, "type") == 0)
		{
			
		}
		else
		{
			kest_parser_error(ps, "Resource \"%s\": attribute \"%s\" unrecognised", name, key);
			ret_val = ERR_BAD_ARGS;
			goto extract_finish;
		}
	}
	
extract_finish:
	
	if (ret_val != NO_ERROR)
	{
		kest_free(mem);
		if (resource->name)
			kest_free(resource->name);
		
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		resource = NULL;
	}
	
	return resource;
}

kest_dsp_resource *kest_extract_delay(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name)
{
	if (!dict || !name)
		return NULL;
	
	const char *type = "Resource";
	
	int delay_ms = !!kest_eff_entry_dict_lookup(dict, "delay_ms");
	int delay_   = !!kest_eff_entry_dict_lookup(dict, "delay");
	int delay_s  = !!kest_eff_entry_dict_lookup(dict, "delay_seconds");
	int delay_samples = !!kest_eff_entry_dict_lookup(dict, "delay_samples");
	
	if (delay_ + delay_ms + delay_s + delay_samples != 1)
	{
		kest_parser_error(ps, "Resource \"%s\": delay ill-specified. Need delay amount in exactly one unit; %d provided",
			name, delay_ + delay_ms + delay_s + delay_samples);
		return NULL;
	}
	
	kest_dsp_resource *resource = kest_allocator_alloc(&kest_dsp_resource_allocator, sizeof(kest_dsp_resource)); // Survives parser arena reset.
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_DELAY;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	kest_delay *delay = kest_delay_create(NULL);
	
	if (!delay)
	{
		kest_free(resource->name);
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	resource->data = delay;
	
	delay->units = KEST_DELAY_UNITS_MS;
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	const char *string = NULL;
	kest_expression *expr = NULL;
	kest_eff_entry_list *list = NULL;
	kest_eff_entry_dict *subdict = NULL;
	
	float value = 0.0f;
	
	int ret_val = NO_ERROR;
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		
		if (!entry)
			continue;
		
		key = entry->name;
		
		expr    = entry->value.val_expr;
		list    = entry->value.val_list;
		subdict = entry->value.val_dict;
		string  = entry->value.val_string;
		
		if (strcmp(key, "type") == 0)
		{
			
		}
		else if (strcmp(key, "delay") == 0 || strcmp(key, "delay") == 0) // If unspecified, assume ms
		{
			//ASSERT_ATTR_CONST_EXPR();
			delay->delay = expr;
			delay->units = KEST_DELAY_UNITS_MS;
		}
		else if (strcmp(key, "delay_seconds") == 0)
		{
			//ASSERT_ATTR_CONST_EXPR();
			delay->delay = expr;
			delay->units = KEST_DELAY_UNITS_SECONDS;
		}
		else if (strcmp(key, "delay_samples") == 0)
		{
			//ASSERT_ATTR_CONST_INT();
			delay->delay = expr;
			delay->units = KEST_DELAY_UNITS_SAMPLES;
		}
		else if (strcmp(key, "size") == 0)
		{
			//ASSERT_ATTR_CONST_EXPR();
			delay->size = expr;
		}
		else
		{
			kest_parser_error(ps, "Resource \"%s\": attribute \"%s\" unrecognised", name, key);
			ret_val = ERR_BAD_ARGS;
			goto extract_finish;
		}
	}
	
extract_finish:
	
	if (ret_val != NO_ERROR)
	{
		kest_free(delay);
		if (resource->name)
			kest_free(resource->name);
		
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		resource = NULL;
	}
	
	return resource;
}


kest_dsp_resource *kest_extract_filter(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name)
{
	if (!dict || !name)
		return NULL;
	
	// Before anything else, check for mandatory attributes
	const char *type = "Resource";
	CHECK_MANDATORY_ATTR("coefs");
	
	kest_dsp_resource *resource = kest_allocator_alloc(&kest_dsp_resource_allocator, sizeof(kest_dsp_resource)); // Survives parser arena reset.
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_FILTER;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
	{
		kest_free(resource->name);
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	resource->data = (void*)filter;
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	const char *string = NULL;
	kest_expression *expr = NULL;
	kest_eff_entry_list *list = NULL;
	kest_eff_entry_dict *subdict = NULL;
	
	float value = 0.0f;
	
	int ret_val = NO_ERROR;
	
	int feed_back_set = 0;
	int feed_forward_set = 0;
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		
		if (!entry)
			continue;
		
		key = entry->name;
		
		expr    = entry->value.val_expr;
		list    = entry->value.val_list;
		subdict = entry->value.val_dict;
		string  = entry->value.val_string;
		
		if (strcmp(key, "type") == 0)
		{
			
		}
		else if (strcmp(key, "feed_forward") == 0)
		{
			ASSERT_ATTR_CONST_INT();
			
			if (value <= 0)
			{
				kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be positive", type, name, key); \
				ret_val = ERR_BAD_ARGS;
			}
			else
			{
				filter->feed_forward = (int)value;
				feed_forward_set = 1;
			}
			
		}
		else if (strcmp(key, "feed_back") == 0)
		{
			ASSERT_ATTR_CONST_INT();
			
			if (value < 0)
			{
				kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be non-negative", type, name, key); \
				ret_val = ERR_BAD_ARGS;
			}
			else
			{
				filter->feed_back = (int)value;
				feed_back_set = 1;
			}
			
		}
		else if (strcmp(key, "coefs") == 0)
		{
			ASSERT_ATTR_LIST();
			
			for (int i = 0; i < list->count; i++)
			{
				if (list->entries[i].type != KEST_EFF_ENTRY_TYPE_EXPR)
				{
					kest_parser_error(ps, "Filter \"%s\": Filter coefficients must be expressions, but coefficient %d of filter \"%s\" is a %s (%d).",
						name, i, resource->name, kest_eff_entry_type_to_string_nice(list->entries[i].type), list->entries[i].type);
					ret_val = ERR_BAD_ARGS;
				}
				
				if (kest_expression_ptr_list_append(&filter->coefs, list->entries[i].value.val_expr) != NO_ERROR)
					ret_val = ERR_ALLOC_FAIL;
			}
		}
		else
		{
			kest_parser_error(ps, "Resource \"%s\": attribute \"%s\" unrecognised", name, key);
			ret_val = ERR_BAD_ARGS;
			goto extract_finish;
		}
	}
	
extract_finish:
	
	if (!filter || ret_val != NO_ERROR)
	{
		if (filter)
		{
			kest_expression_ptr_list_destroy(&filter->coefs);
			kest_free(filter);
		}
		
		if (resource->name)
			kest_free(resource->name);
		
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		resource = NULL;
	}
	else
	{
		if (feed_back_set + feed_forward_set == 2 && !(filter->feed_forward + filter->feed_back == filter->coefs.count))
		{
			kest_parser_error(ps, "Resource \"%s\": a (%d, %d) filter requires %d coefficients, but %d given", name,
				filter->feed_forward, filter->feed_back, filter->feed_forward + filter->feed_back, filter->coefs.count);
			
			kest_expression_ptr_list_destroy(&filter->coefs);
			kest_free(filter);
			
			if (resource->name)
				kest_free(resource->name);
			
			kest_allocator_free(&kest_dsp_resource_allocator, resource);
			resource = NULL;
		}
		else
		{
			if (feed_forward_set)
				filter->feed_back = filter->coefs.count - filter->feed_forward;
			else
				filter->feed_forward = filter->coefs.count - filter->feed_back;
		}
	}
	
	return resource;
}

kest_dsp_resource *kest_extract_lpf(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name)
{
	if (!dict || !name)
		return NULL;
	
	// Before anything else, check for mandatory attributes
	const char *type = "Resource";
	CHECK_MANDATORY_ATTR("cutoff");
	
	kest_dsp_resource *resource = kest_allocator_alloc(&kest_dsp_resource_allocator, sizeof(kest_dsp_resource)); // Survives parser arena reset.
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_FILTER;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
	{
		kest_free(resource->name);
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	resource->data = (void*)filter;
	
	filter->feed_forward = 3;
	filter->feed_back = 2;
	
	kest_expression *Q = &kest_expression_root_2_over_2;
	kest_expression *cutoff = NULL;
	kest_expression *coefs[5];
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	const char *string = NULL;
	kest_expression *expr = NULL;
	kest_eff_entry_list *list = NULL;
	kest_eff_entry_dict *subdict = NULL;
	
	float value = 0.0f;
	int ret_val = NO_ERROR;
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		
		if (!entry)
			continue;
		
		key = entry->name;
		
		expr    = entry->value.val_expr;
		list    = entry->value.val_list;
		subdict = entry->value.val_dict;
		string  = entry->value.val_string;
		
		if (strcmp(key, "type") == 0)
		{
			
		}
		else if (strcmp(key, "cutoff") == 0)
		{
			ASSERT_ATTR_EXPR();
			cutoff = entry->value.val_expr;
		}
		else if (strcmp(key, "Q") == 0)
		{
			ASSERT_ATTR_EXPR();
			Q = entry->value.val_expr;
		}
		else
		{
			kest_parser_error(ps, "Resource \"%s\": attribute \"%s\" unrecognised", name, key);
			ret_val = ERR_BAD_ARGS;
			goto extract_finish;
		}
	}
	
extract_finish:
	
	if (ret_val != NO_ERROR)
	{
		if (filter)
		{
			kest_expression_ptr_list_destroy(&filter->coefs);
			
			kest_free(filter);
		}
		
		if (resource->name)
			kest_free(resource->name);
		
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		resource = NULL;
	}
	else
	{
		ret_val = kest_expr_create_lpf_coefficients(coefs, cutoff, Q);
	
		if (ret_val != NO_ERROR)
		{
			kest_expression_ptr_list_destroy(&filter->coefs);
			
			kest_free(filter);
			kest_free(resource->name);
			kest_allocator_free(&kest_dsp_resource_allocator, resource);
			
			return NULL;
		}
		
		kest_expression_ptr_list_append(&filter->coefs, coefs[0]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[1]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[2]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[3]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[4]);
	}
	
	return resource;
}


kest_dsp_resource *kest_extract_hpf(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name)
{
	if (!dict || !name)
		return NULL;
	
	// Before anything else, check for mandatory attributes
	const char *type = "Resource";
	CHECK_MANDATORY_ATTR("cutoff");
	
	kest_dsp_resource *resource = kest_allocator_alloc(&kest_dsp_resource_allocator, sizeof(kest_dsp_resource)); // Survives parser arena reset.
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_FILTER;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
	{
		kest_free(resource->name);
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	resource->data = (void*)filter;
	
	filter->feed_forward = 3;
	filter->feed_back = 2;
	
	kest_expression *Q = &kest_expression_root_2_over_2;
	kest_expression *cutoff = NULL;
	kest_expression *coefs[5];
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	const char *string = NULL;
	kest_expression *expr = NULL;
	kest_eff_entry_list *list = NULL;
	kest_eff_entry_dict *subdict = NULL;
	
	float value = 0.0f;
	int ret_val = NO_ERROR;
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		
		if (!entry)
			continue;
		
		key = entry->name;
		
		expr    = entry->value.val_expr;
		list    = entry->value.val_list;
		subdict = entry->value.val_dict;
		string  = entry->value.val_string;
		
		if (strcmp(key, "type") == 0)
		{
			
		}
		else if (strcmp(key, "cutoff") == 0)
		{
			ASSERT_ATTR_EXPR();
			cutoff = entry->value.val_expr;
		}
		else if (strcmp(key, "Q") == 0)
		{
			ASSERT_ATTR_EXPR();
			Q = entry->value.val_expr;
		}
		else
		{
			kest_parser_error(ps, "Resource \"%s\": attribute \"%s\" unrecognised", name, key);
			ret_val = ERR_BAD_ARGS;
			goto extract_finish;
		}
	}
	
extract_finish:
	
	if (ret_val != NO_ERROR)
	{
		if (filter)
		{
			kest_expression_ptr_list_destroy(&filter->coefs);
			kest_free(filter);
		}
		
		if (resource->name)
			kest_free(resource->name);
		
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		resource = NULL;
	}
	else
	{
		ret_val = kest_expr_create_hpf_coefficients(coefs, cutoff, Q);
	
		if (ret_val != NO_ERROR)
		{
			kest_expression_ptr_list_destroy(&filter->coefs);
			
			kest_free(filter);
			kest_free(resource->name);
			kest_allocator_free(&kest_dsp_resource_allocator, resource);
			
			return NULL;
		}
		
		kest_expression_ptr_list_append(&filter->coefs, coefs[0]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[1]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[2]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[3]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[4]);
	}
	
	return resource;
}


kest_dsp_resource *kest_extract_bpf(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name)
{
	if (!dict || !name)
		return NULL;
	
	// Before anything else, check for mandatory attributes
	const char *type = "Resource";
	CHECK_MANDATORY_ATTR("center");
	
	kest_dsp_resource *resource = kest_allocator_alloc(&kest_dsp_resource_allocator, sizeof(kest_dsp_resource)); // Survives parser arena reset.
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_FILTER;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
	{
		kest_free(resource->name);
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	resource->data = (void*)filter;
	
	filter->feed_forward = 3;
	filter->feed_back = 2;
	
	kest_expression *Q = &kest_expression_root_2_over_2;
	kest_expression *center = NULL;
	kest_expression *coefs[5];
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	const char *string = NULL;
	kest_expression *expr = NULL;
	kest_eff_entry_list *list = NULL;
	kest_eff_entry_dict *subdict = NULL;
	
	float value = 0.0f;
	int ret_val = NO_ERROR;
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		
		if (!entry)
			continue;
		
		key = entry->name;
		
		expr    = entry->value.val_expr;
		list    = entry->value.val_list;
		subdict = entry->value.val_dict;
		string  = entry->value.val_string;
		
		if (strcmp(key, "type") == 0)
		{
			
		}
		else if (strcmp(key, "center") == 0)
		{
			ASSERT_ATTR_EXPR();
			center = entry->value.val_expr;
		}
		else if (strcmp(key, "Q") == 0)
		{
			ASSERT_ATTR_EXPR();
			Q = entry->value.val_expr;
		}
		else if (strcmp(key, "band_width") == 0)
		{
			
		}
		else
		{
			kest_parser_error(ps, "Resource \"%s\": attribute \"%s\" unrecognised", name, key);
			ret_val = ERR_BAD_ARGS;
			goto extract_finish;
		}
	}
	
extract_finish:
	
	if (ret_val != NO_ERROR)
	{
		if (filter)
		{
			kest_expression_ptr_list_destroy(&filter->coefs);
			
			kest_free(filter);
		}
		
		if (resource->name)
			kest_free(resource->name);
		
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		resource = NULL;
	}
	else
	{
		ret_val = kest_expr_create_bpf_coefficients(coefs, center, Q);
	
		if (ret_val != NO_ERROR)
		{
			kest_expression_ptr_list_destroy(&filter->coefs);
			
			kest_free(filter);
			kest_free(resource->name);
			kest_allocator_free(&kest_dsp_resource_allocator, resource);
			
			return NULL;
		}
		
		kest_expression_ptr_list_append(&filter->coefs, coefs[0]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[1]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[2]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[3]);
		kest_expression_ptr_list_append(&filter->coefs, coefs[4]);
	}
	
	return resource;
}


kest_dsp_resource *kest_extract_lfo(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name)
{
	if (!dict || !name)
		return NULL;
	
	const char *type = "Resource";
	
	kest_eff_entry *freq 	= kest_eff_entry_dict_lookup(dict, "freq");
	
	if (!freq)
		freq = kest_eff_entry_dict_lookup(dict, "frequency");
	
	if (!freq)
	{
		kest_parser_error(ps, "LFO \"%s\": mandatory attribute \"frequency\" not found", name);
		return NULL;
	}
	
	kest_eff_entry *center 	= kest_eff_entry_dict_lookup(dict, "center");
	kest_eff_entry *amp 	= kest_eff_entry_dict_lookup(dict, "amplitude");
	kest_eff_entry *min 	= kest_eff_entry_dict_lookup(dict, "min");
	kest_eff_entry *max		= kest_eff_entry_dict_lookup(dict, "max");
	
	int center_amplitude = (!!center) && (!!amp);
	int min_max = (!!min) && (!!max);
	
	if ((center_amplitude) + min_max != 1)
	{
		kest_parser_error(ps, "LFO \"%s\": LFO ill-specified. Need either center & amplitude or min & max", name);
		return NULL;
	}
	
	kest_dsp_resource *resource = kest_allocator_alloc(&kest_dsp_resource_allocator, sizeof(kest_dsp_resource)); // Survives parser arena reset.
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	resource->type = KEST_DSP_RESOURCE_LFO;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	kest_lfo *lfo = kest_lfo_create(NULL);
	
	if (!lfo)
	{
		kest_free(resource->name);
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	if (center_amplitude)
		lfo->mode = KEST_LFO_MODE_CENTER_AMP;
	else
		lfo->mode = KEST_LFO_MODE_MIN_MAX;
	
	lfo->scale = KEST_LFO_SCALE_LINEAR;
	
	resource->data = (void*)lfo;
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	const char *string = NULL;
	kest_expression *expr = NULL;
	kest_eff_entry_list *list = NULL;
	kest_eff_entry_dict *subdict = NULL;
	
	float value = 0.0f;
	
	int ret_val = NO_ERROR;
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		
		if (!entry)
			continue;
		
		key = entry->name;
		
		expr    = entry->value.val_expr;
		list    = entry->value.val_list;
		subdict = entry->value.val_dict;
		string  = entry->value.val_string;
		
		if (strcmp(key, "type") == 0)
		{
			
		}
		else if (strcmp(key, "freq") == 0 || strcmp(key, "frequency") == 0)
		{
			ASSERT_ATTR_EXPR();
			lfo->frequency = expr;
		}
		else if (strcmp(key, "center") == 0)
		{
			ASSERT_ATTR_EXPR();
			lfo->center = expr;
		}
		else if (strcmp(key, "amplitude") == 0)
		{
			ASSERT_ATTR_EXPR();
			lfo->amplitude = expr;
		}
		else if (strcmp(key, "min") == 0)
		{
			ASSERT_ATTR_EXPR();
			lfo->min = expr;
		}
		else if (strcmp(key, "max") == 0)
		{
			ASSERT_ATTR_EXPR();
			lfo->max = expr;
		}
		else if (strcmp(key, "scale") == 0)
		{
			ASSERT_ATTR_STRING();
			
			if (strcmp(string, "log") == 0 || strcmp(string, "logarithmic") == 0)
			{
				lfo->scale = KEST_LFO_SCALE_LOG;
			}
			else if (strcmp(string, "linear") == 0)
			{
				lfo->scale = KEST_LFO_SCALE_LINEAR;
			}
			else
			{
				kest_parser_error(ps, "%s \"%s\": unknown scale \"%s\"", type, name, string); \
				ret_val = ERR_BAD_ARGS;
			}
		}
		else
		{
			kest_parser_error(ps, "Resource \"%s\": attribute \"%s\" unrecognised", name, key);
			ret_val = ERR_BAD_ARGS;
			goto extract_finish;
		}
	}
	
extract_finish:
	
	if (ret_val != NO_ERROR)
	{
		kest_free(lfo);
		if (resource->name)
			kest_free(resource->name);
		
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		resource = NULL;
	}
	
	return resource;
}


kest_dsp_resource *kest_extract_polynomial(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name)
{
	if (!dict || !name)
		return NULL;
	
	// Before anything else, check for mandatory attributes
	const char *type = "Resource";
	CHECK_MANDATORY_ATTR("coefs");
	
	kest_dsp_resource *resource = kest_allocator_alloc(&kest_dsp_resource_allocator, sizeof(kest_dsp_resource)); // Survives parser arena reset.
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	resource->type = KEST_DSP_RESOURCE_FILTER;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
	{
		kest_free(resource->name);
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		return NULL;
	}
	
	resource->data = (void*)filter;
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	const char *string = NULL;
	kest_expression *expr = NULL;
	kest_eff_entry_list *list = NULL;
	kest_eff_entry_dict *subdict = NULL;
	
	float value = 0.0f;
	
	int ret_val = NO_ERROR;
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		
		if (!entry)
			continue;
		
		key = entry->name;
		
		expr    = entry->value.val_expr;
		list    = entry->value.val_list;
		subdict = entry->value.val_dict;
		string  = entry->value.val_string;
		
		if (strcmp(key, "type") == 0)
		{
			
		}
		else if (strcmp(key, "coefs") == 0)
		{
			ASSERT_ATTR_LIST();
			
			for (int i = 0; i < list->count; i++)
			{
				if (list->entries[i].type != KEST_EFF_ENTRY_TYPE_EXPR)
				{
					kest_parser_error(ps, "Filter \"%s\": Filter coefficients must be expressions, but coefficient %d of filter \"%s\" is a %s (%d).",
						name, i, resource->name, kest_eff_entry_type_to_string_nice(list->entries[i].type), list->entries[i].type);
					ret_val = ERR_BAD_ARGS;
				}
				
				if (kest_expression_ptr_list_append(&filter->coefs, list->entries[i].value.val_expr) != NO_ERROR)
					ret_val = ERR_ALLOC_FAIL;
			}
		}
		else
		{
			kest_parser_error(ps, "Resource \"%s\": attribute \"%s\" unrecognised", name, key);
			ret_val = ERR_BAD_ARGS;
			goto extract_finish;
		}
	}
	
extract_finish:
	
	if (!filter || ret_val != NO_ERROR)
	{
		if (filter)
		{
			kest_expression_ptr_list_destroy(&filter->coefs);
			kest_free(filter);
		}
		if (resource->name)
			kest_free(resource->name);
		
		kest_allocator_free(&kest_dsp_resource_allocator, resource);
		resource = NULL;
	}
	else
	{
		filter->feed_forward = filter->coefs.count;
		filter->feed_back = 0;
	}
	
	return resource;
}
