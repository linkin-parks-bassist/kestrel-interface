#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "m_profile.c";

IMPLEMENT_LINKED_PTR_LIST(m_profile);

static int next_preliminary_profile_id = 1;

int init_m_profile(m_profile *profile)
{
	if (!profile)
		return ERR_NULL_PTR;
	
	profile->id = 0;
	
	int ret_val = init_m_pipeline(&profile->pipeline);
	
	#ifdef M_ENABLE_UI
	profile->view_page = NULL;
	#endif
	
	profile->name = NULL;
	profile->id = next_preliminary_profile_id++;
	
	profile->fname[0] = 0;
	profile->has_fname = 0;
	
	profile->active = 0;
	profile->unsaved_changes = 1;
	
	#ifdef M_ENABLE_SEQUENCES
	#ifdef M_ENABLE_GLOBAL_CONTEXT
	profile->sequence = &global_cxt.main_sequence;
	#else
	profile->sequence = NULL;
	#endif
	#endif
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	init_parameter(&profile->volume, "Gain", 0.0, -12.0, 12.0);
	profile->volume.units = " dB";
	profile->volume.id = (m_parameter_id){.profile_id = 0, .transformer_id = 0xFFFF, .parameter_id = 0};
	
	#ifdef M_ENABLE_REPRESENTATIONS
	profile->file_rep.representee = profile;
	profile->file_rep.representer = NULL;
	profile->file_rep.update = m_profile_file_rep_update;
	profile->representations = NULL;
	m_representation_pll_safe_append(&profile->representations, &profile->file_rep);
	#endif
	
	return NO_ERROR;
}

int profile_set_id(m_profile *profile, uint16_t id)
{
	if (!profile)
		return ERR_NULL_PTR;
	
	profile->id = id;
	profile->volume.id.profile_id = id;
	
	return NO_ERROR;
}

int m_profile_set_active(m_profile *profile)
{
	if (!profile)
		return ERR_NULL_PTR;
	
	profile->active = 1;
	
	m_profile_update_representations(profile);
	
	m_profile_program_fpga(profile);
	
	return NO_ERROR;
}

int m_profile_set_inactive(m_profile *profile)
{
	if (!profile)
		return ERR_NULL_PTR;
	
	profile->active = 0;
	
	m_profile_update_representations(profile);
	
	return NO_ERROR;
}

int m_profile_add_representation(m_profile *profile, m_representation *rep)
{
	#ifdef M_ENABLE_REPRESENTATIONS
	if (!profile || !rep)
		return ERR_NULL_PTR;
	
	m_representation_pll *nl = m_representation_pll_append(profile->representations, rep);
	
	if (nl)
		profile->representations = nl;
	else
		return ERR_ALLOC_FAIL;
	
	M_PRINTF("profile->representations = %p\n", profile->representations);
	
	return NO_ERROR;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
}

int m_profile_update_representations(m_profile *profile)
{
	#ifdef M_ENABLE_REPRESENTATIONS
	if (!profile)
		return ERR_NULL_PTR;
	
	if (profile->representations)
		queue_representation_list_update(profile->representations);
	
	#endif
	return NO_ERROR;
}

int m_profile_remove_representation(m_profile *profile, m_representation *rep)
{
	#ifdef M_ENABLE_REPRESENTATIONS
	if (!profile)
		return ERR_NULL_PTR;
	
	profile->representations = m_representation_pll_remove(profile->representations, rep);
	
	#endif
	return NO_ERROR;
}

int m_profile_set_default_name_from_id(m_profile *profile)
{
	M_PRINTF("m_profile_set_default_name_from_id\n");
	if (!profile)
		return ERR_NULL_PTR;
	
	M_PRINTF("ID = %d\n", profile->id);
	
	// Compute the digits in the ID. 
	int id_digits;
	int id_div = profile->id;
	
	for (id_digits = 0; id_div || !id_digits; id_div = id_div / 10)
		id_digits++;
	
	if (profile->name)
		m_free(profile->name);
	
	profile->name = m_alloc(9 + id_digits);
	
	if (!profile->name)
		return ERR_ALLOC_FAIL;
	
	sprintf(profile->name, "Profile %d", profile->id);
	
	M_PRINTF("Resulting name: %s\n", profile->name);
	
	return NO_ERROR;
}

