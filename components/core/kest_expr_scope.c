#include "kest_int.h"

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "kest_expr_scope.c";
 
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

int kest_scope_entry_init_mem(kest_scope_entry *entry, kest_mem_slot *mem)
{
	if (!entry)
		return ERR_NULL_PTR;
	
	if (!mem)
		return ERR_BAD_ARGS;
	
	entry->type = KEST_SCOPE_ENTRY_TYPE_MEM;
	entry->val.mem = mem;
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
	
	scope->count = 0;
	
	return kest_scope_entry_dict_init(&scope->dict, 8);
}

int kest_scope_add_entry(kest_scope *scope, const char *name, kest_scope_entry entry)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	int ret_val = kest_scope_entry_dict_insert(&scope->dict, name, entry);
	
	if (ret_val == NO_ERROR)
		scope->count++;
	
	return ret_val;
}

int kest_scope_add_mem(kest_scope *scope, const char *name, kest_mem_slot *mem)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	kest_scope_entry entry;
	
	int ret_val = kest_scope_entry_init_mem(&entry, mem);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	return kest_scope_add_entry(scope, name, entry);
}

int kest_scope_add_expr(kest_scope *scope, const char *name, struct kest_expression *expr)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	kest_scope_entry entry;
	
	int ret_val = kest_scope_entry_init_expr(&entry, expr);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	return kest_scope_add_entry(scope, name, entry);
}

int kest_scope_add_param(kest_scope *scope, kest_parameter *param)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	kest_scope_entry entry;
	
	int ret_val = kest_scope_entry_init_param(&entry, param);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	return kest_scope_add_entry(scope, param->name_internal, entry);
}

int kest_scope_add_setting(kest_scope *scope, kest_setting *setting)
{
	if (!scope)
		return ERR_NULL_PTR;
	
	kest_scope_entry entry;
	
	int ret_val = kest_scope_entry_init_setting(&entry, setting);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	return kest_scope_add_entry(scope, setting->name_internal, entry);
}

kest_scope_entry *kest_scope_add_entry_return_ptr(kest_scope *scope, const char *name, kest_scope_entry entry)
{
	if (!scope) return NULL;
	
	scope->count++;
	
	return kest_scope_entry_dict_insert_return_ptr(&scope->dict, name, entry);
}

kest_scope_entry *kest_scope_add_mem_return_entry(kest_scope *scope, const char *name, kest_mem_slot *mem)
{
	if (!scope)
		return NULL;
	
	kest_scope_entry entry;
	
	int ret_val = kest_scope_entry_init_mem(&entry, mem);
	
	if (ret_val != NO_ERROR)
		return NULL;
	
	return kest_scope_add_entry_return_ptr(scope, name, entry);
}

kest_scope_entry *kest_scope_add_expr_return_entry(kest_scope *scope, const char *name, kest_expression *expr)
{
	if (!scope)
		return NULL;
	
	kest_scope_entry entry;
	
	int ret_val = kest_scope_entry_init_expr(&entry, expr);
	
	if (ret_val != NO_ERROR)
		return NULL;
	
	return kest_scope_add_entry_return_ptr(scope, name, entry);
}

kest_scope_entry *kest_scope_add_param_return_entry(kest_scope *scope, kest_parameter *param)
{
	if (!scope)
		return NULL;
	
	kest_scope_entry entry;
	
	int ret_val = kest_scope_entry_init_param(&entry, param);
	
	if (ret_val != NO_ERROR)
		return NULL;
	
	return kest_scope_add_entry_return_ptr(scope, param->name_internal, entry);
}

kest_scope_entry *kest_scope_add_setting_return_entry(kest_scope *scope, kest_setting *setting)
{
	if (!scope)
		return NULL;
	
	kest_scope_entry entry;
	
	int ret_val = kest_scope_entry_init_setting(&entry, setting);
	
	if (ret_val != NO_ERROR)
		return NULL;
	
	return kest_scope_add_entry_return_ptr(scope, setting->name_internal, entry);
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
	
	return scope->count;
	return kest_scope_entry_dict_count(&scope->dict);
}

