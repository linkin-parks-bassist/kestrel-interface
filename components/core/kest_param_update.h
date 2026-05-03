#ifndef KEST_INT_PARAM_UPDATE_H_
#define KEST_INT_PARAM_UPDATE_H_

#define MAX_CONCURRENT_PARAM_UPDATES 16

typedef struct {
	kest_parameter_id id;
	kest_parameter *p;
	kest_effect *t;
	float target;
	int send;
} kest_parameter_update;

int kest_init_parameter_updater();
void kest_param_update_task(void *arg);

int kest_parameter_trigger_update(kest_parameter *param, float target);

#endif
