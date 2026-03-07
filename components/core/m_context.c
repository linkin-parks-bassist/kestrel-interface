#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

#define INITIAL_PROFILE_ARRAY_LENGTH 8
#define PROFILE_ARRAY_CHUNK_SIZE	 8

static const char *FNAME = "m_context.c";

int m_init_context(m_context *cxt)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	cxt->n_profiles = 0;
	
	cxt->active_profile  = NULL;
	cxt->working_profile = NULL;
	
	cxt->profiles  = NULL;
	cxt->sequences = NULL;
	cxt->sequence  = NULL;
	
	m_context_init_main_sequence(cxt);
	
	cxt->main_sequence.name = "Profiles";
	
	cxt->saved_profiles_loaded  = 0;
	cxt->saved_sequences_loaded = 0;
	
	cxt->pages.backstage = NULL;
	cxt->pages.current_page = NULL;
	
	cxt->effects = NULL;
	
	init_parameter(&cxt->input_gain, "Input Gain", -100, -30.0, 30.0);
	cxt->input_gain.units = " dB";
	cxt->input_gain.id = (m_parameter_id){.profile_id = CONTEXT_PROFILE_ID, .transformer_id = 0, .parameter_id = INPUT_GAIN_PID};
	cxt->input_gain.max_velocity = 0.4;
	cxt->input_gain.min_expr = &m_expression_standard_gain_min;
	cxt->input_gain.max_expr = &m_expression_standard_gain_max;
	
	init_parameter(&cxt->output_gain, "Output Gain", -100, -30.0, 30.0);
	cxt->output_gain.units = " dB";
	cxt->output_gain.id = (m_parameter_id){.profile_id = CONTEXT_PROFILE_ID, .transformer_id = 0, .parameter_id = OUTPUT_GAIN_PID};
	cxt->output_gain.max_velocity = 0.4;
	cxt->output_gain.min_expr = &m_expression_standard_gain_min;
	cxt->output_gain.max_expr = &m_expression_standard_gain_max;
	
	#ifdef M_ENABLE_REPRESENTATIONS
	cxt->state_rep.representer = NULL;
	cxt->state_rep.representee = cxt;
	cxt->state_rep.update = m_state_representation_update;
	cxt->state_rep_lstub.data = &cxt->state_rep;
	cxt->state_rep_lstub.next = NULL;
	#endif
	
	#ifdef M_USE_FREERTOS
	cxt->mutex = xSemaphoreCreateMutex();
	#endif
	
	return NO_ERROR;
}

int m_context_init_main_sequence(m_context *cxt)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	init_m_sequence(&cxt->main_sequence);
	
	cxt->main_sequence.name = "Profiles";
	cxt->main_sequence.view_page = &cxt->pages.main_sequence_view;
	
	for (int k = 0; k < M_FILENAME_LEN; k++)
	{
		if (!MAIN_SEQUENCE_FNAME[k])
		{
			cxt->main_sequence.fname[k] = 0;
			break;
		}
		
		cxt->main_sequence.fname[k] = MAIN_SEQUENCE_FNAME[k];
	}
	
	cxt->main_sequence.has_fname = 1;
	cxt->main_sequence.main_sequence = 1;
	
	return NO_ERROR;
}

int m_context_init_effect_list(m_context *cxt)
{
	M_PRINTF("m_context_init_effect_list\n");
	if (!cxt)
		return ERR_NULL_PTR;
	
	return NO_ERROR;
}

int m_context_init_ui(m_context *cxt)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	cxt->pages.backstage = NULL;
	cxt->pages.current_page = NULL;
	
	return NO_ERROR;
}

int m_context_add_profile(m_context *cxt)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	m_profile *profile = m_alloc(sizeof(m_profile));
	
	if (!profile)
		return ERR_ALLOC_FAIL;
	
	init_m_profile(profile);
	
	profile_ll *nl = m_profile_pll_append(cxt->profiles, profile);
	
	if (!nl)
	{
		free_profile(profile);
		return ERR_ALLOC_FAIL;
	}
	
	cxt->profiles = nl;
	
	cxt->n_profiles++;
	
	return NO_ERROR;
}

