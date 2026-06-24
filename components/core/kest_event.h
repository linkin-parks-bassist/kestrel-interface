#ifndef KEST_EVENT_H_
#define KEST_EVENT_H_

#define KEST_EVENT_NONE			0
#define KEST_EVENT_STARTUP 		1
#define KEST_EVENT_PARAM_CHANGE	2

typedef struct {
	int type;
	
	int   val_i;
	float val_f;
	void *val_ptr;
} kest_event;

int kest_event_task_start();

int kest_event_log(kest_event event);

#endif
