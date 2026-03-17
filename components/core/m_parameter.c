#include <float.h>
#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

#define DEFAULT_MAX_VELOCITY 1.0

IMPLEMENT_LINKED_PTR_LIST(m_parameter);
IMPLEMENT_LINKED_PTR_LIST(m_setting);

static const char *FNAME = "m_parameter.c";

int init_parameter_str(m_parameter *param)
{
	if (!param)
		return ERR_NULL_PTR;
	
	param->value = 0.0;
	param->min = 0.0;
	param->min_expr = NULL;
	param->max = 1.0;
	param->max_expr = NULL;
	param->max_velocity = DEFAULT_MAX_VELOCITY;
	param->factor = 1.0;
	param->id = (m_parameter_id){.profile_id = 0, .effect_id = 0, .parameter_id = 0};
	param->name = NULL;
	param->name_internal = NULL;
	param->units = NULL;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	
	#ifdef M_ENABLE_REPRESENTATIONS
	param->reps = NULL;
	param->effect_rep.representer = NULL;
	param->effect_rep.representee = param;
	param->effect_rep.update = m_parameter_effect_rep_update;
	m_representation_pll_safe_append(&param->reps, &param->effect_rep);
	#endif
	
	return NO_ERROR;
}

int init_parameter(m_parameter *param, const char *name, float level, float min, float max)
{
	if (!param) return ERR_NULL_PTR;
	
	param->name = name;
	param->name_internal = NULL;
	param->units = NULL;
	param->value = level;
	param->min = min;
	param->min_expr = NULL;
	param->max = max;
	param->max_expr = NULL;
	param->max_velocity = fabsf(DEFAULT_MAX_VELOCITY * (max - min));
	param->factor = 1.0;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;
	
	#ifdef M_ENABLE_REPRESENTATIONS
	param->reps = NULL;
	param->effect_rep.representer = NULL;
	param->effect_rep.representee = param;
	param->effect_rep.update = m_parameter_effect_rep_update;
	m_representation_pll_safe_append(&param->reps, &param->effect_rep);
	#endif
	return NO_ERROR;
}

m_parameter *new_m_parameter_wni(const char *name, const char *name_internal, float level, float min, float max)
{
	m_parameter *param = m_alloc(sizeof(m_parameter));
	if (!param)
		return NULL;
	
	init_parameter_wni(param, name, name_internal, level, min, max);
	
	return param;
}

int init_parameter_wni(m_parameter *param, const char *name, const char *name_internal, float level, float min, float max)
{
	if (!param)
		return ERR_NULL_PTR;
	init_parameter(param, name, level, min, max);
	param->name_internal = name_internal;
	
	return NO_ERROR;
}

int init_setting_str(m_setting *setting)
{
	if (!setting)
		return ERR_NULL_PTR;
	
	setting->type = TRANSFORMER_SETTING_INT;
	
	setting->name = NULL;
	setting->name_internal = NULL;
	setting->value = 0;
	
	setting->n_options = 0;
	setting->options = NULL;
	
	setting->widget_type = SETTING_WIDGET_FIELD;
	setting->page = TRANSFORMER_SETTING_PAGE_SETTINGS;
	setting->group = -1;
	
	#ifdef M_ENABLE_REPRESENTATIONS
	setting->reps = NULL;
	setting->effect_rep.representer = NULL;
	setting->effect_rep.representee = setting;
	setting->effect_rep.update = m_setting_effect_rep_update;
	m_representation_pll_safe_append(&setting->reps, &setting->effect_rep);
	#endif
	
	return NO_ERROR;
}

int init_setting(m_setting *setting, const char *name, uint16_t level)
{
	if (!setting)
		return ERR_NULL_PTR;
	
	setting->name = name;
	setting->value = level;
	
	setting->n_options = 0;
	setting->options = NULL;
	
	setting->widget_type = SETTING_WIDGET_DROPDOWN;
	
	setting->group = -1;
	
	#ifdef M_ENABLE_REPRESENTATIONS
	setting->reps = NULL;
	setting->effect_rep.representer = NULL;
	setting->effect_rep.representee = setting;
	setting->effect_rep.update = m_setting_effect_rep_update;
	m_representation_pll_safe_append(&setting->reps, &setting->effect_rep);
	#endif
	
	return NO_ERROR;
}

void gut_parameter(m_parameter *param)
{
	if (!param)
		return;
	
	
	
	return;
}

void m_parameter_free(m_parameter *param)
{
	if (!param)
		return;
	
	gut_parameter(param);
	m_free(param);
	
	return;
}

int parameter_set_id(m_parameter *param, uint16_t pid, uint16_t tid, uint16_t ppid)
{
	if (!param)
		return ERR_NULL_PTR;
	
	param->id.profile_id 	= pid;
	param->id.effect_id = tid;
	param->id.parameter_id 	= ppid;
	
	return NO_ERROR;
}

void clone_parameter(m_parameter *dest, m_parameter *src)
{
	if (!dest || !src)
		return;
	
	init_parameter_str(dest);
	
	dest->value = src->value;
	dest->min = src->min;
	dest->min_expr = src->min_expr;
	dest->max = src->max;
	dest->max_expr = src->max_expr;
	
	dest->factor = src->factor;
	
	dest->widget_type = src->widget_type;
	dest->name = src->name;
	dest->name_internal = src->name_internal;
	dest->units = src->units;
	
	dest->max_velocity = src->max_velocity;
	
	dest->scale = src->scale;
	
	dest->group = src->group;
	
	dest->id.parameter_id = src->id.parameter_id;
	
	return;
}

