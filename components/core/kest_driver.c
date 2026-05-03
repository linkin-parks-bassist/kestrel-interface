#include "kest_int.h"

IMPLEMENT_LIST(kest_driver);

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "kest_driver.c";

int kest_driver_init(kest_driver *driver)
{
	if (!driver)
		return ERR_NULL_PTR;
	
	memset(driver, 0, sizeof(kest_driver));
	
	return NO_ERROR;
}

int kest_driver_init_scope_entry(kest_driver *driver, const char *key)
{
	if (!driver)
		return ERR_NULL_PTR;
	
	driver->type = KEST_DRIVER_SCOPE_ENTRY;
	
	kest_driver_scope_entry *sed = kest_alloc(sizeof(kest_driver_scope_entry));
	
	if(!driver->data)
		return ERR_ALLOC_FAIL;
	
	driver->data = sed;
	
	sed->key = key;
	sed->entry = NULL;
	sed->scope = NULL;
	
	return NO_ERROR;
}

int kest_driver_evaluate(kest_driver *driver, float *dest)
{
	KEST_PRINTF("kest_driver_evaluate(driver = %p, dest = %p)\n");
	if (!driver || !dest)
		return ERR_NULL_PTR;
	
	kest_driver_scope_entry *scope_entry_str = NULL;
	int ret_val = NO_ERROR;
	
	switch (driver->type)
	{
		case KEST_DRIVER_SCOPE_ENTRY:
			scope_entry_str = (kest_driver_scope_entry*)driver->data;
			
			if (!scope_entry_str)
			{
				KEST_PRINTF("scope_entry_str = NULL\n");
				return ERR_BAD_ARGS;
			}
			
			if (!scope_entry_str->entry) scope_entry_str->entry = kest_scope_lookup(scope_entry_str->scope, scope_entry_str->key);
			
			ret_val = kest_scope_entry_eval(scope_entry_str->entry, scope_entry_str->scope, dest);
			
			KEST_PRINTF("Scope entry driver eval ran giving error code %s, *dest = %.06f\n", kest_error_code_to_string(ret_val), *dest);
			break;
			
		default:
			return ERR_BAD_ARGS;
	}
	
	KEST_PRINTF("Result: %f\n", *dest);
	return ret_val;
}


int kest_driver_clone(kest_driver *dest, kest_driver *src)
{
	if (!dest || !src)
		return ERR_NULL_PTR;
	
	memcpy(dest, src, sizeof(kest_driver));
	
	dest->data = NULL;
	
	kest_driver_scope_entry *sed = NULL;
	
	switch (dest->type)
	{
		case KEST_DRIVER_SCOPE_ENTRY:
			sed = kest_alloc(sizeof(kest_driver_scope_entry));
			
			if (!sed)
				return ERR_ALLOC_FAIL;
			
			dest->data = sed;
			
			sed->key = ((kest_driver_scope_entry*)src->data)->key;
			sed->scope = NULL;
			sed->entry = NULL;
			break;
	}
	
	return NO_ERROR;
}

kest_driver *kest_driver_make_clone(kest_driver *src)
{
	if (!src) return NULL;
	
	kest_driver *result = kest_alloc(sizeof(kest_driver));
	
	if (!result) return NULL;
	
	kest_driver_clone(result, src);
	
	return result;
}

int kest_driver_clone_for(kest_driver *dest, kest_driver *src, kest_effect *effect)
{
	KEST_PRINTF("kest_driver_clone_for(dest = %p, src = %p, effect = %p)\n",
		dest, src, effect);
	
	if (!dest || !src || !effect)
		return ERR_NULL_PTR;
	
	int ret_val = kest_driver_clone(dest, src);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	kest_driver_scope_entry *sed = NULL;
	
	switch (dest->type)
	{
		case KEST_DRIVER_SCOPE_ENTRY:
			KEST_PRINTF("It is a scope entry\n");
			
			sed = dest->data;
			
			if (!sed)
			{
				return ERR_UNKNOWN_ERR;
			}
			
			sed->scope = effect->scope;
			sed->entry = kest_scope_lookup(effect->scope, ((kest_driver_scope_entry*)src->data)->key);
			
			KEST_PRINTF("sed->key = %p = \"%s\"\n", sed->key, sed->key ? sed->key : "(nil)");
			KEST_PRINTF("sed->scope = %p\n", sed->scope);
			KEST_PRINTF("sed->entry = %p\n", sed->entry);
			break;
	}
	
	return NO_ERROR;
}
