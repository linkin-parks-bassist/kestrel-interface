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