m_parameter *m_parameter_make_clone(m_parameter *src)
{
	if (!src)
		return NULL;
	
	m_parameter *param = m_alloc(sizeof(m_parameter));
	
	if (!param)
		return NULL;
	
	clone_parameter(param, src);
	
	return param;
}

m_parameter *m_parameter_make_clone_for_effect(m_parameter *src, m_effect *effect)
{
	if (!src)
		return NULL;
	
	m_parameter *param = m_alloc(sizeof(m_parameter));
	
	if (!param)
		return NULL;
	
	clone_parameter(param, src);
	
	#ifdef M_ENABLE_REPRESENTATIONS
	param->effect_rep.representer = (void*)effect;
	#endif
	
	return param;
}

int clone_setting(m_setting *dest, m_setting *src)
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
		dest->options = m_alloc(sizeof(m_setting_option) * dest->n_options);
		
		if (!dest->options)
		{
			return ERR_ALLOC_FAIL;
		}
		
		for (int i = 0; i < dest->n_options; i++)
		{
			if (src->options)
			{
				memcpy(&dest->options[i], &src->options[i], sizeof(m_setting_option));
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
	
	return NO_ERROR;
}


m_setting *m_setting_make_clone(m_setting *src)
{
	if (!src)
		return NULL;
	
	m_setting *setting = m_alloc(sizeof(m_setting));
	
	if (!setting)
		return NULL;
	
	clone_setting(setting, src);
	
	return setting;
}

m_setting *m_setting_make_clone_for_effect(m_setting *src, m_effect *effect)
{
	if (!src)
		return NULL;
	
	m_setting *setting = m_alloc(sizeof(m_setting));
	
	if (!setting)
		return NULL;
	
	clone_setting(setting, src);
	
	#ifdef M_ENABLE_REPRESENTATIONS
	setting->effect_rep.representer = (void*)effect;
	#endif
	
	return setting;
}

void gut_setting(m_setting *setting)
{
	if (!setting)
		return;
	
	if (setting->options)
	{
		m_free(setting->options);
	}
	
	return;
}


void m_setting_free(m_setting *setting)
{
	if (!setting)
		return;
	
	gut_setting(setting);
	m_free(setting);
	
	return;
}

int m_parameters_assign_ids(m_parameter_pll *list)
{
	int next_parameter_id = 0;
	M_PRINTF("m_parameters_assign_ids\n");
	
	m_parameter_pll *current = list;
	
	while (current)
	{
		if (current->data)
		{
			M_PRINTF("Assigning ID %d...\n",
				next_parameter_id);
			current->data->id.parameter_id = next_parameter_id++;
		}
		current = current->next;
	}
	
	M_PRINTF("m_parameters_assign_ids done\n");
	return NO_ERROR;
}

int m_settings_assign_ids(m_setting_pll *list)
{
	int next_setting_id = 0;
	M_PRINTF("m_settings_assign_ids\n");
	
	m_setting_pll *current = list;
	
	while (current)
	{
		if (current->data)
		{
			M_PRINTF("Assigning ID %d...\n",
				next_setting_id);
			current->data->id.setting_id = next_setting_id++;
		}
		current = current->next;
	}
	
	M_PRINTF("m_settings_assign_ids done\n");
	return NO_ERROR;
}

#ifdef M_ENABLE_GLOBAL_CONTEXT
m_interval m_parameter_get_range(m_parameter *param)
{	
	m_interval i = m_interval_real_line();
	
	if (!param) return i;
	
	m_effect *effect = cxt_get_effect_by_id(&global_cxt, param->id.profile_id, param->id.effect_id);
	
	if (effect && !effect->scope)
	{
		M_PRINTF("effect is not in posession of a scope... strange. generate it\n");
		effect->scope = m_effect_create_scope(effect);
	}
	
	int no_effect = 0;
	int no_scope = 0;
	
	if (param->min_expr)
	{
		if (m_expression_is_constant(param->min_expr))
		{
			i.a = m_expression_evaluate(param->min_expr, NULL);
		}
		else
		{
			if (effect && effect->scope)
			{
				i.a = m_expression_evaluate(param->min_expr, effect->scope);
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
		if (m_expression_is_constant(param->max_expr))
		{
			i.b = m_expression_evaluate(param->max_expr, NULL);
		}
		else
		{
			if (effect && effect->scope)
			{
				i.b = m_expression_evaluate(param->max_expr, effect->scope);
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

void m_parameter_effect_rep_update(void *representer, void *representee)
{
	m_effect *effect = (m_effect*)representer;
	m_parameter *param = (m_parameter*)representee;
	
	if (!effect || !param)
		return;
	
	m_effect_update_reps(effect);
	
	return;
}

void m_setting_effect_rep_update(void *representer, void *representee)
{
	m_effect *effect = (m_effect*)representer;
	m_setting *setting = (m_setting*)representee;
	
	if (!effect || !setting)
		return;
	
	m_effect_update_reps(effect);
	
	return;
}
