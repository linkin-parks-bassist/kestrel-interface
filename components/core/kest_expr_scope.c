#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_expr_scope.c";

IMPLEMENT_LINKED_PTR_LIST(kest_expr_scope_entry);

kest_expr_scope_entry *kest_new_expr_scope_entry_expr(const char *name, struct kest_expression *expr)
{
	if (!name || !expr)
		return NULL;
	
	kest_expr_scope_entry *result = kest_alloc(sizeof(kest_expr_scope_entry));
	
	if (!result)
		return NULL;
	
	result->type = KEST_SCOPE_ENTRY_TYPE_EXPR;
	result->name = name;
	result->val.expr = expr;
	result->updated = 0;
	
	return result;
}


kest_expr_scope_entry *kest_new_expr_scope_entry_param(kest_parameter *param)
{
	if (!param)
		return NULL;
	
	kest_expr_scope_entry *result = kest_alloc(sizeof(kest_expr_scope_entry));
	
	if (!result)
		return NULL;
	
	result->type = KEST_SCOPE_ENTRY_TYPE_PARAM;
	result->name = param->name_internal;
	result->val.param = param;
	result->updated = 0;
	
	return result;
}

kest_expr_scope_entry *kest_new_expr_scope_entry_setting(struct kest_setting *setting)
{
	if (!setting)
		return NULL;
	
	KEST_PRINTF("kest_new_expr_scope_entry_setting(setting = %p)\n", setting);
	kest_expr_scope_entry *result = kest_alloc(sizeof(kest_expr_scope_entry));
	
	if (!result)
		return NULL;
	
	result->type = KEST_SCOPE_ENTRY_TYPE_SETTING;
	result->name = setting->name_internal;
	result->val.setting = setting;
	result->updated = 0;
	
	KEST_PRINTF("\tresult->type = %d\n\tresult->name = \"%s\"\n\tresult->val.setting = %p\n",
		result->type, result->name, result->val.setting);
	
	return result;
}

int kest_expr_scope_init(kest_expr_scope *scope)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	scope->entries = NULL;
	
	return NO_ERROR;
}

kest_expr_scope *kest_new_expr_scope()
{
	kest_expr_scope *result = kest_alloc(sizeof(kest_expr_scope));
	
	if (!result)
		return NULL;
	
	result->entries = NULL;
	
	return result;
}

int kest_expr_scope_add_expr(kest_expr_scope *scope, const char *name, struct kest_expression *expr)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	if (!name || !expr)
		return ERR_BAD_ARGS;
	
	kest_expr_scope_entry *entry = kest_new_expr_scope_entry_expr(name, expr);
	
	if (!entry)
		return ERR_ALLOC_FAIL;
	
	int ret_val = kest_expr_scope_entry_pll_safe_append(&scope->entries, entry);
	
	return ret_val;
}

int kest_expr_scope_add_param(kest_expr_scope *scope, kest_parameter *param)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	if (!param)
		return ERR_BAD_ARGS;
	
	kest_expr_scope_entry *entry = kest_new_expr_scope_entry_param(param);
	
	if (!entry)
		return ERR_ALLOC_FAIL;
	
	int ret_val = kest_expr_scope_entry_pll_safe_append(&scope->entries, entry);
	
	return ret_val;
}

int kest_expr_scope_add_setting(kest_expr_scope *scope, kest_setting *setting)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	if (!setting)
		return ERR_BAD_ARGS;
	
	kest_expr_scope_entry *entry = kest_new_expr_scope_entry_setting(setting);
	
	if (!entry)
		return ERR_ALLOC_FAIL;
	
	int ret_val = kest_expr_scope_entry_pll_safe_append(&scope->entries, entry);
	
	return ret_val;
}

int kest_expr_scope_add_params(kest_expr_scope *scope, kest_parameter_pll *params)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	int ret_val;
	kest_parameter_pll *current = params;
	
	while (current)
	{
		if ((ret_val = kest_expr_scope_add_param(scope, current->data)) != NO_ERROR)
			return ret_val;
		
		current = current->next;
	}
	
	return NO_ERROR;
}

