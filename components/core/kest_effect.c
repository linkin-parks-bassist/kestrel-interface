#include "kest_int.h"

//#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
//#endif

#define INITIAL_PARAMETER_ARRAY_LENGTH 	8
#define PARAMETER_ARRAY_CHUNK_SIZE	 	8

#define INITIAL_OPTION_ARRAY_LENGTH 	8
#define OPTION_ARRAY_CHUNK_SIZE	 		8

static const char *FNAME = "kest_effect.c";

IMPLEMENT_LINKED_PTR_LIST(kest_effect);

const char *kest_effect_name(kest_effect *effect)
{
	if (!effect)
		return "(NULL)";
	
	if (!effect->eff)
		return "(Unknown)";
	
	return effect->eff->name;
}


int init_effect(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;
	
	memset(effect, 0, sizeof(kest_effect));
	
	effect->id = 0;
	effect->type = 0;
	
	effect->position = 0;
	
	effect->parameters = NULL;
	effect->settings = NULL;
	effect->scope = NULL;
	
	#ifdef KEST_ENABLE_UI
	effect->view_page = NULL;
	#endif
	effect->preset = NULL;
	
	init_parameter(&effect->wet_mix, "Wet Mix", 1.0, 0.0, 1.0);
	effect->wet_mix.id.parameter_id = EFFECT_WET_MIX_PID;
	effect->wet_mix.min_expr = &kest_expression_zero;
	effect->wet_mix.max_expr = &kest_expression_one;
	
	init_setting(&effect->band_mode, "Apply to", EFFECT_MODE_FULL_SPECTRUM);
	effect->band_mode.id.setting_id = EFFECT_BAND_MODE_SID;
	
	effect->band_mode.n_options = 4;
	effect->band_mode.options = kest_alloc(sizeof(kest_setting_option) * effect->band_mode.n_options);
	
	if (!effect->band_mode.options)
		return ERR_ALLOC_FAIL;
	
	effect->band_mode.options[0].value = EFFECT_MODE_FULL_SPECTRUM;
	effect->band_mode.options[0].name  = "All freq";
	
	effect->band_mode.options[1].value = EFFECT_MODE_UPPER_SPECTRUM;
	effect->band_mode.options[1].name  = "Freq > cutoff";
	
	effect->band_mode.options[2].value = EFFECT_MODE_LOWER_SPECTRUM;
	effect->band_mode.options[2].name  = "Freq < cutoff";
	
	effect->band_mode.options[3].value = EFFECT_MODE_BAND;
	effect->band_mode.options[3].name  = "Freq in band";
	
	init_parameter(&effect->band_lp_cutoff, "Cutoff", 4000.0, 1, 4000);
	effect->band_lp_cutoff.scale = PARAMETER_SCALE_LOGARITHMIC;
	effect->band_lp_cutoff.id.parameter_id = EFFECT_BAND_LP_CUTOFF_PID;
	effect->band_lp_cutoff.min_expr = &kest_expression_one;
	effect->band_lp_cutoff.max_expr = &kest_expression_freq_max;
	
	init_parameter(&effect->band_hp_cutoff, "Cutoff", 1.0, 1, 4000);
	effect->band_hp_cutoff.scale = PARAMETER_SCALE_LOGARITHMIC;
	effect->band_hp_cutoff.id.parameter_id = EFFECT_BAND_HP_CUTOFF_PID;
	effect->band_hp_cutoff.min_expr = &kest_expression_one;
	effect->band_hp_cutoff.max_expr = &kest_expression_freq_max;
	
	effect->eff = NULL;
	
	#ifdef KEST_USE_FREERTOS
	effect->mutex = xSemaphoreCreateMutex();
	#endif
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	effect->reps = NULL;
	effect->preset_rep.representer = NULL;
	effect->preset_rep.representee = effect;
	effect->preset_rep.update = kest_effect_preset_rep_update;
	effect->page_rep.representer = NULL;
	effect->page_rep.representee = effect;
	effect->page_rep.update = kest_effect_page_rep_update;
	kest_representation_pll_safe_append(&effect->reps, &effect->preset_rep);
	#endif
	
	kest_effect_fpga_position_init(&effect->position_);
	
	return NO_ERROR;
}

