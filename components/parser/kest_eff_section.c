#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 1
#endif

static const char *FNAME = "kest_eff_section.c";

int token_is_valid_section_name(char *str)
{
	if (!str)
		return 0;
	
	if (strcmp(str, "INFO") 	  == 0) return 1;
	if (strcmp(str, "RESOURCES")  == 0) return 1;
	if (strcmp(str, "PARAMETERS") == 0) return 1;
	if (strcmp(str, "SETTINGS")   == 0) return 1;
	if (strcmp(str, "DEFS")   	  == 0) return 1;
	if (strcmp(str, "CODE") 	  == 0) return 1;
	
	return 0;
}

int get_section_start_score(char *str, int current_score)
{
	if (!str)
		return 0;
	
	if (token_is_char(str, '\n')) return 1;
	
	switch (current_score)
	{
		case 0: return 0;
		case 1: return (token_is_char(str, '.')) 		  ? 2 : 0;
		case 2: return (token_is_valid_section_name(str)) ? 3 : 0;
	}
	
	return 0;
}

int kest_parameters_section_extract(kest_eff_parsing_state *ps, kest_parameter_pll **list, kest_ast_node *sect)
{
	if (!list || !sect | !ps)
		return ERR_NULL_PTR;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)sect->data;
	
	if (!sec) return ERR_BAD_ARGS;
	
	kest_dictionary *dict = sec->dict;
	kest_parameter *param = NULL;
	
	if (!dict)
		return ERR_BAD_ARGS;
	
	for (int i = 0; i < dict->n_entries; i++)
	{
		if (dict->entries[i].type == DICT_ENTRY_TYPE_SUBDICT)
		{
			param = kest_extract_parameter_from_dict(ps, sect, dict->entries[i].value.val_dict);
			
			if (param)
				kest_parameter_pll_safe_append(list, param);
		}
	}
	
	return NO_ERROR;
}

int kest_settings_section_extract(kest_eff_parsing_state *ps, kest_setting_pll **list, kest_ast_node *sect)
{
	if (!list || !sect | !ps)
		return ERR_NULL_PTR;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)sect->data;
	
	if (!sec) return ERR_BAD_ARGS;
	
	kest_dictionary *dict = sec->dict;
	kest_setting *setting = NULL;
	
	if (!dict)
		return ERR_BAD_ARGS;
	
	for (int i = 0; i < dict->n_entries; i++)
	{
		if (dict->entries[i].type == DICT_ENTRY_TYPE_SUBDICT)
		{
			setting = kest_extract_setting_from_dict(ps, sect, dict->entries[i].value.val_dict);
			
			if (setting)
			{
				KEST_PRINTF("Obtained setting \"%s\"; adding to list...\n", setting->name_internal);
				kest_setting_pll_safe_append(list, setting);
			}
		}
	}
	
	return NO_ERROR;
}

int kest_resources_section_extract(kest_eff_parsing_state *ps, kest_dsp_resource_pll **list, kest_ast_node *sect)
{
	if (!list || !sect || !ps)
		return ERR_NULL_PTR;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)sect->data;
	
	if (!sec) return ERR_BAD_ARGS;
	
	kest_dictionary *dict = sec->dict;
	kest_dsp_resource *res= NULL;
	
	if (!dict)
		return ERR_BAD_ARGS;
	
	for (int i = 0; i < dict->n_entries; i++)
	{
		if (dict->entries[i].type == DICT_ENTRY_TYPE_SUBDICT)
		{
			res = kest_extract_resource_from_dict(ps, sect, dict->entries[i].value.val_dict);
			
			if (!res)
			{
				kest_parser_error_at_node(ps, sect, "Failed to interpret resource \"%s\"", dict->entries[i].name);
				return ERR_BAD_ARGS;
			}
			
			if (res)
				kest_dsp_resource_pll_safe_append(list, res);
		}
	}
	
	return NO_ERROR;
}

