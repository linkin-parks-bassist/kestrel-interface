#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_representation.c";

IMPLEMENT_LINKED_PTR_LIST(kest_representation);

#ifdef KEST_USE_FREERTOS
QueueHandle_t kest_rep_update_queue;
int rep_updated_initd = 0;
#endif

void kest_representation_pll_update_all(kest_representation_pll *reps)
{
	KEST_PRINTF("kest_representation_pll_update_all\n");
	kest_representation_pll *current = reps;
	
	while (current)
	{
		if (current->data)
		{
			KEST_PRINTF("Representation: {.representer = %p, representee = %p, update = %p}\n",
				current->data->representer, current->data->representee, current->data->update);
		}
		if (current->data && current->data->update)
		{
			current->data->update(current->data->representer, current->data->representee);
		}
		
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
		if (list) kest_representation_pll_update_all(list);
	}
}

int init_representation_updater()
{
	kest_rep_update_queue = xQueueCreate(16, sizeof(kest_representation_pll*));
	lv_timer_t * timer = lv_timer_create(update_queued_representations_cb, 1,  NULL);
	rep_updated_initd = 1;
	return NO_ERROR;
}
#endif

int queue_representation_list_update(kest_representation_pll *reps)
{
	#ifdef KEST_ENABLE_REPRESENTATIONS
	#ifdef KEST_USE_FREERTOS
	if (!rep_updated_initd)
		return ERR_CURRENTLY_EXHAUSTED;
	
	if (xQueueSend(kest_rep_update_queue, (void*)&reps, (TickType_t)10) != pdPASS)
	{
		return ERR_QUEUE_SEND_FAILED;
	}
	#endif
	return NO_ERROR;
	#endif
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