int init_effect_from_effect_desc(kest_effect *effect, kest_effect_desc *eff)
{
	init_effect(effect);
	effect->eff = eff;
	
	kest_parameter_pll *current_param = eff->parameters;
	kest_setting_pll *current_setting = eff->settings;
	
	while (current_param)
	{
		kest_parameter_pll_safe_append(&effect->parameters, kest_parameter_make_clone_for_effect(current_param->data, effect));
		current_param = current_param->next;
	}
	
	kest_setting *setting;
	
	while (current_setting)
	{
		kest_setting_pll_safe_append(&effect->settings, kest_setting_make_clone_for_effect(current_setting->data, effect));
		current_setting = current_setting->next;
	}
	
	effect->scope = kest_effect_create_scope(effect);
	
	return NO_ERROR;
}

int effect_rectify_param_ids(kest_effect *effect)
{
	KEST_PRINTF("configure_parameter_widget(effect = %p)\n");
	if (!effect)
		return ERR_NULL_PTR;
	
	kest_parameter_pll *current_param = effect->parameters;
	
	if (effect->preset)
	{
		KEST_PRINTF("effect->preset->id = %d\n", effect->preset->id);
	}
	else
	{
		KEST_PRINTF("effect->preset = NULL !\n");
	}
	
	KEST_PRINTF("effect->id = %d\n", effect->id);
	
	while (current_param)
	{
		if (current_param->data)
		{
			if (effect->preset)
				current_param->data->id.preset_id = effect->preset->id;
			current_param->data->id.effect_id = effect->id;
		}
		
		current_param = current_param->next;
	}
	
	setting_ll *current_setting = effect->settings;
	
	while (current_setting)
	{
		if (current_setting->data)
		{
			if (effect->preset)
				current_setting->data->id.preset_id = effect->preset->id;
			current_setting->data->id.effect_id = effect->id;
		}
		
		current_setting = current_setting->next;
	}
	
	if (effect->preset)
		effect->band_mode.id.preset_id = effect->preset->id;
	effect->band_mode.id.effect_id = effect->id;
	
	if (effect->preset)
		effect->band_lp_cutoff.id.preset_id = effect->preset->id;
	effect->band_lp_cutoff.id.effect_id = effect->id;
	
	if (effect->preset)
		effect->band_hp_cutoff.id.preset_id = effect->preset->id;
	effect->band_hp_cutoff.id.effect_id = effect->id;
	
	return NO_ERROR;
}

int effect_set_id(kest_effect *effect, uint16_t preset_id, uint16_t effect_id)
{
	if (!effect)
		return ERR_NULL_PTR;
	
	effect->id = effect_id;
	
	effect_rectify_param_ids(effect);
	
	return NO_ERROR;
}

kest_parameter *effect_add_parameter(kest_effect *effect)
{
	if (!effect)
		return NULL;
	
	int ret_val;
	
	kest_parameter *param = kest_alloc(sizeof(kest_parameter));
	
	if (!param)
		return NULL;
	
	init_parameter_str(param);
	
	kest_parameter_pll *nl = kest_parameter_pll_append(effect->parameters, param);
	
	if (!nl)
	{
		kest_free(param);
		return NULL;
	}
	
	effect->parameters = nl;
	
	return param;
}

kest_setting *effect_add_setting(kest_effect *effect)
{
	if (!effect)
		return NULL;
	
	int ret_val;
	
	kest_setting *setting = kest_alloc(sizeof(kest_setting));
	
	if (!setting)
		return NULL;
	
	init_setting_str(setting);
	
	setting_ll *nl = kest_setting_pll_append(effect->settings, setting);
	
	if (!nl)
	{
		kest_free(setting);
		return NULL;
	}
	
	effect->settings = nl;
	
	return setting;
}

