#include <float.h>
#include "kest_int.h"

#define PRINTLINES_ALLOWED 1

#define DEFAULT_MAX_VELOCITY 0.2

IMPLEMENT_LINKED_PTR_LIST(kest_parameter);
IMPLEMENT_LINKED_PTR_LIST(kest_setting);

IMPLEMENT_PTR_LIST(kest_parameter);
IMPLEMENT_PTR_LIST(kest_setting);

IMPLEMENT_LIST(kest_parameter);
IMPLEMENT_LIST(kest_setting);

IMPLEMENT_POOL(kest_parameter);
kest_allocator kest_parameter_allocator;
kest_parameter_pool kest_parameter_mem_pool;
IMPLEMENT_POOL(kest_setting);
kest_allocator kest_setting_allocator;
kest_setting_pool kest_setting_mem_pool;

static const char *FNAME = "kest_parameter.c";

int init_parameter_str(kest_parameter *param)
{
	if (!param)
		return ERR_NULL_PTR;
	
	memset(param, 0, sizeof(kest_parameter));
	
	param->value = 0.0;
	param->min = 0.0;
	param->min_expr = NULL;
	param->max = 1.0;
	param->max_expr = NULL;
	param->max_velocity = DEFAULT_MAX_VELOCITY;
	param->id = (kest_parameter_id){.preset_id = 0, .effect_id = 0, .parameter_id = 0};
	param->name = NULL;
	param->name_internal = NULL;
	param->units = NULL;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->updated = 0;
	param->effect = NULL;
	
	param->driver_index = KEST_PARAMETER_UNDRIVEN;
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	param->effect_rep.representer = NULL;
	param->effect_rep.representee = param;
	param->effect_rep.update = kest_parameter_effect_rep_update;
	param->widget_rep.representer = NULL;
	param->widget_rep.representee = param;
	KEST_PRINTF("kest_parameter_widget_rep_update = %p\n", kest_parameter_widget_rep_update);
	param->widget_rep.update = kest_parameter_widget_rep_update;
	
	kest_representation_ptr_list_init(&param->reps);
	//kest_representation_ptr_list_append(&param->reps, &param->effect_rep);
	#endif
	
	return NO_ERROR;
}

int init_parameter(kest_parameter *param, const char *name, float level, float min, float max)
{
	if (!param) return ERR_NULL_PTR;
	
	memset(param, 0, sizeof(kest_parameter));
	
	param->name = name;
	param->name_internal = NULL;
	param->units = NULL;
	param->value = level;
	param->min = min;
	param->min_expr = NULL;
	param->max = max;
	param->max_expr = NULL;
	param->max_velocity = fabsf(DEFAULT_MAX_VELOCITY * (max - min));
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;
	param->updated = 0;
	param->effect = NULL;
	
	param->driver_index = KEST_PARAMETER_UNDRIVEN;
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	param->effect_rep.representer = NULL;
	param->effect_rep.representee = param;
	param->effect_rep.update = kest_parameter_effect_rep_update;
	param->widget_rep.representer = NULL;
	param->widget_rep.representee = param;
	param->widget_rep.update = kest_parameter_widget_rep_update;
	
	kest_representation_ptr_list_init_reserved(&param->reps, 1);
	//kest_representation_ptr_list_append(&param->reps, &param->effect_rep);
	#endif
	return NO_ERROR;
}

kest_parameter *new_m_parameter_wni(const char *name, const char *name_internal, float level, float min, float max)
{
	kest_parameter *param = kest_allocator_alloc(&kest_parameter_allocator, 1);
	
	if (!param)
		return NULL;
	
	init_parameter_wni(param, name, name_internal, level, min, max);
	
	return param;
}

int init_parameter_wni(kest_parameter *param, const char *name, const char *name_internal, float level, float min, float max)
{
	if (!param)
		return ERR_NULL_PTR;
	init_parameter(param, name, level, min, max);
	param->name_internal = name_internal;
	
	return NO_ERROR;
}

int init_setting_str(kest_setting *setting)
{
	if (!setting)
		return ERR_NULL_PTR;
	
	memset(setting, 0, sizeof(kest_setting));
	
	setting->type = EFFECT_SETTING_INT;
	
	setting->widget_type = SETTING_WIDGET_FIELD;
	setting->page = EFFECT_SETTING_PAGE_SETTINGS;
	setting->group = -1;
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	setting->effect_rep.representee = setting;
	setting->effect_rep.update = kest_setting_effect_rep_update;
	kest_representation_ptr_list_init_reserved(&setting->reps, 2);
	kest_representation_ptr_list_append(&setting->reps, &setting->effect_rep);
	#endif
	
	return NO_ERROR;
}

