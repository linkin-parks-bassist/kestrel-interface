#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_sequence.c";

IMPLEMENT_LINKED_PTR_LIST(kest_sequence);

int init_m_sequence(kest_sequence *sequence)
{
	if (!sequence)
		return ERR_NULL_PTR;
	
	sequence->name 	  = NULL;
	sequence->profiles = NULL;
	
	sequence->active = 0;
	sequence->unsaved_changes = 1;
	sequence->position = NULL;
	
	#ifdef KEST_ENABLE_UI
	sequence->view_page = NULL;
	#endif
	
	sequence->fname[0] = 0;
	sequence->has_fname = 0;
	
	sequence->listings = NULL;
	
	sequence->main_sequence = 0;
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	sequence->file_rep.representee = sequence;
	sequence->file_rep.representer = NULL;
	sequence->file_rep.update = kest_sequence_file_rep_update;
	sequence->representations = NULL;
	kest_representation_pll_safe_append(&sequence->representations, &sequence->file_rep);
	#endif
	
	return NO_ERROR;
}

int sequence_append_profile(kest_sequence *sequence, kest_profile *profile)
{
	if (!sequence || !profile)
		return ERR_NULL_PTR;
	
	seq_profile_ll *new_node = kest_alloc(sizeof(seq_profile_ll));
	
	if (!new_node)
		return ERR_ALLOC_FAIL;
	
	new_node->data = profile;
	new_node->next = NULL;
	new_node->prev = NULL;
	
	if (!sequence->profiles)
	{
		sequence->profiles = new_node;
		return NO_ERROR;
	}
	
	seq_profile_ll *current = sequence->profiles;
	
	while (current)
	{
		if (!current->next)
			break;
		current = current->next;
	}
	
	current->next = new_node;
	new_node->prev = current;
	
	profile->sequence = sequence;
	
	return NO_ERROR;
}


seq_profile_ll *sequence_append_profile_rp(kest_sequence *sequence, kest_profile *profile)
{
	KEST_PRINTF("sequence_append_profile_rp, line %d\n", __LINE__);
	
	if (!sequence || !profile)
		return NULL;
	
	
	KEST_PRINTF("sequence_append_profile_rp, line %d\n", __LINE__);
	seq_profile_ll *new_node = kest_alloc(sizeof(seq_profile_ll));
	
	if (!new_node)
		return NULL;
	
	
	KEST_PRINTF("sequence_append_profile_rp, line %d\n", __LINE__);
	new_node->data = profile;
	new_node->next = NULL;
	new_node->prev = NULL;
	
	
	KEST_PRINTF("sequence_append_profile_rp, line %d\n", __LINE__);
	if (!sequence->profiles)
	{
		sequence->profiles = new_node;
		return new_node;
	}
	
	KEST_PRINTF("sequence_append_profile_rp, line %d\n", __LINE__);
	
	seq_profile_ll *current = sequence->profiles;
	
	
	KEST_PRINTF("sequence_append_profile_rp, line %d\n", __LINE__);
	while (current)
	{
		if (!current->next)
			break;
		current = current->next;
	}
	
	
	KEST_PRINTF("sequence_append_profile_rp, line %d\n", __LINE__);
	current->next = new_node;
	new_node->prev = current;
	
	
	KEST_PRINTF("sequence_append_profile_rp, line %d\n", __LINE__);
	kest_sequence_update_representations(sequence);
	
	KEST_PRINTF("sequence_append_profile_rp, line %d\n", __LINE__);
	
	return new_node;
}

int kest_sequence_move_profile(kest_sequence *sequence, int pos, int new_pos)
{
	if (!sequence)
		return ERR_NULL_PTR;
	
	if (pos < 0 || new_pos < 0)
		return ERR_BAD_ARGS;
	
	seq_profile_ll *target = sequence->profiles;
	seq_profile_ll *prev = NULL;
	
	for (int i = 0; i < pos; i++)
	{
		prev = target;
		if (target)
		{
			target = target->next;
		}
	}
	
	if (!target)
		return ERR_BAD_ARGS;
	
	if (prev)
	{
		prev->next = target->next;
	}
	
	if (new_pos == 0)
	{
		target->next = sequence->profiles;
		sequence->profiles = target;
		return NO_ERROR;
	}
	
	seq_profile_ll *current = sequence->profiles;
	
	for (int i = 0; i < new_pos; i++)
	{
		prev = current;
		if (current)
			current = current->next;
	}
	
	if (prev)
	{
		target->next = prev->next;
		prev->next = target;
	}
	else
	{
		return ERR_BAD_ARGS;
	}
	
	kest_sequence_update_representations(sequence);
	
	return NO_ERROR;
}

