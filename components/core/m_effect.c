#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

#define INITIAL_PARAMETER_ARRAY_LENGTH 	8
#define PARAMETER_ARRAY_CHUNK_SIZE	 	8

#define INITIAL_OPTION_ARRAY_LENGTH 	8
#define OPTION_ARRAY_CHUNK_SIZE	 		8

static const char *FNAME = "m_effect.c";

IMPLEMENT_LINKED_PTR_LIST(m_effect);

const char *m_effect_name(m_effect *effect)
{
	if (!effect)
		return "(NULL)";
	
	if (!effect->eff)
		return "(Unknown)";
	
	return effect->eff->name;
}


int init_effect(m_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;
	
	effect->id = 0;
	effect->type = 0;
	
	effect->position = 0;
	
	effect->parameters = NULL;
	effect->settings = NULL;
	effect->scope = NULL;
	
	#ifdef M_ENABLE_UI
	effect->view_page = NULL;
	#endif
	effect->profile = NULL;
	
	init_parameter(&effect->wet_mix, "Wet Mix", 1.0, 0.0, 1.0);
	effect->wet_mix.id.parameter_id = TRANSFORMER_WET_MIX_PID;
	effect->wet_mix.min_expr = &m_expression_zero;
	effect->wet_mix.max_expr = &m_expression_one;
	
	init_setting(&effect->band_mode, "Apply to", TRANSFORMER_MODE_FULL_SPECTRUM);
	effect->band_mode.id.setting_id = TRANSFORMER_BAND_MODE_SID;
	
	effect->band_mode.n_options = 4;
	effect->band_mode.options = m_alloc(sizeof(m_setting_option) * effect->band_mode.n_options);
	
	if (!effect->band_mode.options)
		return ERR_ALLOC_FAIL;
	
	effect->band_mode.options[0].value = TRANSFORMER_MODE_FULL_SPECTRUM;
	effect->band_mode.options[0].name  = "All freq";
	
	effect->band_mode.options[1].value = TRANSFORMER_MODE_UPPER_SPECTRUM;
	effect->band_mode.options[1].name  = "Freq > cutoff";
	
	effect->band_mode.options[2].value = TRANSFORMER_MODE_LOWER_SPECTRUM;
	effect->band_mode.options[2].name  = "Freq < cutoff";
	
	effect->band_mode.options[3].value = TRANSFORMER_MODE_BAND;
	effect->band_mode.options[3].name  = "Freq in band";
	
	init_parameter(&effect->band_lp_cutoff, "Cutoff", 4000.0, 1, 4000);
	effect->band_lp_cutoff.scale = PARAMETER_SCALE_LOGARITHMIC;
	effect->band_lp_cutoff.id.parameter_id = TRANSFORMER_BAND_LP_CUTOFF_PID;
	effect->band_lp_cutoff.min_expr = &m_expression_one;
	effect->band_lp_cutoff.max_expr = &m_expression_freq_max;
	
	init_parameter(&effect->band_hp_cutoff, "Cutoff", 1.0, 1, 4000);
	effect->band_hp_cutoff.scale = PARAMETER_SCALE_LOGARITHMIC;
	effect->band_hp_cutoff.id.parameter_id = TRANSFORMER_BAND_HP_CUTOFF_PID;
	effect->band_hp_cutoff.min_expr = &m_expression_one;
	effect->band_hp_cutoff.max_expr = &m_expression_freq_max;
	
	effect->eff = NULL;
	
	#ifdef M_USE_FREERTOS
	effect->mutex = xSemaphoreCreateMutex();
	#endif
	
	#ifdef M_ENABLE_REPRESENTATIONS
	effect->reps = NULL;
	effect->profile_rep.representer = NULL;
	effect->profile_rep.representee = effect;
	effect->profile_rep.update = m_effect_profile_rep_update;
	m_representation_pll_safe_append(&effect->reps, &effect->profile_rep);
	#endif
	
	return NO_ERROR;
}

