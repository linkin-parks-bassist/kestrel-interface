#ifndef KEST_GLOBAL_H_
#define KEST_GLOBAL_H_

DECLARE_LIST(int);
DECLARE_LIST(uint8_t);
DECLARE_PTR_LIST(char);
DECLARE_LINKED_PTR_LIST(char);
typedef char_pll string_ll;
typedef char_ptr_list string_list;

int64_t kest_system_time_ms();
int64_t kest_system_time_us();

#endif
