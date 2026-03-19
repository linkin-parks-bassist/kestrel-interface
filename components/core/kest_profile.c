#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_profile.c";

IMPLEMENT_LINKED_PTR_LIST(kest_profile);

static int next_preliminary_profile_id = 1;

int init_m_profile(kest_profile *profile)
{
	if (!profile)
		return ERR_NULL_PTR;
	
	profile->id = 0;
	
	int ret_val = init_m_pipeline(&profile->pipeline);
	
	#ifdef KEST_ENABLE_UI
	profile->view_page = NULL;
	#endif
	
	profile->name = NULL;
	profile->id = next_preliminary_profile_id++;
	
	profile->fname[0] = 0;
	profile->has_fname = 0;
	
	profile->active = 0;
	profile->unsaved_changes = 1;
	
	#ifdef KEST_ENABLE_SEQUENCES
	#ifdef KEST_ENABLE_GLOBAL_CONTEXT
	profile->sequence = &global_cxt.main_sequence;
	#else
	profile->sequence = NULL;
	#endif
	#endif
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	init_parameter(&profile->volume, "Gain", 0.0, -12.0, 12.0);
	profile->volume.units = " dB";
	profile->volume.id = (kest_parameter_id){.profile_id = 0, .effect_id = 0xFFFF, .parameter_id = 0};
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	profile->file_rep.representee = profile;
	profile->file_rep.representer = NULL;
	profile->file_rep.update = kest_profile_file_rep_update;
	profile->representations = NULL;
	kest_representation_pll_safe_append(&profile->representations, &profile->file_rep);
	#endif
	
	return NO_ERROR;
}

int profile_set_id(kest_profile *profile, uint16_t id)
{
	if (!profile)
		return ERR_NULL_PTR;
	
	profile->id = id;
	profile->volume.id.profile_id = id;
	
	return NO_ERROR;
}

int kest_profile_set_active(kest_profile *profile)
{
	if (!profile)
		return ERR_NULL_PTR;
	
	profile->active = 1;
	
	kest_profile_update_representations(profile);
	
	kest_profile_program_fpga(profile);
	
	return NO_ERROR;
}

int kest_profile_set_inactive(kest_profile *profile)
{
	if (!profile)
		return ERR_NULL_PTR;
	
	profile->active = 0;
	
	kest_profile_update_representations(profile);
	
	return NO_ERROR;
}

int kest_profile_add_representation(kest_profile *profile, kest_representation *rep)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	if (!profile || !rep)
		return ERR_NULL_PTR;
	
	kest_representation_pll *nl = kest_representation_pll_append(profile->representations, rep);
	
	if (nl)
		profile->representations = nl;
	else
		return ERR_ALLOC_FAIL;
	
	KEST_PRINTF("profile->representations = %p\n", profile->representations);
	
	return NO_ERROR;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
}

int kest_profile_update_representations(kest_profile *profile)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	if (!profile)
		return ERR_NULL_PTR;
	
	if (profile->representations)
		queue_representation_list_update(profile->representations);
	
	#endif
	return NO_ERROR;
}

int kest_profile_remove_representation(kest_profile *profile, kest_representation *rep)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	if (!profile)
		return ERR_NULL_PTR;
	
	profile->representations = kest_representation_pll_remove(profile->representations, rep);
	
	#endif
	return NO_ERROR;
}

int kest_profile_set_default_name_from_id(kest_profile *profile)
{
	KEST_PRINTF("kest_profile_set_default_name_from_id\n");
	if (!profile)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("ID = %d\n", profile->id);
	
	// Compute the digits in the ID. 
	int id_digits;
	int id_div = profile->id;
	
	for (id_digits = 0; id_div || !id_digits; id_div = id_div / 10)
		id_digits++;
	
	if (profile->name)
		kest_free(profile->name);
	
	profile->name = kest_alloc(9 + id_digits);
	
	if (!profile->name)
		return ERR_ALLOC_FAIL;
	
	sprintf(profile->name, "Profile %d", profile->id);
	
	KEST_PRINTF("Resulting name: %s\n", profile->name);
	
	return NO_ERROR;
}