int init_setting(kest_setting *setting, const char *name, uint16_t level)
{
	if (!setting)
		return ERR_NULL_PTR;
	
	memset(setting, 0, sizeof(kest_setting));
	
	setting->name = name;
	setting->value = level;
	
	setting->widget_type = SETTING_WIDGET_DROPDOWN;
	
	setting->group = -1;
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	setting->effect_rep.representee = setting;
	setting->effect_rep.update = kest_setting_effect_rep_update;
	kest_representation_ptr_list_init_reserved(&setting->reps, 1);
	kest_representation_ptr_list_append(&setting->reps, &setting->effect_rep);
	#endif
	
	return NO_ERROR;
}

void gut_parameter(kest_parameter *param)
{
	if (!param)
		return;
	
	
	
	return;
}

void kest_parameter_free(kest_parameter *param)
{
	if (!param)
		return;
	
	gut_parameter(param);
	kest_allocator_free(&kest_parameter_allocator, param);
	
	return;
}

int parameter_set_id(kest_parameter *param, uint16_t pid, uint16_t tid, uint16_t ppid)
{
	if (!param)
		return ERR_NULL_PTR;
	
	param->id.preset_id 	= pid;
	param->id.effect_id = tid;
	param->id.parameter_id 	= ppid;
	
	return NO_ERROR;
}

void clone_parameter(kest_parameter *dest, kest_parameter *src)
{
	if (!dest || !src)
		return;
	
	init_parameter_str(dest);
	
	dest->value = src->value;
	dest->min = src->min;
	dest->min_expr = src->min_expr;
	dest->max = src->max;
	dest->max_expr = src->max_expr;
	
	dest->widget_type = src->widget_type;
	dest->name = src->name;
	dest->name_internal = src->name_internal;
	dest->units = src->units;
	
	dest->max_velocity = src->max_velocity;
	
	dest->scale = src->scale;
	
	dest->group = src->group;
	
	dest->effect = src->effect;
	dest->driver_index = src->driver_index;
	
	dest->id.parameter_id = src->id.parameter_id;
	
	dest->updated = 1;
	
	return;
}

kest_parameter *kest_parameter_make_clone(kest_parameter *src)
{
	if (!src)
		return NULL;
	
	kest_parameter *param = kest_allocator_alloc(&kest_parameter_allocator, 1);
	
	if (!param)
		return NULL;
	
	clone_parameter(param, src);
	
	return param;
}

kest_parameter *kest_parameter_make_clone_for_effect(kest_parameter *src, kest_effect *effect)
{
	KEST_PRINTF("\n");
	if (!src)
		return NULL;
	
	kest_parameter *param = kest_allocator_alloc(&kest_parameter_allocator, 1);
	
	if (!param)
		return NULL;
	
	KEST_PRINTF("\n");
	clone_parameter(param, src);
	
	param->effect = effect;
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	KEST_PRINTF("\n");
	param->effect_rep.representer = (void*)effect;
	
	kest_representation_ptr_list_append(&param->reps, &param->effect_rep);
	#endif
	
	return param;
}

int clone_setting(kest_setting *dest, kest_setting *src)
{
	if (!dest || !src)
		return ERR_NULL_PTR;
	
	init_setting_str(dest);
	
	dest->id = src->id;
	
	dest->value = src->value;
	dest->min = src->min;
	dest->max = src->max;
	
	dest->n_options = src->n_options;
	
	if (dest->n_options)
	{
		dest->options = kest_alloc(sizeof(kest_setting_option) * dest->n_options);
		
		if (!dest->options)
		{
			return ERR_ALLOC_FAIL;
		}
		
		for (int i = 0; i < dest->n_options; i++)
		{
			if (src->options)
			{
				memcpy(&dest->options[i], &src->options[i], sizeof(kest_setting_option));
			}
			else
			{
				dest->options[i].value = i;
				dest->options[i].name = "";
			}
		}
	}
	
	dest->widget_type = src->widget_type;
	dest->name = src->name;
	dest->name_internal = src->name_internal;
	
	dest->group = src->group;
	
	dest->units = src->units;
	dest->page = src->page;
	
	dest->updated = 1;
	
	return NO_ERROR;
}


kest_setting *kest_setting_make_clone(kest_setting *src)
{
	if (!src)
		return NULL;
	
	kest_setting *setting = kest_allocator_alloc(&kest_setting_allocator, 1);
	
	if (!setting)
		return NULL;
	
	clone_setting(setting, src);
	
	return setting;
}

kest_setting *kest_setting_make_clone_for_effect(kest_setting *src, kest_effect *effect)
{
	if (!src)
		return NULL;
	
	kest_setting *setting = kest_allocator_alloc(&kest_setting_allocator, 1);
	
	if (!setting)
		return NULL;
	
	clone_setting(setting, src);
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	setting->effect_rep.representer = (void*)effect;
	#endif
	
	return setting;
}

