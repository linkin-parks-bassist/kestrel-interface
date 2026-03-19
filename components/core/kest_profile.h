#ifndef KEST_PROFILE_H_
#define KEST_PROFILE_H_

#define PROFILE_NAME_MAX_LEN 128

#define KEST_PROFILE_MUTEX_TIMEOUT_MS 10


struct kest_glide_button_pll;
struct kest_menu_item_pll;

struct kest_sequence;

typedef struct kest_profile
{
	char *name;
	char fname[KEST_FILENAME_LEN];
	uint16_t id;
	
	int has_fname;
	kest_pipeline pipeline;
	
	#ifdef KEST_ENABLE_SEQUENCES
	struct kest_sequence *sequence;
	#endif
	
	int active;
	int unsaved_changes;
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	kest_representation_pll *representations;
	kest_representation file_rep;
	#endif
	
	#ifdef KEST_ENABLE_UI
	struct kest_ui_page *view_page;
	#endif
	
	kest_parameter volume;
	
	#ifdef KEST_USE_FREERTOS
	SemaphoreHandle_t mutex;
	#endif
} kest_profile;

DECLARE_LINKED_PTR_LIST(kest_profile);

int init_m_profile(kest_profile *profile);
int init_m_pipeline(kest_pipeline *pipeline);

int profile_set_id(kest_profile *profile, uint16_t id);

int kest_profile_set_default_name_from_id(kest_profile *profile);

kest_effect *kest_profile_append_effect_eff(kest_profile *profile, kest_effect_desc *eff);
int kest_profile_remove_effect(kest_profile *profile, uint16_t id);

int kest_profile_move_effect(kest_profile *profile, int new_pos, int old_pos);

int clone_profile(kest_profile *dest, kest_profile *src);
void gut_profile(kest_profile *profile);
void free_profile(kest_profile *profile);
void kest_free_profile(kest_profile *profile);

int kest_profile_set_active(kest_profile *profile);
int kest_profile_set_inactive(kest_profile *profile);

struct kest_menu_item;

int kest_profile_add_representation(kest_profile *profile, kest_representation *rep);
int kest_profile_remove_representation(kest_profile *profile, kest_representation *rep);

int kest_profile_add_name_representation(kest_profile *profile, kest_representation *rep);
int kest_profile_remove_name_representation(kest_profile *profile, kest_representation *rep);

int kest_profile_add_id_representation(kest_profile *profile, kest_representation *rep);
int kest_profile_remove_id_representation(kest_profile *profile, kest_representation *rep);

#ifdef KEST_ENABLE_GLOBAL_CONTEXT
kest_profile *create_new_profile();
#endif

int kest_profile_save(kest_profile *profile);

int kest_profile_update_representations		(kest_profile *profile);
int kest_profile_update_name_representations	(kest_profile *profile);
int kest_profile_update_id_representations		(kest_profile *profile);

int kest_profile_create_fpga_transfer_batch(kest_profile *profile, kest_fpga_transfer_batch *batch);
int kest_profile_if_active_update_fpga(kest_profile *profile);
int kest_profile_program_fpga(kest_profile *profile);

void kest_profile_file_rep_update(void *representer, void *representee);

kest_effect *kest_profile_get_effect_by_id(kest_profile *profile, int id);

#endif
