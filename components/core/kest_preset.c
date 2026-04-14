#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_preset.c";

IMPLEMENT_LINKED_PTR_LIST(kest_preset);

static int next_preliminary_preset_id = 1;

int init_m_preset(kest_preset *preset)
{
	if (!preset)
		return ERR_NULL_PTR;
	
	int ret_val = init_m_pipeline(&preset->pipeline);
	
	#ifdef KEST_ENABLE_UI
	preset->view_page = NULL;
	#endif
	
	preset->name = NULL;
	preset->id = next_preliminary_preset_id++;
	
	preset->fname[0] = 0;
	preset->has_fname = 0;
	
	preset->active = 0;
	preset->unsaved_changes = 1;
	
	#ifdef KEST_ENABLE_SEQUENCES
	#ifdef KEST_ENABLE_GLOBAL_CONTEXT
	preset->sequence = &global_cxt.main_sequence;
	#else
	preset->sequence = NULL;
	#endif
	#endif
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	init_parameter(&preset->volume, "Gain", 0.0, -12.0, 12.0);
	preset->volume.units = " dB";
	preset->volume.id = (kest_parameter_id){.preset_id = 0, .effect_id = 0xFFFF, .parameter_id = 0};
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	preset->file_rep.representee = preset;
	preset->file_rep.representer = NULL;
	preset->file_rep.update = kest_preset_file_rep_update;
	preset->representations = NULL;
	kest_representation_pll_safe_append(&preset->representations, &preset->file_rep);
	#endif
	
	return NO_ERROR;
}

int preset_set_id(kest_preset *preset, uint16_t id)
{
	if (!preset)
		return ERR_NULL_PTR;
	
	preset->id = id;
	preset->volume.id.preset_id = id;
	
	return NO_ERROR;
}

int kest_preset_rectify_ids(kest_preset *preset)
{
	if (!preset)
		return ERR_NULL_PTR;
	
	int id = preset->id;
	
	preset->volume.id.preset_id = id;
	
	kest_pipeline_rectify_ids(&preset->pipeline, id);
	
	return NO_ERROR;
}

int kest_preset_set_active(kest_preset *preset)
{
	if (!preset)
		return ERR_NULL_PTR;
	
	preset->active = 1;
	
	kest_preset_update_representations(preset);
	
	kest_preset_program_fpga(preset);
	
	return NO_ERROR;
}

int kest_preset_set_inactive(kest_preset *preset)
{
	if (!preset)
		return ERR_NULL_PTR;
	
	preset->active = 0;
	
	kest_preset_update_representations(preset);
	
	return NO_ERROR;
}

int kest_preset_add_representation(kest_preset *preset, kest_representation *rep)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	if (!preset || !rep)
		return ERR_NULL_PTR;
	
	kest_representation_pll *nl = kest_representation_pll_append(preset->representations, rep);
	
	if (nl)
		preset->representations = nl;
	else
		return ERR_ALLOC_FAIL;
	
	KEST_PRINTF("preset->representations = %p\n", preset->representations);
	
	return NO_ERROR;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
}

int kest_preset_update_representations(kest_preset *preset)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	if (!preset)
		return ERR_NULL_PTR;
	
	if (preset->representations)
		queue_representation_list_update(preset->representations);
	
	#endif
	return NO_ERROR;
}

int kest_preset_remove_representation(kest_preset *preset, kest_representation *rep)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	if (!preset)
		return ERR_NULL_PTR;
	
	preset->representations = kest_representation_pll_remove(preset->representations, rep);
	
	#endif
	return NO_ERROR;
}

int kest_preset_set_default_name_from_id(kest_preset *preset)
{
	KEST_PRINTF("kest_preset_set_default_name_from_id\n");
	if (!preset)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("ID = %d\n", preset->id);
	
	// Compute the digits in the ID. 
	int id_digits;
	int id_div = preset->id;
	
	for (id_digits = 0; id_div || !id_digits; id_div = id_div / 10)
		id_digits++;
	
	if (preset->name)
		kest_free(preset->name);
	
	preset->name = kest_alloc(9 + id_digits);
	
	if (!preset->name)
		return ERR_ALLOC_FAIL;
	
	sprintf(preset->name, "Preset %d", preset->id);
	
	KEST_PRINTF("Resulting name: %s\n", preset->name);
	
	return NO_ERROR;
}

kest_effect *kest_preset_append_effect_eff(kest_preset *preset, kest_effect_desc *eff)
{
	if (!preset)
		return NULL;
	
	KEST_PRINTF("kest_preset_append_effect_eff(preset = %p, eff = %p)\n", eff);
	kest_effect *effect = kest_pipeline_append_effect_eff(&preset->pipeline, eff);
	KEST_PRINTF("effect = %p\n", effect);
	
	if (!effect)
		return NULL;
	
	effect->preset = preset;
	#ifdef KEST_ENABLE_REPRESENTATIONS
	effect->preset_rep.representer = preset;
	#endif
	effect_rectify_param_ids(effect);
	
	return effect;
}

int kest_preset_remove_effect(kest_preset *preset, uint16_t id)
{
	KEST_PRINTF("kest_preset_remove_effect\n");
	if (!preset)
		return ERR_NULL_PTR;
	
	int ret_val = kest_pipeline_remove_effect(&preset->pipeline, id);
	
	kest_preset_if_active_update_fpga(preset);
	
	KEST_PRINTF("kest_preset_remove_effect done. ret_val = %s\n", kest_error_code_to_string(ret_val));
	return ret_val;
}

