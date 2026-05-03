#ifndef KEST_REPRESENTATION_H_
#define KEST_REPRESENTATION_H_

/**
 * 
 * @brief A system for delegating calls for thread safety/ownership reasons. Due to be deprecated
 * bc I realised I can just use lv_async_call lol
 * 
 */

typedef struct
{
	int info;
	
	void *representer;
	void *representee;
	
	void (*update)(void *representer, void *representee);
} kest_representation;

DECLARE_LINKED_PTR_LIST(kest_representation);
DECLARE_PTR_LIST(kest_representation);
DECLARE_LIST(kest_representation);

int kest_representation_queue_update(kest_representation *rep);

void kest_representation_pll_update_all(kest_representation_pll *reps);

#ifdef KEST_ENABLE_REPRESENTATIONS
int init_representation_updater();
#endif
int queue_representation_list_update(kest_representation_pll *reps);

#ifdef KEST_USE_FREERTOS
extern QueueHandle_t kest_rep_update_queue;
#endif

kest_representation_pll *kest_representation_pll_remove(kest_representation_pll *reps, kest_representation *rep);

/* Re-implementation with local copies and dynamic arrays for better cache locality */
int kest_representation_queue_update_(kest_representation rep);
int kest_representation_list_queue_updates_(kest_representation_list *reps);
int kest_representation_ptr_list_queue_updates_(kest_representation_ptr_list *reps);
#endif
