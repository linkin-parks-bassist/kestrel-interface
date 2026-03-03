#ifndef M_INT_SD_CARD_H_
#define M_INT_SD_CARD_H_

#ifdef M_DESKTOP
#define MOUNT_POINT "./sdcard"
#else
#define MOUNT_POINT "/sdcard"
#endif

int init_sd_card();

int m_sd_mode_msc();
int m_sd_mode_local();

int m_sd_toggle_msc();

extern int sd_msc_mode;

#ifdef M_USE_FREERTOS
extern SemaphoreHandle_t sd_mutex;
#endif

#endif
