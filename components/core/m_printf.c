#include <stdarg.h>
#include "m_int.h"

static const char *FNAME = "m_printf.c";

int prints_initd = 0;
#ifdef M_USE_FREERTOS
static SemaphoreHandle_t print_mutex;
#endif

void m_printf_init()
{
	#ifdef M_ENABLE_PRINTF
	#ifdef M_USE_FREERTOS
	print_mutex = xSemaphoreCreateMutex();
	#endif
	prints_initd = 1;
	#endif
}

void m_printf(const char *fmt, ...)
{
	#ifdef M_ENABLE_PRINTF
	#ifdef M_USE_FREERTOS
	if (!prints_initd || xSemaphoreTake(print_mutex, portMAX_DELAY) != pdPASS) return;
	#endif
	
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 1024, fmt, args);
	va_end(args);
	
	fputs(buf, stdout);
	
	#ifdef M_USE_FREERTOS
	xSemaphoreGive(print_mutex);
	#endif
	#endif
}
