#ifndef KEST_UPDATE_H_
#define KEST_UPDATE_H_

#define KEST_UPDATE_NONE 	0
#define KEST_UPDATE_PARAM	1

typedef struct {
	int type;
	
	union {
		kest_parameter *param;
	} data;
} kest_update;

int kest_update_task_start();

int kest_update_queue(kest_update update);

void kest_active_preset_updater_start();

int kest_active_preset_updater_notify_effect_by_ptr(kest_effect *effect);
int kest_active_preset_updater_notify_effect_by_id(int preset_id, int effect_id);
int kest_active_preset_updater_notify_param (int preset_id, int effect_id, int parameter_id);

#endif
