#ifndef M_SETTINGS_H_
#define M_SETTINGS_H_

typedef struct m_state
{
	float input_gain;
	float output_gain;
	
	m_page_identifier current_page;
	char active_profile_fname[32];
	char active_sequence_fname[32];
} m_state;

int m_cxt_clone_state(m_context *cxt, m_state *state);
int m_cxt_restore_state(m_context *cxt, m_state *state);
int m_cxt_enter_previous_current_page(m_context *cxt, m_state *state);

int m_init_state(m_state *state);

#ifdef M_USE_FREERTOS
extern SemaphoreHandle_t state_mutex;
#endif

#ifdef M_ENABLE_REPRESENTATIONS
extern m_representation state_representation;
void m_state_representation_update(void *representer, void *representee);
#endif


#endif