void gut_setting(kest_setting *setting)
{
	if (!setting)
		return;
	
	if (setting->options)
	{
		kest_free(setting->options);
	}
	
	return;
}


void kest_setting_free(kest_setting *setting)
{
	if (!setting)
		return;
	
	gut_setting(setting);
	kest_allocator_free(&kest_setting_allocator, setting);
	
	return;
}

int kest_parameters_assign_ids(kest_parameter_pll *list)
{
	int next_parameter_id = 0;
	KEST_PRINTF("kest_parameters_assign_ids\n");
	
	kest_parameter_pll *current = list;
	
	while (current)
	{
		if (current->data)
		{
			KEST_PRINTF("Assigning ID %d...\n",
				next_parameter_id);
			current->data->id.parameter_id = next_parameter_id++;
		}
		current = current->next;
	}
	
	KEST_PRINTF("kest_parameters_assign_ids done\n");
	return NO_ERROR;
}

int kest_settings_assign_ids(kest_setting_pll *list)
{
	int next_setting_id = 0;
	KEST_PRINTF("kest_settings_assign_ids\n");
	
	kest_setting_pll *current = list;
	
	while (current)
	{
		if (current->data)
		{
			KEST_PRINTF("Assigning ID %d...\n",
				next_setting_id);
			current->data->id.setting_id = next_setting_id++;
		}
		current = current->next;
	}
	
	KEST_PRINTF("kest_settings_assign_ids done\n");
	return NO_ERROR;
}

#ifdef KEST_ENABLE_GLOBAL_CONTEXT
kest_interval kest_parameter_get_range(kest_parameter *param)
{
	KEST_PRINTF("kest_parameter_get_range\n");
	
	kest_interval i = kest_interval_real_line();
	
	if (!param) return i;
	
	kest_effect *effect = param->effect;
	
	if (effect && !effect->scope)
	{
		KEST_PRINTF("effect is not in posession of a scope... strange\n");
		return i;
	}
	
	int no_effect = 0;
	int no_scope = 0;
	
	if (param->min_expr)
	{
		if (kest_expression_is_constant(param->min_expr))
		{
			i.a = kest_expression_evaluate(param->min_expr, NULL);
		}
		else
		{
			if (effect && effect->scope)
			{
				i.a = kest_expression_evaluate(param->min_expr, effect->scope);
			}
			else
			{
				if (param->min_expr->cached)
				{
					i.a = param->min_expr->cached_val;
				}
			}
		}
	}
	
	if (param->max_expr)
	{
		if (kest_expression_is_constant(param->max_expr))
		{
			i.b = kest_expression_evaluate(param->max_expr, NULL);
		}
		else
		{
			if (effect && effect->scope)
			{
				i.b = kest_expression_evaluate(param->max_expr, effect->scope);
			}
			else
			{
				if (param->max_expr->cached)
				{
					i.b = param->max_expr->cached_val;
				}
			}
		}
	}
	
	return i;
}
#endif

void kest_parameter_effect_rep_update(void *representer, void *representee)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	KEST_PRINTF("kest_parameter_effect_rep_update");
	kest_effect *effect = (kest_effect*)representer;
	kest_parameter *param = (kest_parameter*)representee;
	KEST_PRINTF("(param = %p, effect = %p)\n", param, effect);
	
	if (!effect || !param)
		return;
	
	kest_effect_update_reps(effect);
	
	KEST_PRINTF("kest_parameter_effect_rep_update done\n");
	#endif
	return;
}

void kest_parameter_widget_rep_update(void *representer, void *representee)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	KEST_PRINTF("kest_parameter_widget_rep_update\n");
	kest_parameter_widget *pw = (kest_parameter_widget*)representer;
	kest_parameter *param = (kest_parameter*)representee;
	KEST_PRINTF("(param = %p, pw = %p)\n", param, pw);
	
	if (!pw || !param)
		return;
	
	parameter_widget_refresh(pw);
	
	KEST_PRINTF("kest_parameter_widget_rep_update done\n");
	#endif
	return;
}

void kest_setting_effect_rep_update(void *representer, void *representee)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	kest_effect *effect = (kest_effect*)representer;
	kest_setting *setting = (kest_setting*)representee;
	
	if (!effect || !setting)
		return;
	
	kest_effect_update_reps(effect);
	
	#endif
	return;
}

