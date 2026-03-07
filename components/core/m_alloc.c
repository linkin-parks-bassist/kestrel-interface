#include "m_int.h"

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "m_alloc.c";

static size_t total_current, total_peak;

void *m_alloc(size_t size)
{
	#ifdef M_LOG_ALLOCS
	#endif
	
	if (size == 0)
	{
		#ifdef M_LOG_ALLOCS
		return NULL;
		#else
		return NULL;
		#endif
    }
	
    uint8_t *ptr = malloc(size + sizeof(size_t));
    
    if (!ptr)
    {
		#ifdef M_LOG_ALLOCS
		return NULL;
		#else
		return NULL;
		#endif
    }
    
    *(size_t*)ptr = size;
    
    total_current += size;
    
    if (total_current > total_peak)
        total_peak = total_current;
    
    #ifdef M_LOG_ALLOCS
    return ptr + sizeof(size_t);
    #else
    return ptr + sizeof(size_t);
    #endif
}

void m_free(void *ptr)
{
	#ifdef M_LOG_ALLOCS
	#endif
	
    if (!ptr)
	{
		#ifdef M_LOG_ALLOCS
		return;
		#else
		return;
		#endif
    }
    
    uint8_t *base_ptr = (uint8_t*)ptr - sizeof(size_t);
    
    size_t size = *(size_t*)base_ptr;
    
    total_current -= size;
    
    free(base_ptr);
    
	#ifdef M_LOG_ALLOCS
	return;
	#endif
}

void *m_realloc(void *ptr, size_t size)
{
	#ifdef M_LOG_ALLOCS
	#endif
	
	if (!ptr)
	{
		#ifdef M_LOG_ALLOCS
		return m_alloc(size);
		#else
		return m_alloc(size);
		#endif
	}
	
	if (size == 0)
		m_free(ptr);
	
    uint8_t *base_ptr = (uint8_t*)ptr - sizeof(size_t);
    size_t base_size = *(size_t*)base_ptr;
    
    uint8_t *new_ptr = realloc(base_ptr, size + sizeof(size_t));
    
    if (!new_ptr)
    {
		#ifdef M_LOG_ALLOCS
		return NULL;
		#else
		return NULL;
		#endif
	}
    
    *(size_t*)new_ptr = size;
    
    total_current += (size - base_size);
    
    if (total_current > total_peak)
        total_peak = total_current;
    
    #ifdef M_LOG_ALLOCS
    return new_ptr + sizeof(size_t);
    #else
    return new_ptr + sizeof(size_t);
    #endif
}

char *m_strndup(const char *str, size_t n)
{
	#ifdef M_LOG_ALLOCS
	#endif
	
	if (!str)
	{
		#ifdef M_LOG_ALLOCS
		return NULL;
		#else
		return NULL;
		#endif
	}
	
    size_t len = strnlen(str, n);
    
    char *new_str = m_alloc(len + 1);
    
    if (!new_str)
    {
		#ifdef M_LOG_ALLOCS
		return NULL;
		#else
		return NULL;
		#endif
	}
	
	memcpy(new_str, str, len);
    new_str[len] = '\0';
    
	#ifdef M_LOG_ALLOCS
	return new_str;
	#else
	return new_str;
	#endif
}

void m_mem_monitor_task(void *param)
{
	#ifdef M_USE_FREERTOS
	while (1)
	{
		print_memory_report();
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
	#endif
}


void m_mem_init()
{
	M_PRINTF("m_mem_init()");
	#ifdef PRINT_MEMORY_USAGE
	#ifdef M_USE_FREERTOS
	M_PRINTF("Spinning off memory printer task...\n");
	xTaskCreate(
		m_mem_monitor_task,
		"memory_log",
		2048,
		NULL,
		5,                  
		NULL
	);
	#endif
	#endif
}

#ifndef M_DESKTOP
void lv_mem_init(void)
{
	return;
}

void lv_mem_deinit(void)
{
	return;
}

void * lv_malloc_core(size_t size)
{
	return m_alloc(size);
}
void * lv_realloc_core(void * p, size_t new_size)
{
	return m_realloc(p, new_size);
}

void lv_free_core(void * p)
{
	return m_free(p);
}

void *m_lv_malloc(size_t size)
{
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void m_lv_free(void *ptr)
{
    heap_caps_free(ptr);
}
#endif

void print_memory_report()
{
    M_PRINTF("Memory usage: %d alloc'd, %d at peak\n", total_current, total_peak);
}
