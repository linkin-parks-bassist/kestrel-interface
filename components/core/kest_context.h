#ifndef KEST_ESP32_CONTEXT_H_
#define KEST_ESP32_CONTEXT_H_

#define MAX_N_PROFILES 256

#define CONTEXT_PROFILE_ID  0xFFFF
#define INPUT_GAIN_PID		0x0000
#define OUTPUT_GAIN_PID		0x0001

typedef kest_profile_pll profile_ll;

typedef struct kest_context
{
	int n_profiles;
	int active_profile_id;
	
	kest_profile *working_profile;
	kest_profile *active_profile;
	kest_profile *default_profile;
	
	#ifdef KEST_ENABLE_UI
	kest_global_pages pages;
	#endif
	
	profile_ll  *profiles;
	sequence_ll *sequences;
	
	kest_sequence main_sequence;
	kest_sequence *sequence;
	
	int saved_profiles_loaded;
	int saved_sequences_loaded;
	
	kest_effect_desc_pll *effects;
	
	kest_parameter input_gain;
	kest_parameter output_gain;
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	kest_representation_pll state_rep_lstub;
	kest_representation state_rep;
	#endif
	
	#ifdef KEST_USE_FREERTOS
	SemaphoreHandle_t mutex;
	#endif
} kest_context;

#ifdef KEST_ENABLE_GLOBAL_CONTEXT
extern kest_context global_cxt;
#endif

int kest_init_context(kest_context *cxt);
int kest_context_init_effect_list(kest_context *cxt);
int kest_context_init_main_sequence(kest_context *cxt);
int kest_context_init_ui(kest_context *cxt);

int kest_context_enlarge_profile_array(kest_context *cxt);
int kest_context_set_n_profiles(kest_context *cxt, int n);
int kest_context_add_profile(kest_context *cxt);
kest_profile *kest_context_add_profile_rp(kest_context *cxt);

kest_sequence *kest_context_add_sequence_rp(kest_context *cxt);

kest_profile *cxt_get_profile_by_id(kest_context *cxt, uint16_t profile_id);
kest_effect *cxt_get_effect_by_id(kest_context *cxt, uint16_t profile_id, uint16_t effect_id);
kest_parameter *cxt_get_parameter_by_id(kest_context *cxt, uint16_t profile_id, uint16_t effect_id, uint16_t parameter_id);
int cxt_get_parameter_and_effect_by_id(kest_context *cxt, kest_parameter_id id, kest_parameter **pp, kest_effect **tp);
kest_setting *cxt_get_setting_by_id(kest_context *cxt, uint16_t profile_id, uint16_t effect_id, uint16_t parameter_id);

kest_profile *cxt_get_profile_by_fname(kest_context *cxt, const char *fname);
kest_sequence *cxt_get_sequence_by_fname(kest_context *cxt, const char *fname);

int cxt_effect_id_to_position(kest_context *cxt, uint16_t profile_id, uint16_t effect_id);
int cxt_effect_position_to_id(kest_context *cxt, uint16_t profile_id, uint16_t effect_pos);

int cxt_remove_effect(kest_context *cxt, uint16_t pid, uint16_t tid);
int cxt_remove_profile(kest_context *cxt, kest_profile *profile);
int cxt_remove_sequence(kest_context *cxt, kest_sequence *sequence);

int set_active_profile(kest_profile *profile);
int set_active_profile_from_sequence(kest_profile *profile);
int set_working_profile(kest_profile *profile);

int context_no_default_profile(kest_context *cxt);
int resolve_default_profile(kest_context *cxt);

int set_profile_as_default(kest_context *cxt, kest_profile *profile);

int cxt_save_all_profiles(kest_context *cxt);
void context_print_profiles(kest_context *cxt);

int cxt_set_all_profiles_left_button_to_main_menu(kest_context *cxt);

int cxt_handle_hw_switch(kest_context *cxt, int sw);

kest_profile *cxt_find_profile(kest_context *cxt, const char *fname);

int kest_cxt_obtain_mutex(kest_context *cxt);
int kest_cxt_release_mutex(kest_context *cxt);

int kest_cxt_queue_save_state(kest_context *cxt);

int kest_cxt_set_input_gain(kest_context *cxt, float gain);
int kest_cxt_set_output_gain(kest_context *cxt, float gain);

kest_effect_desc *kest_cxt_get_effect_desc_from_cname(kest_context *cxt, const char *cname);

#endif