int kest_defs_section_extract(kest_eff_parsing_state *ps, kest_expr_scope *scope, struct kest_ast_node *sect)
{
	KEST_PRINTF("kest_defs_section_extract(ps = %p, scope = %p, sect = %p)\n", ps, scope, sect);
	if (!scope || !sect || !ps)
		return ERR_NULL_PTR;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)sect->data;
	
	if (!sec) return ERR_BAD_ARGS;
	
	kest_dictionary *dict = sec->dict;
	kest_named_expression *nexpr;
	
	if (!dict)
		return ERR_BAD_ARGS;
	
	KEST_PRINTF("dict exists and has %d entries.\n", dict->n_entries);
	
	int ret_val = NO_ERROR;
	for (int i = 0; i < dict->n_entries; i++)
	{
		KEST_PRINTF("Entry %d, name: \"%s\", type: %s\n", i,  dict->entries[i].name, kest_dict_entry_type_to_string(dict->entries[i].type));
		if (dict->entries[i].type == DICT_ENTRY_TYPE_EXPR)
		{
			KEST_PRINTF("Expression. Adding to defs...\n");
			nexpr = kest_alloc(sizeof(kest_named_expression));
			
			if (!nexpr)
				return ERR_ALLOC_FAIL;
			
			nexpr->name = kest_strndup(dict->entries[i].name, 64);
			nexpr->expr = dict->entries[i].value.val_expr;
			
			ret_val = kest_named_expression_pll_safe_append(&ps->def_exprs, nexpr);
			
			if (ret_val != NO_ERROR)
			{
				KEST_PRINTF("Error adding to defs: %s\n", kest_error_code_to_string(ret_val));
				return ret_val;
			}
			
			ret_val = kest_expr_scope_add_expr(ps->scope, dict->entries[i].name, dict->entries[i].value.val_expr);
		}
		else
		{
			kest_parser_error_at_line(ps, dict->entries[i].line, ".DEFS section entry DEFS.%s is not an expression\n", dict->entries[i].name);
		}
	}
	
	return NO_ERROR;
}

int kest_dictionary_section_lookup_str(kest_ast_node *section, const char *name, const char **result)
{
	if (!section) return ERR_NULL_PTR;
	if (section->type != KEST_AST_NODE_SECTION) return ERR_BAD_ARGS;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)section->data;
	
	if (!sec || !sec->dict) return ERR_BAD_ARGS;
	
	return kest_dictionary_lookup_str(sec->dict, name, result);
}

int kest_dictionary_section_lookup_float(kest_ast_node *section, const char *name, float *result)
{
	if (!section) return ERR_NULL_PTR;
	if (section->type != KEST_AST_NODE_SECTION) return ERR_BAD_ARGS;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)section->data;
	
	if (!sec || !sec->dict) return ERR_BAD_ARGS;
	
	return kest_dictionary_lookup_float(sec->dict, name, result);
}

int kest_dictionary_section_lookup_int(kest_ast_node *section, const char *name, int *result)
{
	if (!section) return ERR_NULL_PTR;
	if (section->type != KEST_AST_NODE_SECTION) return ERR_BAD_ARGS;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)section->data;
	
	if (!sec || !sec->dict) return ERR_BAD_ARGS;
	
	return kest_dictionary_lookup_int(sec->dict, name, result);
}

int kest_dictionary_section_lookup_expr(kest_ast_node *section, const char *name, kest_expression **result)
{
	if (!section) return ERR_NULL_PTR;
	if (section->type != KEST_AST_NODE_SECTION) return ERR_BAD_ARGS;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)section->data;
	
	if (!sec || !sec->dict) return ERR_BAD_ARGS;
	
	return kest_dictionary_lookup_expr(sec->dict, name, result);
}

int kest_dictionary_section_lookup_dict(kest_ast_node *section, const char *name, kest_dictionary **result)
{
	if (!section) return ERR_NULL_PTR;
	if (section->type != KEST_AST_NODE_SECTION) return ERR_BAD_ARGS;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)section->data;
	
	if (!sec || !sec->dict) return ERR_BAD_ARGS;
	
	return kest_dictionary_lookup_dict(sec->dict, name, result);
}

int kest_parse_dictionary_section(kest_eff_parsing_state *ps, kest_ast_node *section)
{
	if (!ps || !section)
		return ERR_NULL_PTR;
	
	if (section->type != KEST_AST_NODE_SECTION)
		return ERR_BAD_ARGS;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)section->data;
	
	kest_token_ll *tokens = sec->tokens;
	
	if (!tokens)
		return ERR_BAD_ARGS;
	
	ps->current_token = tokens->next;
	
	int ret_val = kest_parse_dictionary(ps, &sec->dict, sec->name);
	
	KEST_PRINTF("Parsed dictionary section. Result:\n");
	
	print_dict(sec->dict);
	
	return ret_val;
}


int kest_parse_code_section(kest_eff_parsing_state *ps, kest_ast_node *section)
{
	if (!ps || !section)
		return ERR_NULL_PTR;
	
	if (section->type != KEST_AST_NODE_SECTION)
		return ERR_BAD_ARGS;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)section->data;
	kest_token_ll *tokens = sec->tokens;
	
	if (!tokens)
		return ERR_BAD_ARGS;
	
	ps->current_token = tokens->next;
	
	int ret_val = kest_parse_asm(ps);
	
	return ret_val;
}
