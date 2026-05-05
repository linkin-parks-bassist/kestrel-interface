#include "kest_int.h"

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "kest_dict_extract.c";


int kest_extract_lpf_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_dsp_resource *res)
{
	KEST_PRINTF("kest_extract_lpf_from_dict\n");
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	kest_expression *cutoff = NULL;
	kest_expression *Q = NULL;
	
	int ret_val;
	kest_dictionary_entry entry;
	ret_val = kest_dictionary_lookup_entry(dict, "cutoff", &entry);
	
	if (ret_val != NO_ERROR)
	{
		kest_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"cutoff\" for filter \"%s\"", res->name);
		return ERR_BAD_ARGS;
	}
	
	switch (entry.type)
	{
		case DICT_ENTRY_TYPE_INT:
			cutoff = kest_expr_new_const(entry.value.val_int);
			if (!cutoff) return ERR_ALLOC_FAIL;
			break;
			
		case DICT_ENTRY_TYPE_FLOAT:
			cutoff = kest_expr_new_const(entry.value.val_float);
			if (!cutoff) return ERR_ALLOC_FAIL;
			break;
			
		case DICT_ENTRY_TYPE_EXPR:
			cutoff = entry.value.val_expr;
			if (!cutoff)
				return ERR_BAD_ARGS;
			break;
		
		default:
			kest_parser_error_at_node(ps, dict_node, "Filter \"%s\": Cutoff must be numerical", res->name);
			return ERR_BAD_ARGS;
	}
	
	ret_val = kest_dictionary_lookup_entry(dict, "Q", &entry);
	
	if (ret_val != NO_ERROR)
	{
		ret_val = kest_dictionary_lookup_entry(dict, "resonance", &entry);
	}
	
	if (ret_val != NO_ERROR)
	{
		Q = &kest_expression_root_2_over_2;
	}
	else
	{
		switch (entry.type)
		{
			case DICT_ENTRY_TYPE_INT:
				Q = kest_expr_new_const(entry.value.val_int);
				if (!Q) return ERR_ALLOC_FAIL;
				break;
				
			case DICT_ENTRY_TYPE_FLOAT:
				Q = kest_expr_new_const(entry.value.val_float);
				if (!Q) return ERR_ALLOC_FAIL;
				break;
				
			case DICT_ENTRY_TYPE_EXPR:
				Q = entry.value.val_expr;
				if (!Q)
					return ERR_BAD_ARGS;
				break;
			
			default:
				kest_parser_error_at_node(ps, dict_node, "Filter \"%s\": Cutoff must be numerical", res->name);
				return ERR_BAD_ARGS;
		}
	}
	
	KEST_PRINTF("cutoff  = %s\n", kest_expression_to_string(cutoff));
    KEST_PRINTF("Q       = %s\n", kest_expression_to_string(Q));
	
	kest_filter *filter = kest_filter_create(NULL);
	
	res->data = (void*)filter;
	
	filter->feed_forward = 3;
	filter->feed_back = 2;
	
	kest_expression *coefs[5];
	
	ret_val = kest_expr_create_lpf_coefficients(coefs, cutoff, Q);
	
	if (ret_val != NO_ERROR)
	{
		res->data = NULL;
		kest_expression_ptr_list_destroy(&filter->coefs);
		return ret_val;
	}
	
	kest_expression_ptr_list_append(&filter->coefs, coefs[0]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[1]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[2]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[3]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[4]);
	
	KEST_PRINTF("Extracted (%d, %d) filter \"%s\", with coefficients\n", filter->feed_forward, filter->feed_back, res->name);
	
	for (int i = 0; i < filter->coefs.count; i++)
	{
		KEST_PRINTF("\t%s\n", kest_expression_to_string(filter->coefs.entries[i]));
	}
	
	
	KEST_PRINTF("kest_extract_lpf_from_dict done\n");
	return NO_ERROR;
}

