#ifndef KEST_UPDATE_H_
#define KEST_UPDATE_H_

void kest_active_preset_updater_start();

int kest_active_preset_updater_notify_effect_by_ptr(kest_effect *effect);
int kest_active_preset_updater_notify_effect_by_id(int preset_id, int effect_id);
int kest_active_preset_updater_notify_param (int preset_id, int effect_id, int parameter_id);

#endif
