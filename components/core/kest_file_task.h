#ifndef KEST_FILE_TASK_H_
#define KEST_FILE_TASK_H_

int kest_init_file_task();

int kest_queue_state_save();
int kest_queue_preset_save(kest_preset *preset);
int kest_queue_sequence_save(kest_sequence *sequence);

#endif