m_transformer *m_profile_append_transformer_eff(m_profile *profile, m_effect_desc *eff)
{
	if (!profile)
		return NULL;
	
	M_PRINTF("m_profile_append_transformer_eff(profile = %p, eff = %p)\n", eff);
	m_transformer *trans = m_pipeline_append_transformer_eff(&profile->pipeline, eff);
	M_PRINTF("trans = %p\n", trans);
	
	if (!trans)
		return NULL;
	
	transformer_rectify_param_ids(trans);
	
	return trans;
}

int m_profile_remove_transformer(m_profile *profile, uint16_t id)
{
	M_PRINTF("m_profile_remove_transformer\n");
	if (!profile)
		return ERR_NULL_PTR;
	
	int ret_val = m_pipeline_remove_transformer(&profile->pipeline, id);
	
	m_profile_if_active_update_fpga(profile);
	
	M_PRINTF("m_profile_remove_transformer done. ret_val = %s\n", m_error_code_to_string(ret_val));
	return ret_val;
}

int m_profile_move_transformer(m_profile *profile, int new_pos, int old_pos)
{
	int ret_val = NO_ERROR;
	
	if (profile)
	{
		if ((ret_val = m_pipeline_move_transformer(&profile->pipeline, new_pos, old_pos)) != NO_ERROR)
			return ret_val;
		
		ret_val = m_profile_if_active_update_fpga(profile);
	}
	else
	{
		ret_val = ERR_NULL_PTR;
	}
	
	return ret_val;
}

int clone_profile(m_profile *dest, m_profile *src)
{
	if (!src || !dest)
		return ERR_NULL_PTR;
	
	M_PRINTF("Cloning profile\n");
	
	M_PRINTF("Clone name...\n");
	dest->name = m_strndup(src->name, PROFILE_NAME_MAX_LEN);
	clone_parameter(&dest->volume, &src->volume);
	dest->id = src->id;
	
	M_PRINTF("Clone pipeline...\n");
	clone_pipeline(&dest->pipeline, &src->pipeline);
	
	M_PRINTF("Done!\n");
	#ifdef M_ENABLE_UI
	dest->view_page = NULL;
	#endif
	
	return NO_ERROR;
}

void gut_profile(m_profile *profile)
{
	if (!profile)
		return;
	
	#ifdef M_ENABLE_UI
	M_PRINTF("Gut view page %p...\n", profile->view_page);
	if (profile->view_page)
		profile->view_page->free_all(profile->view_page);
	
	profile->view_page = NULL;
	#endif
	
	M_PRINTF("Gut name %p...\n", profile->name);
	if (profile->name)
		m_free(profile->name);
	
	profile->name = NULL;
	
	M_PRINTF("Gut profile...\n");
	gut_pipeline(&profile->pipeline);
	M_PRINTF("Done!\n");
}

void free_profile(m_profile *profile)
{
	if (!profile)
		return;
		
	gut_profile(profile);
	
	m_free(profile);
}

void m_free_profile(m_profile *profile)
{
	if (!profile)
		return;
		
	free_profile(profile);
	
	return;
}