kest_scope_entry *kest_scope_index(kest_scope *scope, size_t n)
{
	if (!scope)
		return NULL;
		
	return kest_scope_entry_dict_index(&scope->dict, n);
}

const char *kest_scope_index_key(kest_scope *scope, size_t n)
{
	if (!scope)
		return NULL;
	
	return kest_scope_entry_dict_index_key(&scope->dict, n);
}

int kest_scope_entry_propagate_updates(kest_scope_entry *entry)
{
	if (!entry)
		return 0;
	
	
	
	return 0;
}

int kest_scope_detect_env_updates(kest_scope *scope)
{
	KEST_PRINTF("kest_scope_detect_env_updates(scope = %p)\n", scope);
	
	if (!scope)
		return ERR_NULL_PTR;
	
	int any_updates = 0;
	
	size_t n = scope->count;
	kest_scope_entry *entry = NULL;
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_scope_index(scope, i);
		
		if (!entry)
			continue;
		
		KEST_PRINTF("entry[%d]: \"%s\"\n", i, kest_scope_index_key(scope, i));
		
		switch (entry->type)
		{
			case KEST_SCOPE_ENTRY_TYPE_PARAM:
				KEST_PRINTF("Type: parameter\n");
				
				kest_parameter_if_driven_refresh(entry->val.param);
				entry->updated = entry->val.param ? entry->val.param->updated : 0;
				
				KEST_PRINTF("entry->updated: %d\n", entry->updated);
				break;
				
			case KEST_SCOPE_ENTRY_TYPE_SETTING:
				KEST_PRINTF("Type: setting\n");
				entry->updated = entry->val.setting ? entry->val.setting->updated : 0;
				KEST_PRINTF("entry->updated: %d\n", entry->updated);
				break;
				
			case KEST_SCOPE_ENTRY_TYPE_MEM:
				KEST_PRINTF("Type: mem\n");
				KEST_PRINTF("entry->updated: %d\n", entry->updated);
				break;
				
			default:
				break;
		}
		
		any_updates |= entry->updated;
	}
	
	KEST_PRINTF("kest_scope_detect_env_updates(scope = %p) done\n", scope);
	return any_updates;
}