int kest_expr_scope_add_settings(kest_expr_scope *scope, kest_setting_pll *settings)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("kest_expr_scope_add_settings(scope = %p, settings = %p)\n", scope, settings);
	
	int ret_val;
	kest_setting_pll *current = settings;
	
	KEST_PRINTF("current = %p\n", current);
	while (current)
	{
		KEST_PRINTF("current->data = %p\n", current->data);
		KEST_PRINTF("Adding setting \"%s\"...\n", current->data ? current->data->name_internal : "(NULL)");
		if ((ret_val = kest_expr_scope_add_setting(scope, current->data)) != NO_ERROR)
		{
			
			return ret_val;
		}
		
		current = current->next;
		KEST_PRINTF("current = %p\n", current);
	}
	
	return NO_ERROR;
}

kest_expr_scope_entry *kest_expr_scope_fetch(kest_expr_scope *scope, const char *name)
{
	KEST_PRINTF("kest_expr_scope_fetch(scope = %p, name = %p = %s)\n", scope, name, name ? name : "(NULL)");
	if (!scope || !name)
		return NULL;
	
	kest_expr_scope_entry_pll *current = scope->entries;
	
	int i = 0;
	while (current)
	{
		KEST_PRINTF("Entry %d ", i);
		if (current->data)
		{
			KEST_PRINTF("exists ");
			if (current->data->name)
			{
				KEST_PRINTF("has name \"%s\"\n", current->data->name);
				if (strcmp(current->data->name, name) == 0)
				{
					return current->data;
				}
			}
			else
			{
				KEST_PRINTF("has no name. Moving on...");
			}
		}
		else
		{
			KEST_PRINTF("does not exist. Moving on...");
		}
		
		KEST_PRINTF("\n");
		current = current->next;
	}
	
	return NULL;
}

int kest_expr_scope_propagate_updates(kest_expr_scope *scope)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	
	
	return NO_ERROR;
}

/*
 * 
 * 
 * 
 * 
 * 
 *  Better version
 * 
 * 
 * 
 */
 
int kest_scope_entry_init_expr(kest_scope_entry *entry, kest_expression *expr)
{
	if (!entry)
		return ERR_NULL_PTR;
	
	if (!expr)
		return ERR_BAD_ARGS;
	
	entry->type = KEST_SCOPE_ENTRY_TYPE_EXPR;
	entry->val.expr = expr;
	entry->updated = 0;
	
	return NO_ERROR;
}

int kest_scope_entry_init_param(kest_scope_entry *entry, kest_parameter *param)
{
	if (!entry)
		return ERR_NULL_PTR;
		
	if (!param)
		return ERR_BAD_ARGS;
	
	entry->type = KEST_SCOPE_ENTRY_TYPE_PARAM;
	entry->val.param = param;
	entry->updated = 0;
	
	return NO_ERROR;
}

int kest_scope_entry_init_setting(kest_scope_entry *entry, kest_setting *setting)
{
	if (!entry)
		return ERR_NULL_PTR;
	
	if (!setting)
		return ERR_BAD_ARGS;
	
	entry->type = KEST_SCOPE_ENTRY_TYPE_SETTING;
	entry->val.setting = setting;
	entry->updated = 0;
	
	return NO_ERROR;
}

IMPLEMENT_DICT(kest_scope_entry);

kest_scope *kest_scope_new()
{
	kest_scope *scope = kest_alloc(sizeof(kest_scope));
	
	if (!scope)
		return NULL;
	
	int ret_val = kest_scope_init(scope);
	
	if (ret_val != NO_ERROR)
	{
		kest_free(scope);
		return NULL;
	}
	
	return scope;
}

int kest_scope_init(kest_scope *scope)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	return kest_scope_entry_dict_init(&scope->dict, 8);
}

int kest_scope_add_expr(kest_scope *scope, const char *name, struct kest_expression *expr)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	kest_scope_entry entry;
	
	int ret_val = kest_scope_entry_init_expr(&entry, expr);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	return kest_scope_entry_dict_insert(&scope->dict, name, entry);
}
int kest_scope_add_param(kest_scope *scope, kest_parameter *param)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	kest_scope_entry entry;
	
	int ret_val = kest_scope_entry_init_param(&entry, param);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	return kest_scope_entry_dict_insert(&scope->dict, param->name_internal, entry);
}