kest_effect *kest_profile_append_effect_eff(kest_profile *profile, kest_effect_desc *eff)
{
	if (!profile)
		return NULL;
	
	KEST_PRINTF("kest_profile_append_effect_eff(profile = %p, eff = %p)\n", eff);
	kest_effect *effect = kest_pipeline_append_effect_eff(&profile->pipeline, eff);
	KEST_PRINTF("effect = %p\n", effect);
	
	if (!effect)
		return NULL;
	
	effect_rectify_param_ids(effect);
	
	return effect;
}

int kest_profile_remove_effect(kest_profile *profile, uint16_t id)
{
	KEST_PRINTF("kest_profile_remove_effect\n");
	if (!profile)
		return ERR_NULL_PTR;
	
	int ret_val = kest_pipeline_remove_effect(&profile->pipeline, id);
	
	kest_profile_if_active_update_fpga(profile);
	
	KEST_PRINTF("kest_profile_remove_effect done. ret_val = %s\n", kest_error_code_to_string(ret_val));
	return ret_val;
}

int kest_profile_move_effect(kest_profile *profile, int new_pos, int old_pos)
{
	int ret_val = NO_ERROR;
	
	if (profile)
	{
		if ((ret_val = kest_pipeline_move_effect(&profile->pipeline, new_pos, old_pos)) != NO_ERROR)
			return ret_val;
		
		ret_val = kest_profile_if_active_update_fpga(profile);
	}
	else
	{
		ret_val = ERR_NULL_PTR;
	}
	
	return ret_val;
}

int clone_profile(kest_profile *dest, kest_profile *src)
{
	if (!src || !dest)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("Cloning profile\n");
	
	KEST_PRINTF("Clone name...\n");
	dest->name = kest_strndup(src->name, PROFILE_NAME_MAX_LEN);
	clone_parameter(&dest->volume, &src->volume);
	dest->id = src->id;
	
	KEST_PRINTF("Clone pipeline...\n");
	clone_pipeline(&dest->pipeline, &src->pipeline);
	
	KEST_PRINTF("Done!\n");
	#ifdef KEST_ENABLE_UI
	dest->view_page = NULL;
	#endif
	
	return NO_ERROR;
}

void gut_profile(kest_profile *profile)
{
	if (!profile)
		return;
	
	#ifdef KEST_ENABLE_UI
	KEST_PRINTF("Gut view page %p...\n", profile->view_page);
	if (profile->view_page)
		profile->view_page->free_all(profile->view_page);
	
	profile->view_page = NULL;
	#endif
	
	KEST_PRINTF("Gut name %p...\n", profile->name);
	if (profile->name)
		kest_free(profile->name);
	
	profile->name = NULL;
	
	KEST_PRINTF("Gut profile...\n");
	gut_pipeline(&profile->pipeline);
	KEST_PRINTF("Done!\n");
}

void free_profile(kest_profile *profile)
{
	if (!profile)
		return;
		
	gut_profile(profile);
	
	kest_free(profile);
}

void kest_free_profile(kest_profile *profile)
{
	if (!profile)
		return;
		
	free_profile(profile);
	
	return;
}