#ifdef KEST_ENABLE_UI

int kest_effect_init_view_page(kest_effect *effect, kest_ui_page *parent)
{
	KEST_PRINTF("effect_init_ui_page. effect = %p, parent = %p\n", effect, parent);
	if (!effect)
		return ERR_NULL_PTR;
	
	effect->view_page = kest_alloc(sizeof(kest_ui_page));
	
	if (!effect->view_page)
		return ERR_ALLOC_FAIL;
	
	init_ui_page(effect->view_page);
	init_effect_view(effect->view_page);
	configure_effect_view(effect->view_page, effect);
	effect->view_page->parent = parent;
	
	return NO_ERROR;
}

#endif

int clone_effect(kest_effect *dest, kest_effect *src)
{
	if (!src || !dest)
		return ERR_NULL_PTR;
	
	uint16_t preset_id;
	uint16_t effect_id;
	
	init_effect(dest);
	
	dest->type = src->type;
	dest->position = src->position;
	
	kest_parameter_pll *current_param = src->parameters;
	kest_parameter *param;
	
	while (current_param)
	{
		if(current_param->data)
		{
			param = effect_add_parameter(dest);
			
			if (param)
			{
				memcpy(param, current_param->data, sizeof(kest_parameter));
			}
			else
			{
				return ERR_ALLOC_FAIL;
			}
		}
		
		current_param = current_param->next;
	}
	
	setting_ll *current_setting = src->settings;
	kest_setting *setting;
	
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
	
	#ifdef KEST_ENABLE_UI
	src->view_page = NULL;
	#endif
	
	return NO_ERROR;
}


void gut_effect(kest_effect *effect)
{
	if (!effect)
		return;
	
	kest_parameter_pll_free(effect->parameters);
	kest_setting_pll_destroy(effect->settings, gut_setting);
	effect->parameters = NULL;
	effect->settings = NULL;
	
	gut_setting(&effect->band_mode);
	
	#ifdef KEST_ENABLE_UI
	free_effect_view(effect->view_page);
	effect->view_page = NULL;
	#endif
	
	effect->id 		 = 0;
	effect->type 	 = 0;
	effect->position = 0;
}


void free_effect(kest_effect *effect)
{
	if (!effect)
		return;
	
	kest_parameter_pll_destroy(effect->parameters, kest_parameter_free);
	kest_setting_pll_destroy(effect->settings, kest_setting_free);
	
	#ifdef KEST_ENABLE_UI
	free_effect_view(effect->view_page);
	#endif
	
	kest_free(effect);
}

kest_parameter *effect_get_parameter(kest_effect *effect, int n)
{
	if (!effect)
		return NULL;
	
	kest_parameter_pll *current = effect->parameters;
	int i = 0;
	
	while (current && i < n)
	{
		current = current->next;
		i++;
	}
	
	return current ? current->data : NULL;
}


kest_setting *effect_get_setting(kest_effect *effect, int n)
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

int kest_effect_update_fpga(kest_effect *effect)
{
	KEST_PRINTF("kest_effect_update_fpga(effect = %p, effect->eff = %p)\n", effect, effect ? effect->eff : NULL);
	
	if (!effect)
		return ERR_NULL_PTR;
	
	if (!effect->eff)
		return ERR_BAD_ARGS;
	
	kest_fpga_transfer_batch batch = kest_new_fpga_transfer_batch();
	
	if (!batch.buf)
		return ERR_ALLOC_FAIL;
	
	kest_scope_propagate_updates(effect->scope);
	
	kest_fpga_transfer_batch_append_effect_register_updates(&batch, effect->eff, effect->scope, effect->block_position);
	kest_fpga_transfer_batch_append_effect_resource_updates(&batch, effect->eff, effect->scope, &effect->position_);
	
	kest_scope_clear_updates(effect->scope);
	
	#ifdef KEST_ENABLE_FPGA
	int ret_val = kest_fpga_queue_transfer_batch(batch);
	
	return ret_val;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
}