int kest_sequence_remove_profile(kest_sequence *sequence, kest_profile *profile)
{
	if (!sequence)
		return ERR_NULL_PTR;
	
	seq_profile_ll *current = sequence->profiles;
	
	while (current)
	{
		if (current->data == profile)
			break;
		
		current = current->next;
	}
	
	if (current)
	{
		if (current->prev)
			current->prev->next = current->next;
		else
			sequence->profiles = current->next;
		
		kest_free(current);
	}
	else
	{
		return ERR_BAD_ARGS;
	}
	
	kest_sequence_update_representations(sequence);
	
	return NO_ERROR;
}

int kest_sequence_delete_profile(kest_sequence *sequence, kest_profile *profile)
{
	if (!sequence)
		return ERR_NULL_PTR;
	
	int ret_val = kest_sequence_remove_profile(sequence, profile);
	
	if (profile && ret_val == NO_ERROR)
	{
		kest_free_profile(profile);
	}
	
	return NO_ERROR;
}

void free_sequence(kest_sequence *sequence)
{
	return;
}

int kest_sequence_add_menu_listing(kest_sequence *sequence, kest_menu_item *listing)
{
	#ifdef KEST_ENABLE_UI
	if (!sequence || !listing)
		return ERR_NULL_PTR;
	
	kest_menu_item_pll *nl = kest_menu_item_pll_append(sequence->listings, listing);
	
	if (nl)
		sequence->listings = nl;
	else
		return ERR_ALLOC_FAIL;
	
	return NO_ERROR;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
}

int kest_sequence_begin(kest_sequence *sequence)
{
	if (!sequence)
		return ERR_NULL_PTR;
	
	if (!sequence->profiles)
	{
		KEST_PRINTF("Sequence is empty !\n");
		return NO_ERROR;
	}
	
	global_cxt.sequence = sequence;
	sequence->active = 1;
	
	set_active_profile_from_sequence(sequence->profiles->data);
	
	sequence->position = sequence->profiles;
	
	kest_sequence_update_representations(sequence);
	
	return NO_ERROR;
}

int kest_sequence_begin_at(kest_sequence *sequence, kest_profile *profile)
{
	if (!sequence || !profile)
		return ERR_NULL_PTR;
	
	if (!sequence->profiles)
	{
		KEST_PRINTF("Sequence is empty !\n");
		return NO_ERROR;
	}
	
	global_cxt.sequence = sequence;
	sequence->active = 1;
	
	seq_profile_ll *current = sequence->profiles;
	int found = 0;
	
	while (current && !found)
	{
		if (current->data == profile)
			found = 1;
		else
			current = current->next;
	}
	
	if (!found)
		return ERR_BAD_ARGS;
	
	set_active_profile_from_sequence(current->data);
	
	sequence->position = current;
	
	kest_sequence_update_representations(sequence);
	
	return NO_ERROR;
}


int kest_sequence_regress(kest_sequence *sequence)
{
	if (!sequence)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("regressing sequence\n");
	
	if (!sequence->profiles)
	{
		KEST_PRINTF("Error: empty sequence\n");
		return ERR_BAD_ARGS;
	}
	
	if (!sequence->active || !sequence->position)
	{
		KEST_PRINTF("Error: sequence not active\n");
		return ERR_BAD_ARGS;
	}
	
	if (!sequence->position->prev)
	{
		KEST_PRINTF("Can't regress sequence; sequence at start already\n");
		return NO_ERROR;
	}
	
	sequence->position = sequence->position->prev;
	
	set_active_profile_from_sequence(sequence->position->data);
	
	return NO_ERROR;
}