int init_effect_from_effect_desc(m_effect *effect, m_effect_desc *eff)
{
	init_effect(effect);
	effect->eff = eff;
	
	m_parameter_pll *current_param = eff->parameters;
	m_setting_pll *current_setting = eff->settings;
	
	while (current_param)
	{
		m_parameter_pll_safe_append(&effect->parameters, m_parameter_make_clone_for_effect(current_param->data, effect));
		current_param = current_param->next;
	}
	
	m_setting *setting;
	
	while (current_setting)
	{
		m_setting_pll_safe_append(&effect->settings, m_setting_make_clone_for_effect(current_setting->data, effect));
		current_setting = current_setting->next;
	}
	
	effect->scope = m_effect_create_scope(effect);
	
	return NO_ERROR;
}

int effect_rectify_param_ids(m_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;
	
	m_parameter_pll *current_param = effect->parameters;
	
	while (current_param)
	{
		if (current_param->data)
		{
			if (effect->profile)
				current_param->data->id.profile_id = effect->profile->id;
			current_param->data->id.effect_id = effect->id;
		}
		
		current_param = current_param->next;
	}
	
	setting_ll *current_setting = effect->settings;
	
	while (current_setting)
	{
		if (current_setting->data)
		{
			if (effect->profile)
				current_setting->data->id.profile_id = effect->profile->id;
			current_setting->data->id.effect_id = effect->id;
		}
		
		current_setting = current_setting->next;
	}
	
	if (effect->profile)
		effect->band_mode.id.profile_id = effect->profile->id;
	effect->band_mode.id.effect_id = effect->id;
	
	if (effect->profile)
		effect->band_lp_cutoff.id.profile_id = effect->profile->id;
	effect->band_lp_cutoff.id.effect_id = effect->id;
	
	if (effect->profile)
		effect->band_hp_cutoff.id.profile_id = effect->profile->id;
	effect->band_hp_cutoff.id.effect_id = effect->id;
	
	return NO_ERROR;
}

int effect_set_id(m_effect *effect, uint16_t profile_id, uint16_t effect_id)
{
	if (!effect)
		return ERR_NULL_PTR;
	
	effect->id = effect_id;
	
	effect_rectify_param_ids(effect);
	
	return NO_ERROR;
}

int request_append_effect(uint16_t type, m_effect *local)
{
	#ifdef USE_TEENSY
	if (!local)
		return ERR_NULL_PTR;
	
	if (!local->profile)
		return ERR_BAD_ARGS;
	
	m_message msg = create_m_message(M_MESSAGE_APPEND_TRANSFORMER, "ss", local->profile->id, local->type);
	msg.callback = effect_receive_id;
	msg.cb_arg = local;
	
	return queue_msg_to_teensy(msg);
	#endif
	
	return NO_ERROR;
}

#ifdef USE_TEENSY
void effect_receive_id(m_message message, m_response response)
{
	M_PRINTF("Transformer receive ID!\n");
	m_effect *effect = message.cb_arg;
	
	if (!effect)
		return;
	
	uint16_t pid, tid;
	
	memcpy(&pid, &response.data[0], sizeof(uint16_t));
	memcpy(&tid, &response.data[2], sizeof(uint16_t));
	
	if (!effect->profile || pid != effect->profile->id)
	{
		M_PRINTF("Transformer ID for effect in profile %d sent to effect in %d\n", pid, effect->profile->id);
	}
	else
	{
		M_PRINTF("Transformer %p obtains id %d.%d\n", effect, pid, tid);
		effect->id = tid;
		
		effect_rectify_param_ids(effect);
	}
}
#endif

m_parameter *effect_add_parameter(m_effect *effect)
{
	if (!effect)
		return NULL;
	
	int ret_val;
	
	m_parameter *param = m_alloc(sizeof(m_parameter));
	
	if (!param)
		return NULL;
	
	init_parameter_str(param);
	
	m_parameter_pll *nl = m_parameter_pll_append(effect->parameters, param);
	
	if (!nl)
	{
		m_free(param);
		return NULL;
	}
	
	effect->parameters = nl;
	
	return param;
}

