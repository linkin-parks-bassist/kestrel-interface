#ifndef KEST_FPGA_UPDATE_H_
#define KEST_FPGA_UPDATE_H_

int kest_init_fpga_updater();
int kest_fpga_updater_wake();

extern TaskHandle_t kest_fpga_updater_task_handle;

#endif