float kest_parameter_evaluate(kest_parameter *param)
{
	KEST_PRINTF("kest_parameter_evaluate(param = %p)\n", param);
	if (!param)
		return 1.0f;
	
	KEST_PRINTF("Parameter name: %s, ID: {%d.%d.%d}, value: %s%.05f. param->driver_index = %d. param->pw = %p\n",
		param->name, param->id.preset_id, param->id.effect_id, param->id.parameter_id,
		param->value > 0 ? " " : "", param->value, param->driver_index, param->pw);
	
	/*KEST_PRINTF("param->driver_index = %d\n", param->driver_index);
	KEST_PRINTF("param->driver_override = %d\n", param->driver_override);
	KEST_PRINTF("param->effect = %p\n", param->effect);
	if (param->effect)
		KEST_PRINTF("param->effect->drivers.count = %d\n", param->effect->drivers.count);
	
	KEST_PRINTF("   param->driver_index >= 0 && !param->driver_override && param->effect && param->effect->drivers.count > param->driver_index\n");
	KEST_PRINTF(" =           %d             &&            %d           &&       %d      &&           %d\n",
		param->driver_index >= 0,  !param->driver_override,  param->effect,  (param->effect ? param->effect->drivers.count : 0) > param->driver_index
	);
	KEST_PRINTF(" = %d\n", (param->driver_index >= 0 && !param->driver_override && param->effect && (param->effect ? param->effect->drivers.count : 0) > param->driver_index));*/
	
	if (param->driver_index >= 0 && !param->driver_override && param->effect && param->effect->drivers.count > param->driver_index)
		kest_driver_evaluate(&param->effect->drivers.entries[param->driver_index], &param->value);
	
	kest_interval i = kest_parameter_get_range(param);
	
	if (param->value > i.b)
		param->value = i.b;
	if (param->value < i.a)
		param->value = i.a;
	
	KEST_PRINTF("Returning result (param = %p) %s%.05f\n", param, param->value < 0 ? "" : " ", param->value);
	
	return param->value;
}

int kest_parameter_if_driven_refresh(kest_parameter *param)
{
	KEST_PRINTF("kest_parameter_if_driven_refresh(param = %p)\n", param);
	
	if (!param)
		return ERR_NULL_PTR;
	
	/*KEST_PRINTF("kest_parameter_if_driven_refresh(param = %p)\n", param);
	
	KEST_PRINTF("param->driver_index = %d\n", param->driver_index);
	KEST_PRINTF("param->driver_override = %d\n", param->driver_override);
	KEST_PRINTF("param->effect->drivers.count = %d\n", param->effect->drivers.count);
	
	KEST_PRINTF("   param->driver_index >= 0 && !param->driver_override && param->effect && param->effect->drivers.count > param->driver_index\n");
	KEST_PRINTF("   param->driver_index >= 0 && !param->driver_override && param->effect && param->effect->drivers.count > param->driver_index\n");
	KEST_PRINTF(" =           %d             &&            %d           &&       %d      &&           %d\n",
			param->driver_index >= 0,  !param->driver_override,  param->effect,  param->effect->drivers.count > param->driver_index
		);*/
	
	
	if (param && param->driver_index >= 0 && !param->driver_override && param->effect && param->effect->drivers.count > param->driver_index)
	{
		float value = param->value;
		
		KEST_PRINTF("Parameter %p appears to be validly driven. Recompute value\n", param);
		kest_driver_evaluate(&param->effect->drivers.entries[param->driver_index], &param->value);
		
		param->updated = 1;
	}
	
	return NO_ERROR;
}


int kest_parameter_set(kest_parameter *param, float v)
{
	if (!param)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("kest_parameter_set(param = %p, v = %f)\n", param, v);
	
	KEST_PRINTF("param->driver_index = %d, param->driver_override = %d\n", param->driver_index, param->driver_override);
	if (param->driver_index >= 0 && !param->driver_override)
	{
		KEST_PRINTF("kest_parameter_set(param = %p, v = %f) OVERRULED\n");
		return ERR_OVERRULED;
	}
	
	// Note: add CLAMPS here
	
	param->value = v;
	param->updated = 1;
	
	return NO_ERROR;
}

int kest_parameter_driver_set(kest_parameter *param, float v)
{
	if (!param)
		return ERR_NULL_PTR;
	
	param->value = v;
	param->updated = 1;
	
	return NO_ERROR;
}

void kest_parameter_if_updated_refresh_pw(void *param_)
{	
	kest_parameter *param = param_;
	
	if (!param)
		return;
	
	KEST_PRINTF("kest_parameter_if_updated_refresh_pw(param = %s)\n", param->name);
	
	KEST_PRINTF("param->pw = %p, param->updated = %d, param->driver_index = %d, param->driver_override = %d\n",
		param->pw, param->updated, param->driver_index, param->driver_override);
	
	if (param->pw && param->updated)
	{
		kest_parameter_widget_refresh(param->pw);
	}
}