m_setting *effect_add_setting(m_effect *effect)
{
	if (!effect)
		return NULL;
	
	int ret_val;
	
	m_setting *setting = m_alloc(sizeof(m_setting));
	
	if (!setting)
		return NULL;
	
	init_setting_str(setting);
	
	setting_ll *nl = m_setting_pll_append(effect->settings, setting);
	
	if (!nl)
	{
		m_free(setting);
		return NULL;
	}
	
	effect->settings = nl;
	
	return setting;
}

#ifdef M_ENABLE_UI

int m_effect_init_view_page(m_effect *effect, m_ui_page *parent)
{
	M_PRINTF("effect_init_ui_page. effect = %p, parent = %p\n", effect, parent);
	if (!effect)
		return ERR_NULL_PTR;
	
	effect->view_page = m_alloc(sizeof(m_ui_page));
	
	if (!effect->view_page)
		return ERR_ALLOC_FAIL;
	
	init_ui_page(effect->view_page);
	init_effect_view(effect->view_page);
	configure_effect_view(effect->view_page, effect);
	effect->view_page->parent = parent;
	
	return NO_ERROR;
}
/*
int m_effect_init_view_page(m_effect *effect)
{
	M_PRINTF("m_effect_init_view_page(effect = %p)\n", effect);
	
	if (!effect)
		return ERR_NULL_PTR;
	
	effect->view_page = m_alloc(sizeof(m_ui_page));
	
	if (!effect->view_page)
		return ERR_ALLOC_FAIL;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = init_ui_page(effect->view_page)) != NO_ERROR) return ret_val;
	if ((ret_val = init_effect_view(effect->view_page)) != NO_ERROR) return ret_val;
	if ((ret_val = configure_effect_view(effect->view_page, effect)) != NO_ERROR) return ret_val;
	
	m_profile *profile = effect->profile;
	
	M_PRINTF("effect->profile = %p\n", effect->profile);
	
	if (profile)
	{
		M_PRINTF("effect->profile->view_page = %p\n", effect->profile->view_page);
		effect->view_page->parent = profile->view_page;
	}
	
	M_PRINTF("m_effect_init_view_page done\n");
	return NO_ERROR;
}*/
#endif

int clone_effect(m_effect *dest, m_effect *src)
{
	if (!src || !dest)
		return ERR_NULL_PTR;
	
	uint16_t profile_id;
	uint16_t effect_id;
	
	init_effect(dest);
	
	dest->type = src->type;
	dest->position = src->position;
	
	m_parameter_pll *current_param = src->parameters;
	m_parameter *param;
	
	while (current_param)
	{
		if(current_param->data)
		{
			param = effect_add_parameter(dest);
			
			if (param)
			{
				memcpy(param, current_param->data, sizeof(m_parameter));
			}
			else
			{
				return ERR_ALLOC_FAIL;
			}
		}
		
		current_param = current_param->next;
	}
	
	setting_ll *current_setting = src->settings;
	m_setting *setting;
	
	while (current_setting)
	{
		if(current_setting->data)
		{
			setting = effect_add_setting(dest);
			
			if (setting)
			{
				clone_setting(setting, current_setting->data);
			}
			else
			{
				return ERR_ALLOC_FAIL;
			}
		}
		
		current_setting = current_setting->next;
	}
	
	#ifdef M_ENABLE_UI
	src->view_page = NULL;
	#endif
	
	return NO_ERROR;
}


void gut_effect(m_effect *effect)
{
	if (!effect)
		return;
	
	free_m_parameter_pll(effect->parameters);
	destructor_free_m_setting_pll(effect->settings, gut_setting);
	effect->parameters = NULL;
	effect->settings = NULL;
	
	gut_setting(&effect->band_mode);
	
	#ifdef M_ENABLE_UI
	free_effect_view(effect->view_page);
	effect->view_page = NULL;
	#endif
	
	effect->id 		= 0;
	effect->type 	= 0;
	effect->position = 0;
}