m_profile *m_context_add_profile_rp(m_context *cxt)
{
	if (!cxt)
		return NULL;
	
	M_PRINTF("m_context_add_profile_rp\n");
	
	m_profile *profile = m_alloc(sizeof(m_profile));
	
	if (!profile)
		return NULL;
	
	M_PRINTF("profile = %p\n", profile);
	
	init_m_profile(profile);
	
	M_PRINTF("profile->name = %p\n", profile->name);
	M_PRINTF("\t\t\t= %s\n", profile->name ? profile->name : "(NULL)");
	
	profile_ll *nl = m_profile_pll_append(cxt->profiles, profile);
	
	if (!nl)
	{
		free_profile(profile);
		return NULL;
	}
	
	cxt->profiles = nl;
	
	cxt->n_profiles++;
	
	return profile;
}

m_sequence *m_context_add_sequence_rp(m_context *cxt)
{
	if (!cxt)
		return NULL;
	
	m_sequence *sequence = m_alloc(sizeof(m_sequence));
	
	if (!sequence)
		return NULL;
	
	init_m_sequence(sequence);
	
	sequence_ll *nl = m_sequence_pll_append(cxt->sequences, sequence);
	
	if (!nl)
	{
		free_sequence(sequence);
		return NULL;
	}
	
	cxt->sequences = nl;
	
	return sequence;
}

m_profile *cxt_get_profile_by_id(m_context *cxt, uint16_t profile_id)
{
	if (!cxt)
		return NULL;
	
	profile_ll *current = cxt->profiles;
	
	while (current)
	{
		if (current->data && current->data->id == profile_id)
		{
			return current->data;
		}
		
		current = current->next;
	}
	
	return NULL;
}

m_transformer *cxt_get_transformer_by_id(m_context *cxt, uint16_t profile_id, uint16_t transformer_id)
{
	if (!cxt)
		return NULL;
	
	m_profile *profile = cxt_get_profile_by_id(cxt, profile_id);
	
	if (!profile)
		return NULL;
	
	
	m_transformer_pll *current = profile->pipeline.transformers;
	
	while (current)
	{
		if (current->data && current->data->id == transformer_id)
		{
			return current->data;
		}
		
		current = current->next;
	}
	
	return NULL;
}

m_parameter *cxt_get_parameter_by_id(m_context *cxt, uint16_t profile_id, uint16_t transformer_id, uint16_t parameter_id)
{
	if (!cxt)
		return NULL;
	
	m_transformer *trans = cxt_get_transformer_by_id(cxt, profile_id, transformer_id);
	
	if (!trans)
		return NULL;
	
	return transformer_get_parameter(trans, parameter_id);
}	

int cxt_get_parameter_and_transformer_by_id(m_context *cxt, m_parameter_id id, m_parameter **pp, m_transformer **tp)
{
	if (!cxt || !pp || !tp)
		return ERR_NULL_PTR;
	
	*pp = NULL;
	*tp = NULL;
	
	if (id.profile_id == CONTEXT_PROFILE_ID)
	{
		if (id.transformer_id == 0)
		{
			switch (id.parameter_id)
			{
				case INPUT_GAIN_PID:
					*pp = &global_cxt.input_gain;
					return NO_ERROR;
					
				case OUTPUT_GAIN_PID:
					*pp = &global_cxt.output_gain;
					return NO_ERROR;
				
				default:
					return ERR_BAD_ARGS;
			}
		}
		
		return ERR_BAD_ARGS;
	}
	
	m_transformer *trans = cxt_get_transformer_by_id(cxt, id.profile_id, id.transformer_id);
	
	
	if (!trans)
	{
		return ERR_BAD_ARGS;
	}

	m_parameter *param = transformer_get_parameter(trans, id.parameter_id);
	
	if (!param)
	{
		return ERR_BAD_ARGS;
	}
	
	*pp = param;
	*tp = trans;
	
	return NO_ERROR;
}