int kest_preset_move_effect(kest_preset *preset, int new_pos, int old_pos)
{
	int ret_val = NO_ERROR;
	
	if (preset)
	{
		if ((ret_val = kest_pipeline_move_effect(&preset->pipeline, new_pos, old_pos)) != NO_ERROR)
			return ret_val;
		
		ret_val = kest_preset_if_active_update_fpga(preset);
	}
	else
	{
		ret_val = ERR_NULL_PTR;
	}
	
	return ret_val;
}

int clone_preset(kest_preset *dest, kest_preset *src)
{
	if (!src || !dest)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("Cloning preset\n");
	
	KEST_PRINTF("Clone name...\n");
	dest->name = kest_strndup(src->name, PRESET_NAME_MAX_LEN);
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

void gut_preset(kest_preset *preset)
{
	if (!preset)
		return;
	
	#ifdef KEST_ENABLE_UI
	KEST_PRINTF("Gut view page %p...\n", preset->view_page);
	if (preset->view_page)
		preset->view_page->free_all(preset->view_page);
	
	preset->view_page = NULL;
	#endif
	
	KEST_PRINTF("Gut name %p...\n", preset->name);
	if (preset->name)
		kest_free(preset->name);
	
	preset->name = NULL;
	
	KEST_PRINTF("Gut preset...\n");
	gut_pipeline(&preset->pipeline);
	KEST_PRINTF("Done!\n");
}

void free_preset(kest_preset *preset)
{
	if (!preset)
		return;
		
	gut_preset(preset);
	
	kest_free(preset);
}

void kest_free_preset(kest_preset *preset)
{
	if (!preset)
		return;
		
	free_preset(preset);
	
	return;
}

#ifdef USE_TEENSY
void new_preset_receive_id(kest_message msg, kest_response response)
{
	kest_preset *preset = msg.cb_arg;
	
	if (!preset)
	{
		KEST_PRINTF("ERROR: Preset ID recieved, but no preset associated !\n");
		return;
	}
	
	uint16_t id;
	memcpy(&id, response.data, sizeof(uint16_t));
	
	KEST_PRINTF("New preset recieved its ID: %d\n", id);
	
	preset_set_id(preset, id);
	kest_preset_set_default_name_from_id(preset);
	
	
	kest_preset_update_representations(preset);
}
#endif

	
kest_preset *create_new_preset_with_teensy()
{
	#ifdef USE_TEENSY
	kest_preset *new_preset = kest_context_add_preset_rp(&global_cxt);
	
	if (!new_preset)
	{
		KEST_PRINTF("ERROR: Couldn't create new preset\n");
		return NULL;
	}
	
	kest_message msg = create_m_message_nodata(KEST_MESSAGE_CREATE_PRESET);
	
	msg.callback = new_preset_receive_id;
	msg.cb_arg = new_preset;
	
	queue_msg_to_teensy(msg);
	
	create_preset_view_for(new_preset);

	return new_preset;
	#else
	return NULL;
	#endif
}

#ifdef KEST_ENABLE_GLOBAL_CONTEXT
kest_preset *create_new_preset()
{
	kest_preset *new_preset = kest_context_add_preset_rp(&global_cxt);
	
	if (!new_preset)
	{
		KEST_PRINTF("ERROR: Couldn't create new preset\n");
		return NULL;
	}
	
	#ifdef KEST_ENABLE_UI
	create_preset_view_for(new_preset);
	#endif

	return new_preset;
}
#endif

int kest_preset_save(kest_preset *preset)
{
	#ifdef KEST_ENABLE_SDCARD
	if (!preset)
		return ERR_NULL_PTR;
	
	int ret_val = save_preset(preset);
	
	if (ret_val == NO_ERROR)
	{
		preset->unsaved_changes = 0;
		#ifdef KEST_ENABLE_REPRESENTATIONS
		kest_preset_update_representations(preset);
		#endif
	}
	
	return NO_ERROR;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
}

int kest_preset_create_fpga_transfer_batch(kest_preset *preset, kest_fpga_transfer_batch *batch)
{
	if (!preset || !batch)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("kest_preset_create_fpga_transfer_batch(preset = %p, batch = %p)\n", preset, batch);
	
	int ret_val = kest_pipeline_create_fpga_transfer_batch(&preset->pipeline, batch);
	
	KEST_PRINTF("kest_preset_create_fpga_transfer_batch done (%s)\n", kest_error_code_to_string(ret_val));
	return ret_val;
}

int kest_preset_program_fpga(kest_preset *preset)
{
	if (!preset)
		return ERR_NULL_PTR;
	
	kest_fpga_transfer_batch batch;
	
	#ifndef KEST_LIBRARY
	KEST_PRINTF("kest_preset_program_fpga\n");
	int ret_val = kest_pipeline_create_fpga_transfer_batch(&preset->pipeline, &batch);
	
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

int kest_preset_if_active_update_fpga(kest_preset *preset)
{
	KEST_PRINTF("kest_preset_if_active_update_fpga(preset = %p)\n", preset);
	if (!preset)
		return ERR_NULL_PTR;
	
	if (!preset->active)
		return NO_ERROR;
	
	int ret_val = kest_preset_program_fpga(preset);
	
	return ret_val;
}

void kest_preset_file_rep_update(void *representer, void *representee)
{
	KEST_PRINTF("kest_preset_file_rep_update\n");
	#ifdef KEST_ENABLE_REPRESENTATIONS
	if (!representee)
		return;
	
	kest_preset *preset = (kest_preset*)representee;
	
	save_preset(preset);
	#endif
	KEST_PRINTF("kest_preset_file_rep_update done\n");
	return;
}


kest_effect *kest_preset_get_effect_by_id(kest_preset *preset, int id)
{
	kest_effect *result = kest_pipeline_get_effect_by_id(&preset->pipeline, id);
	
	return result;
}