void free_effect(m_effect *effect)
{
	if (!effect)
		return;
	
	destructor_free_m_parameter_pll(effect->parameters, m_parameter_free);
	destructor_free_m_setting_pll(effect->settings, m_setting_free);
	
	#ifdef M_ENABLE_UI
	free_effect_view(effect->view_page);
	#endif
	
	m_free(effect);
}

m_parameter *effect_get_parameter(m_effect *effect, int n)
{
	if (!effect)
		return NULL;
	
	m_parameter_pll *current = effect->parameters;
	int i = 0;
	
	while (current && i < n)
	{
		current = current->next;
		i++;
	}
	
	return current ? current->data : NULL;
}


m_setting *effect_get_setting(m_effect *effect, int n)
{
	if (!effect)
		return NULL;
	
	setting_ll *current = effect->settings;
	int i = 0;
	
	while (current && i < n)
	{
		current = current->next;
		i++;
	}
	
	return current ? current->data : NULL;
}

int m_effect_update_fpga_registers(m_effect *effect)
{
	#ifdef M_ENABLE_FPGA

	if (!effect)
		return ERR_NULL_PTR;
	
	if (!effect->eff)
		return ERR_BAD_ARGS;
	
	m_fpga_transfer_batch batch = m_new_fpga_transfer_batch();
	
	if (!batch.buf)
		return ERR_ALLOC_FAIL;
	
	m_fpga_transfer_batch_append_effect_register_updates(&batch, effect->eff, effect->scope, effect->block_position);
	
	int ret_val = m_fpga_queue_transfer_batch(batch);
	
	return ret_val;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
}


m_expr_scope *m_effect_create_scope(m_effect *effect)
{
	if (!effect)
		return NULL;
	
	m_expr_scope *scope = m_new_expr_scope();
	
	if (!scope)
		return NULL;
	
	m_parameter_pll *current_param = effect->parameters;
	m_setting_pll *current_setting = effect->settings;
	m_named_expression_pll *current_def_expr = effect->eff ? effect->eff->def_exprs : NULL;
	
	while (current_param)
	{
		if (current_param->data)
			m_expr_scope_add_param(scope, current_param->data);
		
		current_param = current_param->next;
	}
	
	while (current_setting)
	{
		if (current_setting->data)
			m_expr_scope_add_setting(scope, current_setting->data);
		
		current_setting = current_setting->next;
	}
	
	
	while (current_def_expr)
	{
		if (current_def_expr->data)
			m_expr_scope_add_expr(scope, current_def_expr->data->name, current_def_expr->data->expr);
		
		current_def_expr = current_def_expr->next;
	}
	
	return scope;
}

int m_effect_set_parameter(m_effect *effect, const char *name, float value)
{
	if (!effect)
		return ERR_NULL_PTR;
	
	int ret_val;
	
	m_parameter_pll *current = effect->parameters;
	
	while (current)
	{
		if (current && current->data && current->data->name_internal && (strcmp(current->data->name_internal, name) == 0))
		{
			current->data->value = value;
			return NO_ERROR;
		}
		
		current = current->next;
	}
	
	return ERR_BAD_ARGS;
}

int m_effect_set_setting(m_effect *effect, const char *name, int value)
{
	if (!effect)
		return ERR_NULL_PTR;
	
	int ret_val;
	
	m_setting_pll *current = effect->settings;
	
	while (current)
	{
		if (current && current->data && current->data->name_internal && (strcmp(current->data->name_internal, name) == 0))
		{
			current->data->value = value;
			return NO_ERROR;
		}
		
		current = current->next;
	}
	
	return ERR_BAD_ARGS;
}


void m_effect_profile_rep_update(void *representer, void *representee)
{
	#ifdef M_ENABLE_REPRESENTATIONS
	m_profile *profile = representer;
	m_effect *effect = representee;
	
	if (!representer || !representee)
		return;
	
	save_profile(profile);
	#endif
	
	return;
}

int m_effect_update_reps(m_effect *effect)
{
	#ifdef M_ENABLE_REPRESENTATIONS
	if (!effect)
		return ERR_NULL_PTR;
	
	queue_representation_list_update(effect->reps);
	#endif
	
	return NO_ERROR;
}