int kest_extract_hpf_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_dsp_resource *res)
{
	KEST_PRINTF("kest_extract_hpf_from_dict\n");
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	kest_expression *cutoff = NULL;
	kest_expression *Q = NULL;
	
	int ret_val;
	kest_dictionary_entry entry;
	ret_val = kest_dictionary_lookup_entry(dict, "cutoff", &entry);
	
	if (ret_val != NO_ERROR)
	{
		kest_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"cutoff\" for filter \"%s\"", res->name);
		return ERR_BAD_ARGS;
	}
	
	switch (entry.type)
	{
		case DICT_ENTRY_TYPE_INT:
			cutoff = kest_expr_new_const(entry.value.val_int);
			if (!cutoff) return ERR_ALLOC_FAIL;
			break;
			
		case DICT_ENTRY_TYPE_FLOAT:
			cutoff = kest_expr_new_const(entry.value.val_float);
			if (!cutoff) return ERR_ALLOC_FAIL;
			break;
			
		case DICT_ENTRY_TYPE_EXPR:
			cutoff = entry.value.val_expr;
			if (!cutoff)
				return ERR_BAD_ARGS;
			break;
		
		default:
			kest_parser_error_at_node(ps, dict_node, "Filter \"%s\": Cutoff must be numerical", res->name);
			return ERR_BAD_ARGS;
	}
	
	ret_val = kest_dictionary_lookup_entry(dict, "Q", &entry);
	
	if (ret_val != NO_ERROR)
	{
		ret_val = kest_dictionary_lookup_entry(dict, "resonance", &entry);
	}
	
	if (ret_val != NO_ERROR)
	{
		Q = &kest_expression_root_2_over_2;
	}
	else
	{
		switch (entry.type)
		{
			case DICT_ENTRY_TYPE_INT:
				Q = kest_expr_new_const(entry.value.val_int);
				if (!Q) return ERR_ALLOC_FAIL;
				break;
				
			case DICT_ENTRY_TYPE_FLOAT:
				Q = kest_expr_new_const(entry.value.val_float);
				if (!Q) return ERR_ALLOC_FAIL;
				break;
				
			case DICT_ENTRY_TYPE_EXPR:
				Q = entry.value.val_expr;
				if (!Q)
					return ERR_BAD_ARGS;
				break;
			
			default:
				kest_parser_error_at_node(ps, dict_node, "Filter \"%s\": Cutoff must be numerical", res->name);
				return ERR_BAD_ARGS;
		}
	}
	
	KEST_PRINTF("cutoff  = %s\n", kest_expression_to_string(cutoff));
    KEST_PRINTF("Q       = %s\n", kest_expression_to_string(Q));
	
	kest_filter *filter = kest_filter_create(NULL);
	
	res->data = (void*)filter;
	
	filter->feed_forward = 3;
	filter->feed_back = 2;
	
	kest_expression *coefs[5];
	
	ret_val = kest_expr_create_hpf_coefficients(coefs, cutoff, Q);
	
	if (ret_val != NO_ERROR)
	{
		res->data = NULL;
		kest_expression_ptr_list_destroy(&filter->coefs);
		return ret_val;
	}
	
	kest_expression_ptr_list_append(&filter->coefs, coefs[0]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[1]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[2]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[3]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[4]);
	
	KEST_PRINTF("Extracted (%d, %d) filter \"%s\", with coefficients\n", filter->feed_forward, filter->feed_back, res->name);
	
	for (int i = 0; i < filter->coefs.count; i++)
	{
		KEST_PRINTF("\t%s\n", kest_expression_to_string(filter->coefs.entries[i]));
	}
	
	
	KEST_PRINTF("kest_extract_hpf_from_dict done\n");
	return NO_ERROR;
}

int kest_extract_bpf_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_dsp_resource *res)
{
	KEST_PRINTF("kest_extract_bpf_from_dict\n");
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	kest_expression *center = NULL;
	kest_expression *Q = NULL;
	
	int ret_val;
	kest_dictionary_entry entry;
	ret_val = kest_dictionary_lookup_entry(dict, "center", &entry);
	
	if (ret_val != NO_ERROR)
	{
		kest_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"center\" for filter \"%s\"", res->name);
		return ERR_BAD_ARGS;
	}
	
	switch (entry.type)
	{
		case DICT_ENTRY_TYPE_INT:
			center = kest_expr_new_const(entry.value.val_int);
			if (!center) return ERR_ALLOC_FAIL;
			break;
			
		case DICT_ENTRY_TYPE_FLOAT:
			center = kest_expr_new_const(entry.value.val_float);
			if (!center) return ERR_ALLOC_FAIL;
			break;
			
		case DICT_ENTRY_TYPE_EXPR:
			center = entry.value.val_expr;
			if (!center)
				return ERR_BAD_ARGS;
			break;
		
		default:
			kest_parser_error_at_node(ps, dict_node, "Filter \"%s\": Cutoff must be numerical", res->name);
			return ERR_BAD_ARGS;
	}
	
	ret_val = kest_dictionary_lookup_entry(dict, "Q", &entry);
	
	if (ret_val != NO_ERROR)
	{
		ret_val = kest_dictionary_lookup_entry(dict, "resonance", &entry);
	}
	
	if (ret_val != NO_ERROR)
	{
		Q = &kest_expression_root_2_over_2;
	}
	else
	{
		switch (entry.type)
		{
			case DICT_ENTRY_TYPE_INT:
				Q = kest_expr_new_const(entry.value.val_int);
				if (!Q) return ERR_ALLOC_FAIL;
				break;
				
			case DICT_ENTRY_TYPE_FLOAT:
				Q = kest_expr_new_const(entry.value.val_float);
				if (!Q) return ERR_ALLOC_FAIL;
				break;
				
			case DICT_ENTRY_TYPE_EXPR:
				Q = entry.value.val_expr;
				if (!Q)
					return ERR_BAD_ARGS;
				break;
			
			default:
				kest_parser_error_at_node(ps, dict_node, "Filter \"%s\": Cutoff must be numerical", res->name);
				return ERR_BAD_ARGS;
		}
	}
	
	KEST_PRINTF("center  = %s\n", kest_expression_to_string(center));
    KEST_PRINTF("Q       = %s\n", kest_expression_to_string(Q));
	
	kest_filter *filter = kest_filter_create(NULL);
	
	res->data = (void*)filter;
	
	filter->feed_forward = 3;
	filter->feed_back = 2;
	
	kest_expression *coefs[5];
	
	ret_val = kest_expr_create_bpf_coefficients(coefs, center, Q);
	
	if (ret_val != NO_ERROR)
	{
		res->data = NULL;
		kest_expression_ptr_list_destroy(&filter->coefs);
		return ret_val;
	}
	
	kest_expression_ptr_list_append(&filter->coefs, coefs[0]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[1]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[2]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[3]);
	kest_expression_ptr_list_append(&filter->coefs, coefs[4]);
	
	KEST_PRINTF("Extracted (%d, %d) filter \"%s\", with coefficients\n", filter->feed_forward, filter->feed_back, res->name);
	
	for (int i = 0; i < filter->coefs.count; i++)
	{
		KEST_PRINTF("\t%s\n", kest_expression_to_string(filter->coefs.entries[i]));
	}
	
	
	KEST_PRINTF("kest_extract_bpf_from_dict done\n");
	return NO_ERROR;
}

