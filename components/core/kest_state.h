#ifndef KEST_SETTINGS_H_
#define KEST_SETTINGS_H_

typedef struct kest_state
{
	float input_gain;
	float output_gain;
	
	kest_page_identifier current_page;
	char active_preset_fname[32];
	char active_sequence_fname[32];
} kest_state;

int kest_cxt_clone_state(kest_context *cxt, kest_state *state);
int kest_cxt_restore_state(kest_context *cxt, kest_state *state);
int kest_cxt_enter_previous_current_page(kest_context *cxt, kest_state *state);

int kest_init_state(kest_state *state);

#ifdef KEST_USE_FREERTOS
extern SemaphoreHandle_t state_mutex;
#endif

#ifdef KEST_ENABLE_REPRESENTATIONS
extern kest_representation state_representation;
void kest_state_representation_update(void *representer, void *representee);
#endif


#endif