#ifdef USE_TEENSY
void new_profile_receive_id(m_message msg, m_response response)
{
	m_profile *profile = msg.cb_arg;
	
	if (!profile)
	{
		M_PRINTF("ERROR: Profile ID recieved, but no profile associated !\n");
		return;
	}
	
	uint16_t id;
	memcpy(&id, response.data, sizeof(uint16_t));
	
	M_PRINTF("New profile recieved its ID: %d\n", id);
	
	profile_set_id(profile, id);
	m_profile_set_default_name_from_id(profile);
	
	
	m_profile_update_representations(profile);
}
#endif

	
m_profile *create_new_profile_with_teensy()
{
	#ifdef USE_TEENSY
	m_profile *new_profile = m_context_add_profile_rp(&global_cxt);
	
	if (!new_profile)
	{
		M_PRINTF("ERROR: Couldn't create new profile\n");
		return NULL;
	}
	
	m_message msg = create_m_message_nodata(M_MESSAGE_CREATE_PROFILE);
	
	msg.callback = new_profile_receive_id;
	msg.cb_arg = new_profile;
	
	queue_msg_to_teensy(msg);
	
	create_profile_view_for(new_profile);

	return new_profile;
	#else
	return NULL;
	#endif
}

#ifdef M_ENABLE_GLOBAL_CONTEXT
m_profile *create_new_profile()
{
	m_profile *new_profile = m_context_add_profile_rp(&global_cxt);
	
	if (!new_profile)
	{
		M_PRINTF("ERROR: Couldn't create new profile\n");
		return NULL;
	}
	
	#ifdef M_ENABLE_UI
	create_profile_view_for(new_profile);
	#endif

	return new_profile;
}
#endif

int m_profile_save(m_profile *profile)
{
	#ifdef M_ENABLE_SDCARD
	if (!profile)
		return ERR_NULL_PTR;
	
	int ret_val = save_profile(profile);
	
	if (ret_val == NO_ERROR)
	{
		profile->unsaved_changes = 0;
		#ifdef M_ENABLE_REPRESENTATIONS
		m_profile_update_representations(profile);
		#endif
	}
	
	return NO_ERROR;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
}

int m_profile_create_fpga_transfer_batch(m_profile *profile, m_fpga_transfer_batch *batch)
{
	if (!profile || !batch)
		return ERR_NULL_PTR;
	
	M_PRINTF("m_profile_create_fpga_transfer_batch(profile = %p, batch = %p)\n", profile, batch);
	
	int ret_val = m_pipeline_create_fpga_transfer_batch(&profile->pipeline, batch);
	
	M_PRINTF("m_profile_create_fpga_transfer_batch done (%s)\n", m_error_code_to_string(ret_val));
	return ret_val;
}

int m_profile_program_fpga(m_profile *profile)
{
	if (!profile)
		return ERR_NULL_PTR;
	
	m_fpga_transfer_batch batch;
	
	#ifndef M_LIBRARY
	M_PRINTF("m_profile_program_fpga\n");
	int ret_val = m_pipeline_create_fpga_transfer_batch(&profile->pipeline, &batch);
	
	M_PRINTF("m_pipeline_create_fpga_transfer_batch returned with error code %s\n", m_error_code_to_string(ret_val));
	if (ret_val != NO_ERROR)
		return ret_val;
	
	M_PRINTF("Queueing transfer batch...\n");
	if ((ret_val = m_fpga_queue_program_batch(batch)) != NO_ERROR)
	{
		M_PRINTF("An error was encountered: %s\n", m_error_code_to_string(ret_val));
		return ret_val;
	}
	#endif
	
	return NO_ERROR;
}

int m_profile_if_active_update_fpga(m_profile *profile)
{
	M_PRINTF("m_profile_if_active_update_fpga");
	if (!profile)
		return ERR_NULL_PTR;
	
	if (!profile->active)
		return NO_ERROR;
	
	int ret_val = m_profile_program_fpga(profile);
	
	return ret_val;
}

void m_profile_file_rep_update(void *representer, void *representee)
{
	#ifdef M_ENABLE_REPRESENTATIONS
	if (!representee)
		return;
	
	m_profile *profile = (m_profile*)representee;
	
	save_profile(profile);
	#endif
	return;
}


m_transformer *m_profile_get_transformer_by_id(m_profile *profile, int id)
{
	m_transformer *result = m_pipeline_get_transformer_by_id(&profile->pipeline, id);
	
	return result;
}