int kest_extract_enum_setting_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_setting *setting)
{
	if (!dict || !setting)
		return ERR_NULL_PTR;
	
	int ret_val;
	setting->widget_type = SETTING_WIDGET_DROPDOWN;
	
	return NO_ERROR;
}

int kest_extract_bool_setting_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_setting *setting)
{
	if (!dict || !setting)
		return ERR_NULL_PTR;
	
	int ret_val;
	setting->widget_type = SETTING_WIDGET_SWITCH;
	
	const char *str;
	
	setting->value = 0;
	if ((ret_val = kest_dictionary_lookup_str(dict, "default", &str)) == NO_ERROR)
	{
		if (strcmp(str, "true") == 0)
		{
			setting->value = 1;
		}
		else if (strcmp(str, "false") != 0)
		{
			kest_parser_warn_at_node(ps, dict_node, "Unknown default value \"%s\" given to bool setting \"%s\". Defaulting to false", str, setting->name_internal);
		}
	}
	else
	{
		kest_parser_warn_at_node(ps, dict_node, "No default value given to bool setting \"%s\". Defaulting to false", setting->name_internal);
	}
	
	return NO_ERROR;
}

int kest_extract_int_setting_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_setting *setting)
{
	if (!dict || !setting)
		return ERR_NULL_PTR;
	
	int ret_val;
	setting->widget_type = SETTING_WIDGET_FIELD;
	
	kest_expression *expr;
	
	setting->value = 0;
	if ((ret_val = kest_dictionary_lookup_expr(dict, "default", &expr)) == NO_ERROR)
	{
		if (!kest_expression_is_constant(expr))
		{
			kest_parser_error_at_node(ps, dict_node, "Default value must be constant");
			return ERR_BAD_ARGS;
		}
		
		setting->value = (int)(roundf(kest_expression_evaluate(expr, NULL)));
	}
	else
	{
		kest_parser_warn_at_node(ps, dict_node, "Could not find mandatory attribute \"default\" for setting \"%s\"; defulating to 0.", setting->name_internal);
	}
	
	setting->min = 0;
	if ((ret_val = kest_dictionary_lookup_expr(dict, "min", &expr)) == NO_ERROR)
	{
		if (!kest_expression_is_constant(expr))
		{
			kest_parser_error_at_node(ps, dict_node, "Min must be constant");
			return ERR_BAD_ARGS;
		}
		
		setting->min = (int)(roundf(kest_expression_evaluate(expr, NULL)));
	}
	else
	{
		kest_parser_warn_at_node(ps, dict_node, "Could not find mandatory attribute \"min\" for setting \"%s\"; defulating to 0.", setting->name_internal);
	}
	
	setting->max = 0;
	if ((ret_val = kest_dictionary_lookup_expr(dict, "max", &expr)) == NO_ERROR)
	{
		if (!kest_expression_is_constant(expr))
		{
			kest_parser_error_at_node(ps, dict_node, "Max must be constant");
			return ERR_BAD_ARGS;
		}
		
		setting->max = (int)(roundf(kest_expression_evaluate(expr, NULL)));
	}
	else
	{
		kest_parser_warn_at_node(ps, dict_node, "Could not find mandatory attribute \"max\" for setting \"%s\"; defulating to 0.", setting->name_internal);
	}
	
	return NO_ERROR;
}