int kest_sequence_advance(kest_sequence *sequence)
{
	if (!sequence)
		return ERR_NULL_PTR;
	
	KEST_PRINTF("advancing sequence. sequence = %p\n", sequence);
	
	if (!sequence->profiles)
	{
		KEST_PRINTF("Error: empty sequence\n");
		return ERR_BAD_ARGS;
	}
	
	KEST_PRINTF("Sequence nonempty\n");
	
	if (!sequence->active || !sequence->position)
	{
		KEST_PRINTF("Error: sequence not active\n");
		return ERR_BAD_ARGS;
	}
	
	KEST_PRINTF("Sequence active. Position: %p\n", sequence->position);
	
	if (!sequence->position->next)
	{
		KEST_PRINTF("Can't regress sequence; sequence at end already\n");
		return NO_ERROR;
	}
	
	KEST_PRINTF("sequence->position->next = %p\n", sequence->position->next);
	
	sequence->position = sequence->position->next;
	
	KEST_PRINTF("New sequence->position: %p. sequence->position->data: %p\n", 
		sequence->position, (sequence->position) ? sequence->position->data : NULL);
	
	return set_active_profile_from_sequence(sequence->position->data);
}

int kest_sequence_stop(kest_sequence *sequence)
{
	if (!sequence)
		return ERR_NULL_PTR;
	
	global_cxt.sequence = NULL;
	sequence->active = 0;
	
	set_active_profile_from_sequence(NULL);
	
	sequence->position = NULL;
	
	kest_sequence_update_representations(sequence);
	
	return NO_ERROR;
}


int kest_sequence_stop_from_profile(kest_sequence *sequence)
{
	if (!sequence)
		return ERR_NULL_PTR;
	
	global_cxt.sequence = NULL;
	sequence->active = 0;
	
	sequence->position = NULL;
	
	kest_sequence_update_representations(sequence);
	
	return NO_ERROR;
}

int kest_sequence_activate_at(kest_sequence *sequence, kest_profile *profile)
{
	KEST_PRINTF("kest_sequence_activate_at\n");
	if (!sequence)
		return ERR_NULL_PTR;
	
	seq_profile_ll *current = sequence->profiles;
	
	while (current)
	{
		if (current->data && current->data == profile)
		{
			sequence->position = current;
			sequence->active = 1;
			
			kest_sequence_update_representations(sequence);
			return NO_ERROR;
		}
		
		current = current->next;
	}
	
	KEST_PRINTF("kest_sequence_activate_at done\n");
	return ERR_BAD_ARGS;
}

int kest_sequence_add_representation(kest_sequence *sequence, kest_representation *rep)
{
	#ifdef KEST_ENABLE_UI
	if (!sequence)
		return ERR_NULL_PTR;
	
	kest_representation_pll *nl = kest_representation_pll_append(sequence->representations, rep);
	
	if (nl)
		sequence->representations = nl;
	else
		return ERR_ALLOC_FAIL;
	
	return NO_ERROR;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
}

int kest_sequence_update_representations(kest_sequence *sequence)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	if (!sequence)
		return ERR_NULL_PTR;
	
	kest_representation_pll *current = sequence->representations;
	
	if (!current)
	{
		KEST_PRINTF("Sequence %p has no representations.\n", sequence);
	}
	else
	{
		int i = 0;
		while (current)
		{
			if (current->data)
			{
				KEST_PRINTF("Seq %p rep %d: {.representer = %p, representee = %p, update = %p}\n",
					sequence, i, current->data->representer, current->data->representee, current->data->update);
			}
			
			current = current->next;
			i++;
		}
	}
	
	if (sequence->representations)
		queue_representation_list_update(sequence->representations);
	
	return NO_ERROR;
	#else
	return ERR_FEATURE_DISABLED;
	#endif
}


void kest_sequence_file_rep_update(void *representer, void *representee)
{
	if (!representee)
		return;
	
	kest_sequence *sequence = (kest_sequence*)representee;
	
	save_sequence(sequence);
	
	return;
}
