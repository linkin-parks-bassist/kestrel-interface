#include "kest_int.h"

IMPLEMENT_LIST(int);
IMPLEMENT_LIST(uint8_t);
IMPLEMENT_PTR_LIST(char);
IMPLEMENT_LINKED_PTR_LIST(char);

int64_t kest_system_time_ms()
{
	#ifdef KEST_ESP32
	return esp_timer_get_time() / 1000;
	#elif defined(KEST_USE_FREERTOS)
	return (int64_t)pdTICKS_TO_MS(xTaskGetTickCount());
	#endif
	return 0;
}

int64_t kest_system_time_us()
{
	#ifdef KEST_ESP32
	return esp_timer_get_time();
	#elif defined(KEST_USE_FREERTOS)
	return (int64_t)1000*pdTICKS_TO_MS(xTaskGetTickCount());
	#endif
	return 0;
}

