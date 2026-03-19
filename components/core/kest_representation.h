#ifndef KEST_REPRESENTATION_H_
#define KEST_REPRESENTATION_H_

typedef struct
{
	int info;
	
	void *representer;
	void *representee;
	
	void (*update)(void *representer, void *representee);
} kest_representation;

DECLARE_LINKED_PTR_LIST(kest_representation);

void kest_representation_pll_update_all(kest_representation_pll *reps);

#ifdef KEST_ENABLE_REPRESENTATIONS
int init_representation_updater();
#endif
int queue_representation_list_update(kest_representation_pll *reps);

#ifdef KEST_USE_FREERTOS
extern QueueHandle_t kest_rep_update_queue;
#endif

kest_representation_pll *kest_representation_pll_remove(kest_representation_pll *reps, kest_representation *rep);

#endif
