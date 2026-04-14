#include <stdarg.h>
#include "kest_int.h"

static const char *FNAME = "kest_printf.c";

int prints_initd = 0;
#ifdef KEST_USE_FREERTOS
static SemaphoreHandle_t print_mutex;
#endif

void kest_printf_init()
{
	#ifdef KEST_ENABLE_PRINTF
	#ifdef KEST_USE_FREERTOS
	print_mutex = xSemaphoreCreateMutex();
	#endif
	prints_initd = 1;
	#endif
}

void kest_printf(const char *fmt, ...)
{
	#ifdef KEST_ENABLE_PRINTF
	#ifdef KEST_USE_FREERTOS
	if (!prints_initd || xSemaphoreTake(print_mutex, portMAX_DELAY) != pdPASS) return;
	#endif
	
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 1024, fmt, args);
	va_end(args);
	
	fputs(buf, stdout);
	
	#ifdef KEST_USE_FREERTOS
	xSemaphoreGive(print_mutex);
	#endif
	#endif
}

void kest_puts(kest_string str)
{
	#ifdef KEST_ENABLE_PRINTF
	#ifdef KEST_USE_FREERTOS
	if (!prints_initd || xSemaphoreTake(print_mutex, portMAX_DELAY) != pdPASS) return;
	#endif
	
	int fza = str.count < str.capacity ? str.count : str.capacity - 1;
	char swpchar = str.entries[fza];
	str.entries[fza] = 0;
	
	fputs(str.entries, stdout);
	
	str.entries[fza] = swpchar;
	
	#ifdef KEST_USE_FREERTOS
	xSemaphoreGive(print_mutex);
	#endif
	#endif
}