m_setting *cxt_get_setting_by_id(m_context *cxt, uint16_t profile_id, uint16_t transformer_id, uint16_t parameter_id)
{
	if (!cxt)
		return NULL;
	
	m_transformer *trans = cxt_get_transformer_by_id(cxt, profile_id, transformer_id);
	
	if (!trans)
		return NULL;
	
	return transformer_get_setting(trans, parameter_id);
}

int cxt_transformer_id_to_position(m_context *cxt, uint16_t profile_id, uint16_t transformer_id)
{
	if (!cxt)
		return -ERR_NULL_PTR;
	
	m_profile *profile = cxt_get_profile_by_id(cxt, profile_id);
	
	if (!profile)
		return -ERR_INVALID_PROFILE_ID;
	
	m_transformer_pll *current = profile->pipeline.transformers;
	
	int i = 0;
	while (current)
	{
		if (current->data && current->data->id == transformer_id)
			return i;
		
		current = current->next;
		i++;
	}
	
	return -ERR_INVALID_TRANSFORMER_ID;
}

int cxt_transformer_position_to_id(m_context *cxt, uint16_t profile_id, uint16_t transformer_pos)
{
	if (!cxt)
		return -ERR_NULL_PTR;
	
	m_profile *profile = cxt_get_profile_by_id(cxt, profile_id);
	
	if (!profile)
		return -ERR_INVALID_PROFILE_ID;
	
	m_transformer_pll *current = profile->pipeline.transformers;
	
	int i = 0;
	while (current)
	{
		if (i == transformer_pos)
		{
			if (!current->data)
				return ERR_NULL_PTR;
			
			return current->data->id;
		}
		
		current = current->next;
		i++;
	}
	
	return -ERR_INVALID_TRANSFORMER_ID;
}

int cxt_remove_profile(m_context *cxt, m_profile *profile)
{
	if (!cxt || !profile)
		return ERR_NULL_PTR;
	
	profile_ll *current = cxt->profiles;
	profile_ll *prev = NULL;
	
	if (profile && profile->has_fname)
		remove(profile->fname);
	
	while (current)
	{
		if (current->data == profile)
		{
			#ifdef USE_TEENSY
			queue_msg_to_teensy(create_m_message(M_MESSAGE_DELETE_PROFILE, "s", profile->id));
			#endif
			
			if (!prev)
			{
				cxt->profiles = current->next;
			}
			else
			{
				prev->next = current->next;
			}
			
			free_profile(profile);
			m_free(current);
			
			return NO_ERROR;
		}
		
		prev = current;
		current = current->next;
	}
	
	return ERR_INVALID_PROFILE_ID;
}

int cxt_remove_sequence(m_context *cxt, m_sequence *sequence)
{
	if (!cxt || !sequence)
		return ERR_NULL_PTR;
	
	sequence_ll *current = cxt->sequences;
	sequence_ll *prev = NULL;
	
	if (sequence && sequence->has_fname)
		remove(sequence->fname);
	
	while (current)
	{
		if (current->data == sequence)
		{
			if (!prev)
			{
				cxt->sequences = current->next;
			}
			else
			{
				prev->next = current->next;
			}
			
			free_sequence(sequence);
			m_free(current);
			
			return NO_ERROR;
		}
		
		prev = current;
		current = current->next;
	}
	
	return ERR_INVALID_PROFILE_ID;
}

int cxt_remove_transformer(m_context *cxt, uint16_t pid, uint16_t tid)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	int ret_val = m_profile_remove_transformer(cxt_get_profile_by_id(cxt, pid), tid);
	
	if (ret_val == NO_ERROR)
	{
		#ifdef USE_TEENSY
		queue_msg_to_teensy(create_m_message(M_MESSAGE_REMOVE_TRANSFORMER, "ss", pid, tid));
		#endif
	}
	
	return ret_val;
}