kest_setting *kest_extract_setting_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict)
{
	if (!dict)
		return NULL;
	
	int ret_val;
	char *str;
	float v;
	int i;
	kest_expression *expr;
	
	kest_setting *setting = kest_alloc(sizeof(kest_setting));
	if (!setting) return NULL;
	
	init_setting_str(setting);
	
	setting->name_internal = kest_strndup(dict->name, 128);
	
	if (!setting->name_internal)
		goto setting_extract_abort;
	
	if ((ret_val = kest_dictionary_lookup_str(dict, "name", (void*)&str)) == NO_ERROR)
	{
		setting->name = kest_strndup(str, 128);
		KEST_PRINTF("Obtained setting name; \"%s\"\n", setting->name);
	}
	else
	{
		kest_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"name\" for setting \"%s\"", setting->name_internal);
		goto setting_extract_abort;
	}
	
	setting->type = EFFECT_SETTING_ENUM;
	if ((ret_val = kest_dictionary_lookup_str(dict, "type", (void*)&str)) == NO_ERROR)
	{
		KEST_PRINTF("Obtained setting type; \"%s\"\n", str);
		
		if (strcmp(str, "enum") == 0)
		{
			setting->type = EFFECT_SETTING_ENUM;
		}
		else if (strcmp(str, "bool") == 0)
		{
			setting->type = EFFECT_SETTING_BOOL;
		}
		else if (strcmp(str, "int") == 0)
		{
			setting->type = EFFECT_SETTING_INT;
		}
		else
		{
			kest_parser_warn_at_node(ps, dict_node, "Unknown scale \"%s\" given to setting \"%s\". Defaulting to int.", str, setting->name_internal);
		}
	}
	else
	{
		kest_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"type\" for setting \"%s\"", setting->name_internal);
		goto setting_extract_abort;
	}
	
	switch (setting->type)
	{
		case EFFECT_SETTING_ENUM:
			ret_val = kest_extract_enum_setting_from_dict(ps, dict_node, dict, setting);
			
			if (ret_val != NO_ERROR)
				goto setting_extract_abort;
			break;
		
		case EFFECT_SETTING_BOOL:
			ret_val = kest_extract_bool_setting_from_dict(ps, dict_node, dict, setting);
			
			if (ret_val != NO_ERROR)
				goto setting_extract_abort;
			break;
		
		case EFFECT_SETTING_INT:
			ret_val = kest_extract_int_setting_from_dict(ps, dict_node, dict, setting);
			
			if (ret_val != NO_ERROR)
				goto setting_extract_abort;
			break;
	}
	
	if ((ret_val = kest_dictionary_lookup_str(dict, "units", (void*)&str)) == NO_ERROR)
		setting->units = kest_strndup(str, 128);
	
	setting->page = EFFECT_SETTING_PAGE_SETTINGS;
	
	if ((ret_val = kest_dictionary_lookup_str(dict, "page", (void*)&str)) == NO_ERROR)
	{
		KEST_PRINTF("Obtained setting page; \"%s\"\n", str);
		
		if (strcmp(str, "main") == 0)
			setting->page = EFFECT_SETTING_PAGE_MAIN;
		else if (strcmp(str, "settings") == 0)
			setting->page = EFFECT_SETTING_PAGE_SETTINGS;
		else
			kest_parser_warn_at_node(ps, dict_node, "Unknown page \"%s\" given for setting \"%s\". Defaulting to settings page.", str, setting->name_internal);
	}
	
	if ((ret_val = kest_dictionary_lookup_expr(dict, "group", &expr)) == NO_ERROR)
	{
		if (!kest_expression_is_constant(expr))
		{
			kest_parser_error_at_node(ps, dict_node, "Group value must be constant");
			goto setting_extract_abort;
		}
		
		setting->group = (int)(roundf(kest_expression_evaluate(expr, NULL)));
	}
	
	KEST_PRINTF("Extracted a setting;\n");
	KEST_PRINTF("\tname: \"%s\"\n", setting->name);
	KEST_PRINTF("\tname_internal: \"%s\"\n", setting->name_internal);
	KEST_PRINTF("\tpage: %s\n", (setting->page == EFFECT_SETTING_PAGE_MAIN) ? "main" : "settings");
	
	return setting;
	
setting_extract_abort:
	
	if (setting)
	{
		gut_setting(setting);
		kest_free(setting);
	}
	
	return NULL;
}

