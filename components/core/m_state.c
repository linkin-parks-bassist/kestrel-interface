#include "m_int.h"

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "m_state.c";

#ifdef M_USE_FREERTOS
SemaphoreHandle_t state_mutex;
#endif

int m_cxt_clone_state(m_context *cxt, m_state *state)
{
	int len;
	
	if (!cxt)
		return ERR_NULL_PTR;
	
	if (!state)
		return ERR_BAD_ARGS;
	
	if (m_cxt_obtain_mutex(cxt) != NO_ERROR)
		return ERR_MUTEX_UNAVAILABLE;
	
	state->input_gain  = cxt->input_gain.value;
	state->output_gain = cxt->output_gain.value;
	
	m_ui_page_create_identifier(cxt->pages.current_page, &state->current_page);
	
	if (cxt->active_profile && cxt->active_profile->has_fname)
	{
		len = strlen(cxt->active_profile->fname);
		if (len > 31)
			len = 31;
		
		memcpy(state->active_profile_fname, cxt->active_profile->fname, len);
		state->active_profile_fname[len] = 0;
	}
	else
	{
		state->active_profile_fname[0] = 0;
	}
	
	if (cxt->sequence && cxt->sequence->has_fname)
	{
		len = strlen(cxt->sequence->fname);
		if (len > M_FILENAME_LEN - 1)
			len = M_FILENAME_LEN - 1;
		
		memcpy(state->active_sequence_fname, cxt->sequence->fname, len);
		state->active_sequence_fname[len] = 0;
	}
	else
	{
		state->active_sequence_fname[0] = 0;
	}
	
	m_cxt_release_mutex(cxt);
	
	return NO_ERROR;
}


void m_state_representation_update(void *representer, void *representee)
{
	M_PRINTF("m_state_representation_update\n");
	
	if (!representee)
		return;
	
	m_context *cxt = (m_context*)representee;
	m_state state;
	
	memset(&state, 0, sizeof(m_state));
	
	int ret_val;
	
	if ((ret_val = m_cxt_clone_state(cxt, &state)) != NO_ERROR)
	{
		M_PRINTF("Failed to clone state; aborting\n");
		return;
	}
	
	M_PRINTF("Cloned state\n");
	
	safe_file_write((int (*)(void*, const char*))save_state_to_file, &state, SETTINGS_FNAME);
	M_PRINTF("m_state_representation_update done\n");
	
	return;
}


int m_cxt_restore_state(m_context *cxt, m_state *state)
{
	M_PRINTF("m_cxt_restore_state\n");
	
	if (!cxt || !state)
		return ERR_NULL_PTR;
	
	m_profile *profile = NULL;
	m_sequence *sequence = NULL;
	m_ui_page *page;
	
	int ret_val = NO_ERROR;
	int tries = 0;
	
	profile = cxt_get_profile_by_fname(cxt, state->active_profile_fname);
	
	if (profile)
		set_active_profile_from_sequence(profile);
	
	m_cxt_set_input_gain (cxt, state->input_gain);
	m_cxt_set_output_gain(cxt, state->output_gain);
	
	M_PRINTF("m_cxt_restore_state done\n");
	return NO_ERROR;
}

int m_cxt_enter_previous_current_page(m_context *cxt, m_state *state)
{
	if (!cxt || !state)
		return ERR_NULL_PTR;
		
	m_ui_page *page = m_page_id_find_page(cxt, state->current_page);
	
	int ret_val = enter_ui_page(page);
	
	return ret_val;
}
