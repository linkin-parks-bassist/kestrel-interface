#ifndef KEST_INT_SD_CARD_H_
#define KEST_INT_SD_CARD_H_

#ifdef KEST_DESKTOP
#define MOUNT_POINT "./sdcard"
#else
#define MOUNT_POINT "/sdcard"
#endif

int init_sd_card();

int kest_sd_mode_msc();
int kest_sd_mode_local();

int kest_sd_toggle_msc();

extern int sd_msc_mode;

#ifdef KEST_USE_FREERTOS
extern SemaphoreHandle_t sd_mutex;
#endif

#endif