int kest_scope_add_setting(kest_scope *scope, kest_setting *setting)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	kest_scope_entry entry;
	
	int ret_val = kest_scope_entry_init_setting(&entry, setting);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	return kest_scope_entry_dict_insert(&scope->dict, setting->name_internal, entry);
}

int kest_scope_add_params(kest_scope *scope, kest_parameter_pll *params)
{
	if (!scope || !params)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	kest_parameter_pll *current = params;
	
	while (current)
	{
		ret_val = kest_scope_add_param(scope, current->data);
		
		if (ret_val != NO_ERROR)
			return ret_val;
		
		current = current->next;
	}
	
	return ret_val;
}

int kest_scope_add_settings(kest_scope *scope, kest_setting_pll *settings)
{
	if (!scope || !settings)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	kest_setting_pll *current = settings;
	
	while (current)
	{
		ret_val = kest_scope_add_setting(scope, current->data);
		
		if (ret_val != NO_ERROR)
			return ret_val;
		
		current = current->next;
	}
	
	return ret_val;
}

kest_scope_entry *kest_scope_fetch(kest_scope *scope, const char *name)
{
	return kest_scope_entry_dict_lookup(&scope->dict, name);
}

kest_scope_entry *kest_scope_lookup(kest_scope *scope, const char *name)
{
	return kest_scope_entry_dict_lookup(&scope->dict, name);
}

size_t kest_scope_count(kest_scope *scope)
{
	if (!scope)
		return 0;
	
	return kest_scope_entry_dict_count(&scope->dict);
}

kest_scope_entry *kest_scope_index(kest_scope *scope, size_t n)
{
	if (!scope)
		return NULL;
		
	return kest_scope_entry_dict_index(&scope->dict, n);
}

int kest_scope_propagate_updates(kest_scope *scope)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	kest_scope_entry *entry = NULL;
	kest_scope_entry *ref_entry = NULL;
	size_t n = kest_scope_count(scope);
	int ret_val;
	
	string_list names;
	
	if (!n)
		return NO_ERROR;
	
	char_ptr_list_init(&names);
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_scope_index(scope, i);
		
		if (!entry)
			continue;
		
		if (entry->type == KEST_SCOPE_ENTRY_TYPE_PARAM)
		{
			entry->updated = entry->val.param ? entry->val.param->updated : 0;
		}
		else if (entry->type == KEST_SCOPE_ENTRY_TYPE_SETTING)
		{
			entry->updated = entry->val.setting ? entry->val.setting->updated : 0;
		}
	}
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_scope_index(scope, i);
		
		if (entry && entry->type == KEST_SCOPE_ENTRY_TYPE_EXPR)
		{
			ret_val = kest_expression_get_references(entry->val.expr, &names);
			
			if (ret_val != NO_ERROR)
			{
				char_ptr_list_destroy(&names);
				return ret_val;
			}
			
			for (int j = 0; j < names.count; j++)
			{
				ref_entry = kest_scope_lookup(scope, names.entries[j]);
				
				if (ref_entry && ref_entry->updated)
				{
					entry->updated = 1;
					break;
				}
			}
			
			char_ptr_list_drain(&names);
		}
	}
	
	char_ptr_list_destroy(&names);
	
	return NO_ERROR;
}


int kest_scope_clear_updates(kest_scope *scope)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	size_t n = kest_scope_count(scope);
	
	if (!n)
		return NO_ERROR;
	
	kest_scope_entry *entry = NULL;
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_scope_index(scope, i);
		
		if (entry) entry->updated = 0;
		else continue;
		
		if (entry->type == KEST_SCOPE_ENTRY_TYPE_PARAM)
		{
			if (entry->val.param)
				entry->val.param->updated = 0;
		}
		
		if (entry->type == KEST_SCOPE_ENTRY_TYPE_SETTING)
		{
			if (entry->val.setting)
				entry->val.setting->updated = 0;
		}
	}
	
	return NO_ERROR;
}