#ifdef USE_TEENSY
void new_profile_receive_id(kest_message msg, kest_response response)
{
	kest_profile *profile = msg.cb_arg;
	
	if (!profile)
	{
		KEST_PRINTF("ERROR: Profile ID recieved, but no profile associated !\n");
		return;
	}
	
	uint16_t id;
	memcpy(&id, response.data, sizeof(uint16_t));
	
	KEST_PRINTF("New profile recieved its ID: %d\n", id);
	
	profile_set_id(profile, id);
	kest_profile_set_default_name_from_id(profile);
	
	
	kest_profile_update_representations(profile);
}
#endif

	
kest_profile *create_new_profile_with_teensy()
{
	#ifdef USE_TEENSY
	kest_profile *new_profile = kest_context_add_profile_rp(&global_cxt);
	
	if (!new_profile)
	{
		KEST_PRINTF("ERROR: Couldn't create new profile\n");
		return NULL;
	}
	
	kest_message msg = create_m_message_nodata(KEST_MESSAGE_CREATE_PROFILE);
	
	msg.callback = new_profile_receive_id;
	msg.cb_arg = new_profile;
	
	queue_msg_to_teensy(msg);
	
	create_profile_view_for(new_profile);

	return new_profile;
	#else
	return NULL;
	#endif
}

#ifdef KEST_ENABLE_GLOBAL_CONTEXT
kest_profile *create_new_profile()
{
	kest_profile *new_profile = kest_context_add_profile_rp(&global_cxt);
	
	if (!new_profile)
	{
		KEST_PRINTF("ERROR: Couldn't create new profile\n");
		return NULL;
	}
	
	#ifdef KEST_ENABLE_UI
	create_profile_view_for(new_profile);
	#endif

	return new_profile;
}
#endif

int kest_profile_save(kest_profile *profile)
{
	#ifdef KEST_ENABLE_SDCARD
	if (!profile)
		return ERR_NULL_PTR;
	
	int ret_val = save_profile(profile);
	
	if (ret_val == NO_ERROR)
	{
		profile->unsaved_changes = 0;
		#ifdef KEST_ENABLE_REPRESENTATIONS
		kest_profile_update_representations(profile);
		#endif
	}
	
	return NO_ERROR;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
}

int kest_profile_create_fpga_transfer_batch(kest_profile *profile, kest_fpga_transfer_batch *batch)
{
	if (!profile || !batch)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("kest_profile_create_fpga_transfer_batch(profile = %p, batch = %p)\n", profile, batch);
	
	int ret_val = kest_pipeline_create_fpga_transfer_batch(&profile->pipeline, batch);
	
	KEST_PRINTF("kest_profile_create_fpga_transfer_batch done (%s)\n", kest_error_code_to_string(ret_val));
	return ret_val;
}

int kest_profile_program_fpga(kest_profile *profile)
{
	if (!profile)
		return ERR_NULL_PTR;
	
	kest_fpga_transfer_batch batch;
	
	#ifndef KEST_LIBRARY
	KEST_PRINTF("kest_profile_program_fpga\n");
	int ret_val = kest_pipeline_create_fpga_transfer_batch(&profile->pipeline, &batch);
	
	KEST_PRINTF("kest_pipeline_create_fpga_transfer_batch returned with error code %s\n", kest_error_code_to_string(ret_val));
	if (ret_val != NO_ERROR)
		return ret_val;
	
	KEST_PRINTF("Queueing transfer batch...\n");
	if ((ret_val = kest_fpga_queue_program_batch(batch)) != NO_ERROR)
	{
		KEST_PRINTF("An error was encountered: %s\n", kest_error_code_to_string(ret_val));
		return ret_val;
	}
	#endif
	
	return NO_ERROR;
}

int kest_profile_if_active_update_fpga(kest_profile *profile)
{
	KEST_PRINTF("kest_profile_if_active_update_fpga");
	if (!profile)
		return ERR_NULL_PTR;
	
	if (!profile->active)
		return NO_ERROR;
	
	int ret_val = kest_profile_program_fpga(profile);
	
	return ret_val;
}

void kest_profile_file_rep_update(void *representer, void *representee)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	if (!representee)
		return;
	
	kest_profile *profile = (kest_profile*)representee;
	
	save_profile(profile);
	#endif
	return;
}


kest_effect *kest_profile_get_effect_by_id(kest_profile *profile, int id)
{
	kest_effect *result = kest_pipeline_get_effect_by_id(&profile->pipeline, id);
	
	return result;
}
