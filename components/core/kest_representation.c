#include "kest_int.h"

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "kest_representation.c";

IMPLEMENT_LINKED_PTR_LIST(kest_representation);
IMPLEMENT_PTR_LIST(kest_representation);
IMPLEMENT_LIST(kest_representation);

#ifdef KEST_USE_FREERTOS
QueueHandle_t kest_rep_update_queue;
QueueHandle_t kest_rep_update_queue_;
int rep_updater_initd = 0;
#endif

void kest_representation_pll_update_all(kest_representation_pll *reps)
{
	KEST_PRINTF("kest_representation_pll_update_all\n");
	kest_representation_pll *current = reps;
	
	while (current)
	{
		KEST_PRINTF("\n");
		if (current->data)
		{
			KEST_PRINTF("Representation: {.representer = %p, representee = %p, update = %p}\n",
				current->data->representer, current->data->representee, current->data->update);
		}
		if (current->data && current->data->update)
		{
			current->data->update(current->data->representer, current->data->representee);
		}
		
		KEST_PRINTF("\n");
		current = current->next;
	}
	
	KEST_PRINTF("kest_representation_pll_update_all done\n");
	return;
}

#ifdef KEST_ENABLE_REPRESENTATIONS
void update_queued_representations_cb(lv_timer_t *timer)
{	
	kest_representation_pll *list;
	
	while (xQueueReceive(kest_rep_update_queue, &list, 0) == pdTRUE)
	{
		KEST_PRINTF("\n");
		if (list) kest_representation_pll_update_all(list);
	}
}

void update_queued_representations_cb_(lv_timer_t *timer);

int init_representation_updater()
{
	return ERR_FEATURE_DISABLED;
	
	kest_rep_update_queue  = xQueueCreate(16, sizeof(kest_representation_pll*));
	kest_rep_update_queue_ = xQueueCreate(128, sizeof(kest_representation));
	lv_timer_t *timer  = lv_timer_create(update_queued_representations_cb,  1, NULL);
	lv_timer_t *timer_ = lv_timer_create(update_queued_representations_cb_, 1, NULL);
	rep_updater_initd = 1;
	return NO_ERROR;
}
#endif

int kest_representation_queue_update(kest_representation *rep)
{
	return ERR_FEATURE_DISABLED;
	KEST_PRINTF("kest_representation_queue_update\n");
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	#ifdef KEST_USE_FREERTOS
	if (!rep) return ERR_NULL_PTR;
	
	if (!rep_updater_initd)
	{
		KEST_PRINTF("Aborting; representation updater non initialised\n");
		return ERR_QUEUE_SEND_FAILED;
	}
	
	
	kest_representation_pll *node = kest_alloc(sizeof(kest_representation_pll));
	
	if (!node)
		return ERR_ALLOC_FAIL;
	
	node->next = NULL;
	node->data = rep;
	
	KEST_PRINTF("kest_representation_queue_update done\n");
	return queue_representation_list_update(node);
	#endif
	KEST_PRINTF("kest_representation_queue_update done\n");
	return NO_ERROR;
	#endif
	KEST_PRINTF("kest_representation_queue_update done\n");
	return ERR_FEATURE_DISABLED;
}

int queue_representation_list_update(kest_representation_pll *reps)
{
	return ERR_FEATURE_DISABLED;
	KEST_PRINTF("queue_representation_list_update\n");
	#ifdef KEST_ENABLE_REPRESENTATIONS
	#ifdef KEST_USE_FREERTOS
	if (!rep_updater_initd)
	{
		KEST_PRINTF("Aborting; representation updater non initialised\n");
		return ERR_QUEUE_SEND_FAILED;
	}
	
	if (xQueueSend(kest_rep_update_queue, (void*)&reps, (TickType_t)10) != pdPASS)
	{
		KEST_PRINTF("queue_representation_list_update failed\n");
		return ERR_CURRENTLY_EXHAUSTED;
	}
	#endif
	KEST_PRINTF("queue_representation_list_update done\n");
	return NO_ERROR;
	#endif
	KEST_PRINTF("queue_representation_list_update done\n");
	return ERR_FEATURE_DISABLED;
}