int kest_extract_delay_buffer_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_dsp_resource *res)
{
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	int ret_val;
	
	kest_expression *expr;
	
	ret_val = kest_dictionary_lookup_expr(dict, "size", &expr);
	
	if (ret_val == NO_ERROR)
	{
		res->size = expr;
	}
	else
	{
		res->size = NULL;
	}
	
	ret_val = kest_dictionary_lookup_expr(dict, "delay", &expr);
		
	if (ret_val == NO_ERROR)
	{
		res->delay = expr;
	}
	else
	{
		kest_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"delay\" for delay buffer \"%s\"", res->name);
		return ERR_BAD_ARGS;
	}

	KEST_PRINTF("Found delay \"%s\", size %p, delay %p\n", res->name, res->size, res->delay);
	
	return NO_ERROR;
}

int kest_extract_mem_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_dsp_resource *res)
{
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	int ret_val;
	
	int size = 1;
	
	kest_expression *expr;
	kest_mem_slot *mem = kest_mem_slot_create(NULL);
	
	if (!mem) return ERR_ALLOC_FAIL;
	
	kest_scope_add_mem(ps->scope, res->name, mem);
	
	res->data = mem;
	
	mem->read_enable = 1;
	mem->read.period_ms = 7;
	
	KEST_PRINTF("Extracting mem slot \"%s\"...\n", res->name);
	
	ret_val = kest_dictionary_lookup_expr(dict, "size", &expr);
	
	if (ret_val == NO_ERROR)
	{
		size = (int)(roundf(fabs(kest_expression_evaluate(expr, NULL))));
		
		if (size == 0) size = 1;
		
		res->mem_size = size;
	}
	else
	{
		res->mem_size = 1;
	}
	
	kest_dictionary_entry *entry = NULL;
	
	entry = kest_dictionary_get_entry(dict, "read_ms");
	
	if (entry)
	{
		if (!kest_dictionary_entry_is_constant_number(entry))
		{
			kest_parser_error_at_node(ps, dict_node, "Read period must be constant; \"%s\" (type \"%s\") is not\n",
				kest_expression_to_string(expr), kest_expression_type_to_str(expr->type));
			goto mem_extract_abort;
		}
		
		mem->read.period_ms = kest_const_num_dictionary_entry_evaluate(entry);
		mem->read_enable = 1;
	}
	
	
	KEST_PRINTF("Extracted a mem slot;\n");
	KEST_PRINTF("\tres->mem_size: \"%d\"\n", res->mem_size);
	KEST_PRINTF("\t((kest_mem_slot*)res->data)->read_enable: \"%d\"\n", ((kest_mem_slot*)res->data)->read_enable);
	KEST_PRINTF("\t((kest_mem_slot*)res->data)->read.period_ms: \"%d\"\n", ((kest_mem_slot*)res->data)->read.period_ms);
	
	return NO_ERROR;
	
mem_extract_abort:
	
	return ERR_BAD_ARGS;
}

int kest_extract_filter_coefs_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_dsp_resource *res)
{
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("Extracting coefficients for filter \"%s\"...\n", res->name);
	
	kest_filter *filter = (kest_filter*)res->data;
	
	if (!filter)
		return ERR_ALLOC_FAIL;
	
	kest_dictionary_entry_list *coefs = NULL;
	
	int ret_val = kest_dictionary_lookup_list(dict, "coefs", &coefs);
	
	if (ret_val != NO_ERROR)
	{
		kest_parser_error_at_node(ps, dict_node, "Could not find mandatory attribute \"coefs\" for filter \"%s\"", res->name);
		return ERR_BAD_ARGS;
	}
	
	KEST_PRINTF("Found list; pointer %p.\n", coefs);
	
	KEST_PRINTF("%d entries. begin traversal\n", coefs->count);
	
	kest_string *string;
	char *str;
	
	for (int i = 0; i < coefs->count; i++)
	{
		string = kest_dict_entry_to_string(&coefs->entries[i]);
		if (!string)
		{
			KEST_PRINTF("Alloc fail!!\n");
			return ERR_ALLOC_FAIL;
		}
		str = kest_string_to_native(string);
		if (!str)
		{
			KEST_PRINTF("Alloc fail 2!!\n");
			return ERR_ALLOC_FAIL;
		}
		KEST_PRINTF("Entry %d; %s\n", i, str);
		kest_free(str);
		kest_string_destroy(string);
		string = NULL;
		
		if (coefs->entries[i].type != DICT_ENTRY_TYPE_EXPR)
		{
			kest_parser_error_at_node(ps, dict_node, "Filter coefficients must be expressions, but coefficient %d of filter \"%s\" is a %s (%d).",
				i, res->name, kest_dict_entry_type_to_string_nice(coefs->entries[i].type), coefs->entries[i].type);
			return ERR_BAD_ARGS;
		}
		
		kest_expression_ptr_list_append(&filter->coefs, coefs->entries[i].value.val_expr);
	}
	
	return NO_ERROR;
}

