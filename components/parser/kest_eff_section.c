#include "kest_int.h"

#define PRINTLINES_ALLOWED 0

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
	if (!list || !sect || !ps)
		return ERR_NULL_PTR;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)sect->data;
	
	if (!sec) return ERR_BAD_ARGS;
	
	kest_parameter *parameter = NULL;
	kest_eff_entry_dict *dict = &sec->dict_;
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		key = kest_eff_entry_dict_index_key(dict, i);
		
		if (!entry)
			continue;
		
		if (entry->type != KEST_EFF_ENTRY_TYPE_SUBDICT)
		{
			kest_parser_error_at_node(ps, sect, "Syntax error at entry \"%s\"", key);
		}
		
		parameter = kest_extract_parameter(ps, entry->value.val_dict, key);
		
		if (!parameter)
			return ERR_BAD_ARGS;
		
		if (parameter)
			kest_parameter_pll_safe_append(list, parameter);
	}
	
	return NO_ERROR;
}

int kest_settings_section_extract(kest_eff_parsing_state *ps, kest_setting_pll **list, kest_ast_node *sect)
{
	if (!list || !sect || !ps)
		return ERR_NULL_PTR;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)sect->data;
	
	if (!sec) return ERR_BAD_ARGS;
	
	kest_setting *setting = NULL;
	kest_eff_entry_dict *dict = &sec->dict_;
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		key = kest_eff_entry_dict_index_key(dict, i);
		
		if (!entry)
			continue;
		
		if (entry->type != KEST_EFF_ENTRY_TYPE_SUBDICT)
		{
			kest_parser_error_at_node(ps, sect, "Syntax error at entry \"%s\"", key);
		}
		
		setting = kest_extract_setting(ps, entry->value.val_dict, key);
		
		if (!setting)
			return ERR_BAD_ARGS;
		
		if (setting)
			kest_setting_pll_safe_append(list, setting);
	}
	
	return NO_ERROR;
}

int kest_resources_section_extract(kest_eff_parsing_state *ps, kest_dsp_resource_pll **list, kest_ast_node *sect)
{
	if (!list || !sect || !ps)
		return ERR_NULL_PTR;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)sect->data;
	
	if (!sec) return ERR_BAD_ARGS;
	
	kest_dsp_resource *res = NULL;
	kest_eff_entry_dict *dict = &sec->dict_;
	kest_eff_entry *entry = NULL;
	const char *key = NULL;
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		key = kest_eff_entry_dict_index_key(dict, i);
		
		if (!entry)
			continue;
		
		if (entry->type != KEST_EFF_ENTRY_TYPE_SUBDICT)
		{
			kest_parser_error_at_node(ps, sect, "Syntax error at entry \"%s\"", key);
		}
		
		res = kest_extract_resource(ps, entry->value.val_dict, key);
		
		if (!res)
			return ERR_BAD_ARGS;
		
		if (res)
			kest_dsp_resource_pll_safe_append(list, res);
	}
	
	return NO_ERROR;
}

int kest_defs_section_extract(kest_eff_parsing_state *ps, kest_scope *scope, struct kest_ast_node *sect)
{
	KEST_PRINTF("kest_defs_section_extract(ps = %p, scope = %p, sect = %p)\n", ps, scope, sect);
	if (!scope || !sect || !ps)
		return ERR_NULL_PTR;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)sect->data;
	
	KEST_PRINTF("sec = %p\n");
	
	if (!sec)
	{
		return ERR_BAD_ARGS;
	}
	
	kest_eff_entry_dict *dict = &sec->dict_;
	kest_eff_entry *entry = NULL;
	kest_named_expression *nexpr;
	
	size_t n = kest_eff_entry_dict_count(dict);
	
	if (!dict)
		return ERR_BAD_ARGS;
	
	KEST_PRINTF("dict exists and has %d entries.\n", n);
	
	int ret_val = NO_ERROR;
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_eff_entry_dict_index(dict, i);
		if (!entry)
			continue;
		
		KEST_PRINTF("Entry %d, name: \"%s\", type: %s\n", i,  entry->name, kest_eff_entry_type_to_string(entry->type));
		
		if (entry->type == DICT_ENTRY_TYPE_EXPR)
		{
			KEST_PRINTF("Expression. Adding to defs...\n");
			nexpr = kest_alloc(sizeof(kest_named_expression));
			
			if (!nexpr)
				return ERR_ALLOC_FAIL;
			
			nexpr->name = kest_strndup(entry->name, 64);
			nexpr->expr = entry->value.val_expr;
			
			ret_val = kest_named_expression_pll_safe_append(&ps->def_exprs, nexpr);
			
			if (ret_val != NO_ERROR)
			{
				KEST_PRINTF("Error adding to defs: %s\n", kest_error_code_to_string(ret_val));
				return ret_val;
			}
			
			ret_val = kest_scope_add_expr(ps->scope, entry->name, entry->value.val_expr);
		}
		else
		{
			kest_parser_error_at_line(ps, entry->line, ".DEFS section entry DEFS.%s is not an expression\n", entry->name);
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

int kest_parse_entry_section(kest_eff_parsing_state *ps, kest_ast_node *section)
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
	
	int ret_val = kest_eff_entry_dict_init(&sec->dict_);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	ret_val = kest_parse_eff_entries(ps, &sec->dict_);
	
	KEST_PRINTF("Parsed section. Result:\n");
	
	kest_eff_entry_dict_print(&sec->dict_);
	
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

kest_eff_entry *kest_eff_section_index(kest_ast_node *section, size_t i)
{
	if (!section)
		return NULL;
	
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)section->data;
	
	if (!sec)
		return NULL;
	
	return kest_eff_entry_dict_index(&sec->dict_, i);
}

kest_eff_entry *kest_eff_section_lookup(kest_ast_node *section, const char *key)
{
	if (!section)
		return NULL;
		
	kest_eff_desc_file_section *sec = (kest_eff_desc_file_section*)section->data;
	
	if (!sec)
		return NULL;
	
	return kest_eff_entry_dict_lookup(&sec->dict_, key);
}