int kest_scope_propagate_updates(kest_scope *scope)
{
	KEST_PRINTF("kest_scope_propagate_updates(scope = %p)\n", scope);
	
	if (!scope)
		return ERR_NULL_PTR;
	
	kest_scope_entry *entry = NULL;
	kest_scope_entry *ref_entry = NULL;
	size_t n = scope->count;
	const char *current_key;
	int ret_val;
	
	KEST_PRINTF("scope->count = %d\n", n);
	KEST_PRINTF("kest_scope_entry_dict_count(&scope->dict) = %d\n", kest_scope_entry_dict_count(&scope->dict));
	
	int any_updates = 0;
	
	string_list names;
	
	if (!n) return 0;
	
	char_ptr_list_init(&names);
	
	any_updates |= kest_scope_detect_env_updates(scope);
	
	for (size_t i = 0; i < n; i++)
	{
		entry = kest_scope_index(scope, i);
		current_key = kest_scope_index_key(scope, i);
		
		if (!entry)
			continue;
		
		KEST_PRINTF("entry[%d]: \"%s\". entry->updated: %d\n", i, current_key, entry->updated);
		
		if (entry->type == KEST_SCOPE_ENTRY_TYPE_EXPR)
		{
			KEST_PRINTF("... Which is an expression. Specifically, \"%s\".\n", kest_expression_to_string(entry->val.expr));
			
			ret_val = kest_expression_get_references(entry->val.expr, &names);
			
			if (ret_val != NO_ERROR)
			{
				KEST_PRINTF("Hmm, I tried to list out its expressions, but something went wrong: %s\n", kest_error_code_to_string(ret_val));
				char_ptr_list_drain(&names);
				break;
			}
			
			KEST_PRINTF("It refers to: %s\n", (names.count == 0) ? "nothing" : "");
			
			for (int j = 0; j < names.count; j++)
			{
				KEST_PRINTF("\t\"%s\", ", names.entries[j]);
				ref_entry = kest_scope_lookup(scope, names.entries[j]);
				
				if (!ref_entry)
				{
					KEST_PRINTF("which is not found in scope !\n");
					continue;
				}
				
				if (ref_entry->updated)
				{
					KEST_PRINTF("which is updated! Therefore ya boy %s is too.\n", current_key);
					entry->updated = 1;
					any_updates = 1;
					break;
				}
				else
				{
					KEST_PRINTF("which is not updated. Oh well.\n");
				}
			}
			
			char_ptr_list_drain(&names);
		}
		
		any_updates |= entry->updated;
	}
	
	char_ptr_list_destroy(&names);
	
	return any_updates;
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

int kest_scope_entry_eval(kest_scope_entry *entry, kest_scope *scope, float *dest)
{
	KEST_PRINTF("kest_scope_entry_eval(entry = %p, scope = %p, dest = %p)\n",
		entry, scope, dest);
	if (!entry || !dest)
		return ERR_NULL_PTR;
	
	kest_parameter *param = NULL;
	kest_setting *setting = NULL;
	kest_mem_slot *mem_slot = NULL;
	
	switch (entry->type)
	{
		case KEST_SCOPE_ENTRY_TYPE_EXPR:
			*dest = kest_expression_evaluate(entry->val.expr, scope);
			break;
			
		case KEST_SCOPE_ENTRY_TYPE_PARAM:
			param = entry->val.param;
			if (!param)
				return ERR_BAD_ARGS;
			*dest = kest_parameter_evaluate(param);
			break;
		
		case KEST_SCOPE_ENTRY_TYPE_SETTING:
			setting = entry->val.setting;
			if (!setting)
				return ERR_BAD_ARGS;
			*dest = (float)setting->value;
			break;
		
		case KEST_SCOPE_ENTRY_TYPE_MEM:
			mem_slot = entry->val.mem;
			
			if (!mem_slot)
				return ERR_BAD_ARGS;
			
			*dest = kest_fpga_sample_to_float(mem_slot->value);
			
			KEST_PRINTF("Evaluating mem_slot scope entry. Value: %s%.05f\n", *dest > 0 ? " " : "", *dest);
			break;
		
		default: return ERR_BAD_ARGS;
	}
	
	return NO_ERROR;
}

int kest_scope_entry_eval_rec(kest_scope_entry *entry, kest_scope *scope, float *dest, int depth)
{
	KEST_PRINTF("kest_scope_entry_eval_rec(entry = %p, scope = %p, dest = %p, depth = %d)\n",
		entry, scope, dest, depth);
	
	if (!entry || !dest)
		return ERR_NULL_PTR;
	
	kest_parameter *param 	= NULL;
	kest_setting *setting 	= NULL;
	kest_mem_slot *mem_slot = NULL;
	
	switch (entry->type)
	{
		case KEST_SCOPE_ENTRY_TYPE_EXPR:
			*dest = kest_expression_evaluate_rec(entry->val.expr, scope, depth);
			break;
			
		case KEST_SCOPE_ENTRY_TYPE_PARAM:
			param = entry->val.param;
			if (!param)
				return ERR_BAD_ARGS;
			*dest = kest_parameter_evaluate(param);
			break;
		
		case KEST_SCOPE_ENTRY_TYPE_SETTING:
			setting = entry->val.setting;
			if (!setting)
				return ERR_BAD_ARGS;
			*dest = (float)setting->value;
			break;
		
		case KEST_SCOPE_ENTRY_TYPE_MEM:
			mem_slot = entry->val.mem;
			
			if (!mem_slot)
				return ERR_BAD_ARGS;
			
			*dest = kest_fpga_sample_to_float(mem_slot->value);
			
			KEST_PRINTF("Evaluating mem_slot scope entry. Value: %s%.05f\n", *dest > 0 ? " " : "", *dest);
			break;
		
		default: return ERR_BAD_ARGS;
	}
	
	return NO_ERROR;
}