int kest_extract_biquad_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_dsp_resource *res)
{
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
		return ERR_ALLOC_FAIL;
	
	res->data = (void*)filter;
	
	filter->feed_forward = 3;
	filter->feed_back = 2;
	
	int ret_val = kest_extract_filter_coefs_from_dict(ps, dict_node, dict, res);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	kest_expression_ptr_list *coefs = &filter->coefs;
	
	if (filter->feed_forward + filter->feed_back != coefs->count)
	{
		kest_parser_error_at_node(ps, dict_node, "Filter \"%s\" is declared to be a biquad, but supplies %d coefficients, not 5.", res->name, coefs->count);
		return ERR_BAD_ARGS;
	}
	
	KEST_PRINTF("Extracted (%d, %d) filter \"%s\", with coefficients\n", filter->feed_forward, filter->feed_back, res->name);
	
	for (int i = 0; i < coefs->count; i++)
	{
		KEST_PRINTF("\t%s\n", kest_expression_to_string(coefs->entries[i]));
	}
	
	return NO_ERROR;
}

int kest_extract_polynomial_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_dsp_resource *res)
{
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
		return ERR_ALLOC_FAIL;
	
	res->data = (void*)filter;
	
	int ret_val = kest_extract_filter_coefs_from_dict(ps, dict_node, dict, res);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	kest_expression *expr;
	
	ret_val = kest_dictionary_lookup_expr(dict, "feed_back", &expr);
	
	filter->feed_back = 0;
	filter->feed_forward = filter->coefs.count;
	
	KEST_PRINTF("Extracted %d-degree polynomial \"%s\", with coefficients\n", filter->feed_forward, filter->feed_back, res->name);
	
	kest_expression_ptr_list *coefs = &filter->coefs;
	
	for (int i = 0; i < coefs->count; i++)
	{
		KEST_PRINTF("\t%s\n", kest_expression_to_string(coefs->entries[i]));
	}
	
	return NO_ERROR;
}

int kest_extract_filter_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict, kest_dsp_resource *res)
{
	if (!dict || !res)
		return ERR_NULL_PTR;
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
		return ERR_ALLOC_FAIL;
	
	res->data = (void*)filter;
	
	int ret_val = kest_extract_filter_coefs_from_dict(ps, dict_node, dict, res);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	kest_expression *expr;
	
	ret_val = kest_dictionary_lookup_expr(dict, "feed_back", &expr);
	
	if (ret_val == NO_ERROR)
	{
		filter->feed_back = (int)(roundf(kest_expression_evaluate(expr, NULL)));
		
		if (filter->feed_back < 0)
		{
			kest_parser_error_at_node(ps, dict_node, "Filter \"%s\" has negative feed-back degree %d, which doesn't make sense.",
				res->name, filter->feed_back);
			return ERR_BAD_ARGS;
		}
		if (filter->feed_back > filter->coefs.count)
		{
			kest_parser_error_at_node(ps, dict_node, "Filter \"%s\" has feed-back degree %d, which is higher than its coefficient count %d!",
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
	
	KEST_PRINTF("Extracted (%d, %d) filter \"%s\", with coefficients\n", filter->feed_forward, filter->feed_back, res->name);
	
	kest_expression_ptr_list *coefs = &filter->coefs;
	
	for (int i = 0; i < coefs->count; i++)
	{
		KEST_PRINTF("\t%s\n", kest_expression_to_string(coefs->entries[i]));
	}
	
	return NO_ERROR;
}

kest_dsp_resource *kest_extract_resource_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict)
{
	if (!dict)
		return NULL;
	
	int ret_val;
	float delay_len;
	char *type_str = NULL;
	
	kest_dsp_resource *res = kest_alloc(sizeof(kest_dsp_resource));
	
	if (!res) return NULL;
	
	kest_init_dsp_resource(res);
	
	res->name = kest_strndup(dict->name, 128);
	
	if (!res->name)
		goto resource_extract_abort;
	
	ret_val = kest_dictionary_lookup_str(dict, "type", (void*)&type_str);
	
	if (ret_val != NO_ERROR || !type_str)
	{
		kest_parser_error_at_node(ps, dict_node, "Could not find attribute \"type\" for resource \"%s\"", dict->name);
		goto resource_extract_abort;
	}
	
	res->type = string_to_resource_type(type_str);
	
	if (res->type == KEST_DSP_RESOURCE_NOTHING)
	{
		kest_parser_error_at_node(ps, dict_node, "Resource type \"%s\" unrecognised", type_str);
		goto resource_extract_abort;
	}
	else if (res->type == KEST_DSP_RESOURCE_DELAY)
	{
		kest_extract_delay_buffer_from_dict(ps, dict_node, dict, res);
	}
	else if (res->type == KEST_DSP_RESOURCE_MEM)
	{
		kest_extract_mem_from_dict(ps, dict_node, dict, res);
	}
	else if (res->type == KEST_DSP_RESOURCE_FILTER)
	{
		if (strcmp(type_str, "biquad") == 0)
			kest_extract_biquad_from_dict(ps, dict_node, dict, res);
		else if (strcmp(type_str, "lpf") == 0)
			kest_extract_lpf_from_dict(ps, dict_node, dict, res);
		else if (strcmp(type_str, "hpf") == 0)
			kest_extract_hpf_from_dict(ps, dict_node, dict, res);
		else if (strcmp(type_str, "bpf") == 0)
			kest_extract_bpf_from_dict(ps, dict_node, dict, res);
		else if (strcmp(type_str, "polynomial") == 0)
			kest_extract_polynomial_from_dict(ps, dict_node, dict, res);
		else
			kest_extract_filter_from_dict(ps, dict_node, dict, res);
	}
	
	return res;
	
resource_extract_abort:
	if (res)
	{
		if (res->name)
			kest_free(res->name);
		
		kest_free(res);
	}
	
	return NULL;
}

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
		continue; \
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
		continue; \
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
			continue; \
		} \
		 \
		value = kest_expression_evaluate(expr, NULL); \
	} \
} while (0)