int set_active_profile(m_profile *profile)
{
	if (profile)
		m_profile_set_active(profile);
	
	if (profile == global_cxt.active_profile)
		return NO_ERROR;
	
	if (profile && profile->sequence)
	{
		M_PRINTF("profile has a sequence. call m_sequence_activate_at\n");
		m_sequence_activate_at(profile->sequence, profile);
	}
	
	m_profile_set_inactive(global_cxt.active_profile);
	
	global_cxt.active_profile = profile;
	
	uint16_t id = profile ? profile->id : 0;
	
	#ifdef USE_TEENSY
	int ret_val = queue_msg_to_teensy(create_m_message(M_MESSAGE_SWITCH_PROFILE, "s", id));
	#endif
	
	return NO_ERROR;
}

// This version is called from a sequence-related-cb, so there is no need to
// tell the sequence about it; it is handled from the caller
int set_active_profile_from_sequence(m_profile *profile)
{
	M_PRINTF("set_active_profile_from_sequence, profile = %p\n", profile);
	if (profile)
		m_profile_set_active(profile);
	
	if (profile == global_cxt.active_profile)
		return NO_ERROR;
	
	m_profile_set_inactive(global_cxt.active_profile);
	
	global_cxt.active_profile = profile;
	
	int ret_val = NO_ERROR;
	#ifdef USE_TEENSY
	uint16_t id = profile ? profile->id : 0;
	ret_val = queue_msg_to_teensy(create_m_message(M_MESSAGE_SWITCH_PROFILE, "s", id));
	#endif
	
	return ret_val;
}

int set_working_profile(m_profile *profile)
{
	if (!profile)
		return ERR_NULL_PTR;
	
	if (profile == global_cxt.working_profile)
		return NO_ERROR;
	
	global_cxt.working_profile = profile;
	
	return NO_ERROR;
}

void context_print_profiles(m_context *cxt)
{
	if (!cxt)
		return;
		
	M_PRINTF("Printing profiles...\n");
	
	profile_ll *current = global_cxt.profiles;
	
	
	int i = 0;
	while (current)
	{
		M_PRINTF("Profile %d, stored at %p, ", i, current->data);
		
		if (current->data)
		{
			int j = 0;
			m_transformer_pll *ct = current->data->pipeline.transformers;
			
			while (ct)
			{
				ct = ct->next;
				j++;
			}
			M_PRINTF("has name %s, and has %d transformers%s", current->data->name ? current->data->name : "(NULL)", j, (j > 0) ? ", which are\n" : "\n\n");
			
			ct = current->data->pipeline.transformers;
			
			while (ct)
			{
				M_PRINTF("\t%s,\n", (ct->data && m_transformer_name(ct->data)) ? m_transformer_name(ct->data) : "UNKNOWN");
				ct = ct->next;
			}
		}
		current = current->next;
		i++;
	}
	
	if (i == 0)
	{
		M_PRINTF("There are none!\n");
	}
}

int cxt_set_all_profiles_left_button_to_main_menu(m_context *cxt)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	profile_ll *current = cxt->profiles;
	
	while (current)
	{
		if (current->data && current->data->view_page)
		{
			//profile_view_set_left_button_mode(current->data->view_page, LEFT_BUTTON_MENU);
		}
		
		current = current->next;
	}
	
	return NO_ERROR;
}

int cxt_handle_hw_switch(m_context *cxt, int sw)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	M_PRINTF("cxt_handle_hw_switch, sw = %d\n", sw);
	
	if (cxt->sequence)
	{
		if (sw == 0)
			m_sequence_regress(cxt->sequence);
		else if (sw == 1)
			m_sequence_advance(cxt->sequence);
	}
	
	return NO_ERROR;
}

m_profile *cxt_find_profile(m_context *cxt, const char *fname)
{
	if (!cxt)
		return NULL;
	
	profile_ll *current = cxt->profiles;
	
	M_PRINTF("Searching for profile with fname %s...\n", fname);
	while (current)
	{
		if (current->data && current->data->has_fname)
		{
			M_PRINTF("Check %s\n", current->data->fname);
			if (strncmp(current->data->fname, fname, PROFILE_NAME_MAX_LEN) == 0)
			{
				M_PRINTF("Match!\n");
				return current->data;
			}
			M_PRINTF("No match\n");
		}
		
		current = current->next;
	}
	
	return NULL;
}

