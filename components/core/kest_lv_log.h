#ifndef KEST_INT_LOGGING_H_
#define KEST_INT_LOGGING_H_

#define KEST_INT_LV_LOG_BUF_LEN 1024

int kest_log_init();

void kest_lv_log_cb(lv_log_level_t level, const char *buf);

#endif