#define ASSERT_ATTR_CONST_INT() do { \
	if (entry->type != KEST_EFF_ENTRY_TYPE_EXPR) { \
		kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be a numerical expression", type, name, key); \
		ret_val = ERR_BAD_ARGS; \
		continue; \
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
			continue; \
		} \
		 \
		value = kest_expression_evaluate(expr, NULL); \
		 \
		if (fabsf(value - roundf(value)) > 1e-6) \
		{ \
			kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be an integer", type, name, key); \
			ret_val = ERR_BAD_ARGS; \
			continue; \
		} \
		\
		value = roundf(value); \
	} \
} while (0)


#define ASSERT_ATTR_STRING() do { \
	if (entry->type != KEST_EFF_ENTRY_TYPE_STR) { \
		kest_parser_error(ps, "%s \"%s\": attribute \"%s\" must be a string", type, name, key); \
		ret_val = ERR_BAD_ARGS; \
		continue; \
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
		continue; \
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
		continue; \
	} \
	else \
	{ \
		if (!entry->value.val_expr) { \
			KEST_PRINTF_FORCE("entry->value.val_dict = NULL. Bug ?\n"); \
			ret_val = ERR_BAD_ARGS; \
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
	
	kest_parameter *param = kest_alloc(sizeof(kest_parameter)); // TODO: move to parser arena
	
	if (!param)
		return NULL;
	
	init_parameter_str(param);
	param->name_internal = kest_strndup(name, 128);
	
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
	
	int ret_val = NO_ERROR;
	
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
					kest_driver_init_scope_entry(&driver, expr->val.ref_name);
					kest_driver_list_append(&ps->drivers, driver);
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
		kest_free(param->name_internal);
		
		if (param->name)
			kest_free(param->name);
		
		if (param->units)
			kest_free(param->units);
		
		kest_free(param);
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
	
	kest_setting *setting = kest_alloc(sizeof(kest_setting)); // TODO: move to parser arena
	
	if (!setting)
		return NULL;
	
	init_setting_str(setting);
	setting->name_internal = kest_strndup(name, 128);
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	const char *string = NULL;
	kest_expression *expr = NULL;
	kest_eff_entry_list *list = NULL;
	kest_eff_entry_dict *subdict = NULL;
	
	float value = 0.0f;
	
	int ret_val = NO_ERROR;
	
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
		}
		else if (strcmp(key, "default") == 0)
		{
			ASSERT_ATTR_CONST_EXPR();
			setting->value = value;
		}
		else if (strcmp(key, "min") == 0)
		{
			ASSERT_ATTR_CONST_EXPR();
			setting->min = (int)(roundf(kest_expression_evaluate(expr, NULL)));
		}
		else if (strcmp(key, "max") == 0)
		{
			ASSERT_ATTR_CONST_EXPR();
			setting->max = (int)(roundf(kest_expression_evaluate(expr, NULL)));
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
			
			// TODO: Implement lol
		}
		else
		{
			kest_parser_error(ps, "Setting \"%s\": unrecognised attribute \"%s\"", name, key);
			ret_val = ERR_BAD_ARGS;
			goto extract_finish;
		}
	}
	
extract_finish:
	
	if (ret_val != NO_ERROR)
	{
		kest_free(setting->name_internal);
		
		if (setting->name)
			kest_free(setting->name);
		
		kest_free(setting);
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
	
	const char *type_string = type_entry->value.val_string;
	
	if (!type_string)
	{
		KEST_PRINTF_FORCE("type_entry->value.val_string = NULL. Bug ?\n");
		return NULL;
	}
	
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
	
	kest_dsp_resource *resource = kest_alloc(sizeof(kest_dsp_resource)); // TODO: move to parser arena
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_MEM;
	resource->mem_size = 1;
	
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_free(resource);
		return NULL;
	}
	
	kest_mem_slot *mem = kest_mem_slot_create(NULL);
	
	if (!mem)
	{
		if (resource->name)
			kest_free(resource->name);
		
		kest_free(resource);
		
		return NULL;
	}
	
	resource->data = mem;
	
	mem->read_enable = 1;
	mem->read.period_ms = 10;
	
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
		if (resource->name)
			kest_free(resource->name);
		
		kest_free(resource);
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
	
	kest_dsp_resource *resource = kest_alloc(sizeof(kest_dsp_resource)); // TODO: move to parser arena
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_DELAY;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_free(resource);
		return NULL;
	}
	
	kest_delay *delay = kest_delay_create(NULL);
	
	if (!delay)
	{
		kest_free(resource->name);
		kest_free(resource);
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
			ASSERT_ATTR_CONST_EXPR();
			delay->delay = expr;
			delay->units = KEST_DELAY_UNITS_MS;
		}
		else if (strcmp(key, "delay_seconds") == 0)
		{
			ASSERT_ATTR_CONST_EXPR();
			delay->delay = expr;
			delay->units = KEST_DELAY_UNITS_SECONDS;
		}
		else if (strcmp(key, "delay_samples") == 0)
		{
			ASSERT_ATTR_CONST_INT();
			delay->delay = expr;
			delay->units = KEST_DELAY_UNITS_SAMPLES;
		}
		else if (strcmp(key, "size") == 0)
		{
			ASSERT_ATTR_CONST_EXPR();
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
		if (resource->name)
			kest_free(resource->name);
		
		kest_free(resource);
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
	
	kest_dsp_resource *resource = kest_alloc(sizeof(kest_dsp_resource)); // TODO: move to parser arena
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_FILTER;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_free(resource);
		return NULL;
	}
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
	{
		kest_free(resource->name);
		kest_free(resource);
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
		
		kest_free(resource);
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
			
			kest_free(resource);
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
	
	kest_dsp_resource *resource = kest_alloc(sizeof(kest_dsp_resource)); // TODO: move to parser arena
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_FILTER;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_free(resource);
		return NULL;
	}
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
	{
		kest_free(resource->name);
		kest_free(resource);
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
		
		kest_free(resource);
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
			kest_free(resource);
			
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
	
	kest_dsp_resource *resource = kest_alloc(sizeof(kest_dsp_resource)); // TODO: move to parser arena
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_FILTER;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_free(resource);
		return NULL;
	}
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
	{
		kest_free(resource->name);
		kest_free(resource);
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
		
		kest_free(resource);
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
			kest_free(resource);
			
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
	
	kest_dsp_resource *resource = kest_alloc(sizeof(kest_dsp_resource)); // TODO: move to parser arena
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	
	resource->type = KEST_DSP_RESOURCE_FILTER;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_free(resource);
		return NULL;
	}
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
	{
		kest_free(resource->name);
		kest_free(resource);
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
		
		kest_free(resource);
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
			kest_free(resource);
			
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
	
	kest_dsp_resource *resource = kest_alloc(sizeof(kest_dsp_resource)); // TODO: move to parser arena
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	resource->type = KEST_DSP_RESOURCE_LFO;
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_free(resource);
		return NULL;
	}
	
	kest_lfo *lfo = kest_lfo_create(NULL);
	
	if (!lfo)
	{
		kest_free(resource->name);
		kest_free(resource);
		return NULL;
	}
	
	if (center_amplitude)
		lfo->mode = KEST_LFO_MODE_CENTER_AMP;
	else
		lfo->mode = KEST_LFO_MODE_MIN_MAX;
	
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
		if (resource->name)
			kest_free(resource->name);
		
		kest_free(resource);
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
	
	kest_dsp_resource *resource = kest_alloc(sizeof(kest_dsp_resource)); // TODO: move to parser arena
	
	if (!resource)
		return NULL;
	
	kest_init_dsp_resource(resource);
	resource->name = kest_strndup(name, 128);
	
	if (!resource->name)
	{
		kest_free(resource);
		return NULL;
	}
	
	kest_filter *filter = kest_filter_create(NULL);
	
	if (!filter)
	{
		kest_free(resource->name);
		kest_free(resource);
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
		if (resource->name)
			kest_free(resource->name);
		
		kest_free(resource);
		resource = NULL;
	}
	else
	{
		filter->feed_forward = filter->coefs.count;
		filter->feed_back = 0;
	}
	
	return resource;
}