int cxt_save_all_profiles(m_context *cxt)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	profile_ll *current = cxt->profiles;
	
	while (current)
	{
		if (current->data)
		{
			m_profile_save(current->data);
		}
		
		current = current->next;
	}
	
	return NO_ERROR;
}

int m_cxt_obtain_mutex(m_context *cxt)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	#ifdef M_USE_FREERTOS
	if (xSemaphoreTake(cxt->mutex, 0) != pdTRUE) return ERR_MUTEX_UNAVAILABLE;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
	
	return NO_ERROR;
}

int m_cxt_obtain_mutex_wait_forever(m_context *cxt)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	#ifdef M_USE_FREERTOS
	if (xSemaphoreTake(cxt->mutex, portMAX_DELAY) != pdTRUE)
		return ERR_MUTEX_UNAVAILABLE;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
	
	return NO_ERROR;
}


int m_cxt_release_mutex(m_context *cxt)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	#ifdef M_USE_FREERTOS
	xSemaphoreGive(cxt->mutex);
	#else
	return ERR_FEATURE_DISABLED;
	#endif
	
	return NO_ERROR;
}

int m_cxt_queue_save_state(m_context *cxt)
{
	if (!cxt)
		return ERR_NULL_PTR;
	
	queue_representation_list_update(&cxt->state_rep_lstub);
	
	return NO_ERROR;
}

m_profile *cxt_get_profile_by_fname(m_context *cxt, const char *fname)
{
	M_PRINTF("cxt_get_profile_by_fname(cxt = %p, fname = %s)\n", cxt, fname ? fname : "(NULL)");
	
	if (!cxt || !fname)
		return NULL;
	
	m_profile_pll *current = cxt->profiles;
	
	M_PRINTF("Searching the list. %s\n", !current ? "... but it is empty!" : "");
	int i = 0;
	while (current)
	{
		M_PRINTF("Profile %d ", i);
		if (current->data)
		{
			M_PRINTF("has fname \"%s\".\n", current->data->has_fname ? current->data->fname : "(NULL)");
		}
		else
		{
			M_PRINTF("... doesn't exist???\n");
		}
		if (current->data && current->data->has_fname && fnames_agree(current->data->fname, fname))
		{
			M_PRINTF("That's the one :) returning it\n");
			return current->data;
		}
		
		current = current->next;
		i++;
	}
	
	return NULL;
}

m_sequence *cxt_get_sequence_by_fname(m_context *cxt, const char *fname)
{
	if (!cxt || !fname)
		return NULL;
	
	m_sequence_pll *current = cxt->sequences;
	
	while (current)
	{
		if (current->data && current->data->has_fname && fnames_agree(current->data->fname, fname))
			return current->data;
		
		current = current->next;
	}
	
	return NULL;
}

int m_cxt_set_input_gain(m_context *cxt, float gain)
{
	M_PRINTF("m_cxt_set_input_gain\n");
	
	if (!cxt) return ERR_NULL_PTR;
	
	int ret_val = m_parameter_trigger_update(&cxt->input_gain, gain);
	
	M_PRINTF("m_cxt_set_input_gain done\n");
	return ret_val;
}

int m_cxt_set_output_gain(m_context *cxt, float gain)
{
	M_PRINTF("m_cxt_set_output_gain\n");
	if (!cxt) return ERR_NULL_PTR;
	
	int ret_val = m_parameter_trigger_update(&cxt->output_gain, gain);
	
	M_PRINTF("m_cxt_set_input_gain done\n");
	return ret_val;
}

m_effect_desc *m_cxt_get_effect_desc_from_cname(m_context *cxt, const char *cname)
{
	if (!cxt || !cname)
		return NULL;
	
	m_effect_desc_pll *current = cxt->effects;
	
	while (current)
	{
		if (current->data && current->data->cname && strcmp(current->data->cname, cname) == 0)
			return current->data;
		
		current = current->next;
	}
	
	return NULL;
}