kest_representation_pll *kest_representation_pll_remove(kest_representation_pll *reps, kest_representation *rep)
{
	if (!reps)
		return NULL;
	
	kest_representation_pll *current = reps;
	kest_representation_pll *prev = NULL;
	kest_representation_pll *head = reps;
	
	while (current)
	{
		if (current->data == rep)
		{
			if (prev)
				prev->next = current->next;
			else
				head = current->next;
			
			kest_free(current);
			return head;
		}
		
		prev = current;
		current = current->next;
	}
	
	return head;
}

/*
 * 
 * 
 * 
 * Re-implementation with local copies and dynamic arrays for better cache locality
 * 
 * 
 * 
 */


#define PRINTLINES_ALLOWED 0


int kest_representation_queue_update_(kest_representation rep)
{
	KEST_PRINTF("kest_representation_queue_update_\n");
	
	#ifdef KEST_ENABLE_REPRESENTATIONS
	#ifdef KEST_USE_FREERTOS
	if (!rep_updater_initd) return ERR_CURRENTLY_EXHAUSTED;
	
	if (xQueueSend(kest_rep_update_queue_, (void*)&rep, (TickType_t)10) != pdPASS)
	{
		KEST_PRINTF("kest_representation_queue_update_ failed\n");
		return ERR_CURRENTLY_EXHAUSTED;
	}
	
	KEST_PRINTF("kest_representation_queue_update_ done\n");
	return NO_ERROR;
	#endif
	KEST_PRINTF("kest_representation_queue_update_ done\n");
	return NO_ERROR;
	#endif
	KEST_PRINTF("kest_representation_queue_update_ done\n");
	return ERR_FEATURE_DISABLED;
}

int kest_representation_list_queue_updates_(kest_representation_list *reps)
{
	KEST_PRINTF("kest_representation_list_queue_updates\n");
	#ifdef KEST_ENABLE_REPRESENTATIONS
	#ifdef KEST_USE_FREERTOS
	if (!reps) 							return ERR_NULL_PTR;
	if (reps->count && !reps->entries) 	return ERR_BAD_ARGS;
	if (!rep_updater_initd) 			return ERR_QUEUE_SEND_FAILED;
	
	for (int i = 0; i < reps->count; i++)
	{
		if (xQueueSend(kest_rep_update_queue_, (void*)&reps->entries[i], (TickType_t)10) != pdPASS)
		{
			KEST_PRINTF("queue_representation_list_update failed\n");
			return ERR_CURRENTLY_EXHAUSTED;
		}
	}
	
	#endif
	KEST_PRINTF("kest_representation_list_queue_updates done\n");
	return NO_ERROR;
	#endif
	KEST_PRINTF("kest_representation_list_queue_updates done\n");
	return ERR_FEATURE_DISABLED;
}

int kest_representation_ptr_list_queue_updates_(kest_representation_ptr_list *reps)
{
	KEST_PRINTF("kest_representation_list_queue_updates\n");
	#ifdef KEST_ENABLE_REPRESENTATIONS
	#ifdef KEST_USE_FREERTOS
	if (!reps) 							return ERR_NULL_PTR;
	if (reps->count && !reps->entries) 	return ERR_BAD_ARGS;
	if (!rep_updater_initd) 			return ERR_QUEUE_SEND_FAILED;
	
	for (int i = 0; i < reps->count; i++)
	{
		if (reps->entries[i])
		{
			if (xQueueSend(kest_rep_update_queue_, (void*)reps->entries[i], (TickType_t)10) != pdPASS)
			{
				KEST_PRINTF("queue_representation_list_update failed\n");
				return ERR_CURRENTLY_EXHAUSTED;
			}
		}
	}
	
	#endif
	KEST_PRINTF("kest_representation_list_queue_updates done\n");
	return NO_ERROR;
	#endif
	KEST_PRINTF("kest_representation_list_queue_updates done\n");
	return ERR_FEATURE_DISABLED;
}

#ifdef KEST_ENABLE_REPRESENTATIONS
void update_queued_representations_cb_(lv_timer_t *timer)
{	
	kest_representation rep;
	
	while (xQueueReceive(kest_rep_update_queue_, &rep, 0) == pdTRUE)
	{
		if (rep.update)
		{
			rep.update(rep.representer, rep.representee);
		}
	}
}
#endif