kest_scope *kest_effect_create_scope(kest_effect *effect)
{
	if (!effect)
		return NULL;
	
	kest_scope *scope = kest_scope_new();
	
	if (!scope)
		return NULL;
	
	kest_parameter_pll *current_param = effect->parameters;
	kest_setting_pll *current_setting = effect->settings;
	kest_named_expression_pll *current_def_expr = effect->eff ? effect->eff->def_exprs : NULL;
	
	while (current_param)
	{
		if (current_param->data)
			kest_scope_add_param(scope, current_param->data);
		
		current_param = current_param->next;
	}
	
	while (current_setting)
	{
		if (current_setting->data)
			kest_scope_add_setting(scope, current_setting->data);
		
		current_setting = current_setting->next;
	}
	
	
	while (current_def_expr)
	{
		if (current_def_expr->data)
			kest_scope_add_expr(scope, current_def_expr->data->name, current_def_expr->data->expr);
		
		current_def_expr = current_def_expr->next;
	}
	
	return scope;
}

int kest_effect_set_parameter(kest_effect *effect, const char *name, float value)
{
	if (!effect)
		return ERR_NULL_PTR;
	
	int ret_val;
	
	kest_parameter_pll *current = effect->parameters;
	
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

int kest_effect_set_setting(kest_effect *effect, const char *name, int value)
{
	if (!effect)
		return ERR_NULL_PTR;
	
	int ret_val;
	
	kest_setting_pll *current = effect->settings;
	
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


void kest_effect_preset_rep_update(void *representer, void *representee)
{
	KEST_PRINTF("kest_effect_preset_rep_update\n");
	#ifdef KEST_ENABLE_REPRESENTATIONS
	kest_preset *preset = (kest_preset*)representer;
	kest_effect *effect = (kest_effect*)representee;
	
	if (!representee)
		return;
	
	if (!representee)
		representee = effect->preset;
	
	if (!representee)
		return;
	
	save_preset(preset);
	#endif
	
	KEST_PRINTF("kest_effect_preset_rep_update done\n");
	return;
}

void kest_effect_page_rep_update(void *representer, void *representee)
{
	KEST_PRINTF("kest_effect_preset_rep_update\n");
	#ifdef KEST_ENABLE_REPRESENTATIONS
	kest_ui_page *page = (kest_ui_page*)representer;
	kest_effect *effect = (kest_effect*)representee;
	
	if (!page || !effect)
		return;
	
	kest_effect_view_str *str = page->data_struct;
	
	if (!str)
		return;
	
	kest_parameter_pll *current_param = effect->parameters;
	kest_parameter *param;
	
	while (current_param)
	{
		param = current_param->data;
		
		if (param)
		{
			if (param->widget_rep.representee)
				kest_representation_queue_update(&param->widget_rep);
		}
		
		current_param = current_param->next;
	}
	
	#endif
	
	KEST_PRINTF("kest_effect_preset_rep_update done\n");
	return;
}

int kest_effect_update_reps(kest_effect *effect)
{
	KEST_PRINTF("kest_effect_update_reps(effect = %p)\n", effect);
	#ifdef KEST_ENABLE_REPRESENTATIONS
	if (!effect)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("effect->preset_rep = {.repr = %p, .repee = %p, .update = %p}\n",
		effect->preset_rep.representer, effect->preset_rep.representee, effect->preset_rep.update);
	
	kest_representation_pll *cr = effect->reps;
	
	int found = 0;
	while (cr) {
		if (cr->data == &effect->preset_rep)
		{
			KEST_PRINTF("Preset rep is in list\n");
			found = 1;
			break;
		}
	}
	
	if (!found)
	{
		KEST_PRINTF("Preset rep is not in list\n");
	}
	
	queue_representation_list_update(effect->reps);
	#endif
	
	KEST_PRINTF("kest_effect_update_reps done\n");
	return NO_ERROR;
}
