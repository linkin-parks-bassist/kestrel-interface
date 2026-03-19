#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_alloc.c";

static size_t total_current, total_peak;

void *kest_alloc(size_t size)
{
	if (size == 0)
		return NULL;
	
    uint8_t *ptr = malloc(size + sizeof(size_t));
    
    if (!ptr)
		return NULL;
    
    *(size_t*)ptr = size;
    
    total_current += size;
    
    if (total_current > total_peak)
        total_peak = total_current;
    
    
    return ptr + sizeof(size_t);
}

void kest_free(void *ptr)
{
    if (!ptr) return;
    
    uint8_t *base_ptr = (uint8_t*)ptr - sizeof(size_t);
    
    size_t size = *(size_t*)base_ptr;
    
    total_current -= size;
    
    free(base_ptr);
}

void *kest_realloc(void *ptr, size_t size)
{
	if (!ptr)
		return kest_alloc(size);
	
	if (size == 0)
		kest_free(ptr);
	
    uint8_t *base_ptr = (uint8_t*)ptr - sizeof(size_t);
    size_t base_size = *(size_t*)base_ptr;
    
    uint8_t *new_ptr = realloc(base_ptr, size + sizeof(size_t));
    
    if (!new_ptr)
		return NULL;
    
    *(size_t*)new_ptr = size;
    
    total_current += (size - base_size);
    
    if (total_current > total_peak)
        total_peak = total_current;
    
    
    return new_ptr + sizeof(size_t);
    
}

char *kest_strndup(const char *str, size_t n)
{
	#ifdef KEST_LOG_ALLOCS
	#endif
	
	if (!str)
	{
		
		return NULL;
		
	}
	
    size_t len = strnlen(str, n);
    
    char *new_str = kest_alloc(len + 1);
    
    if (!new_str)
    {
		
		return NULL;
		
	}
	
	memcpy(new_str, str, len);
    new_str[len] = '\0';
    
	
	return new_str;
	
}

void kest_mem_monitor_task(void *param)
{
	#ifdef KEST_USE_FREERTOS
	while (1)
	{
		print_memory_report();
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
	#endif
}


void kest_mem_init()
{
	KEST_PRINTF("kest_mem_init()");
	#ifdef PRINT_MEMORY_USAGE
	#ifdef KEST_USE_FREERTOS
	KEST_PRINTF("Spinning off memory printer task...\n");
	xTaskCreate(
		kest_mem_monitor_task,
		"memory_log",
		2048,
		NULL,
		5,                  
		NULL
	);
	#endif
	#endif
}

#ifndef KEST_DESKTOP
#ifndef KEST_LIBRARY
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
	return kest_alloc(size);
}
void * lv_realloc_core(void * p, size_t new_size)
{
	return kest_realloc(p, new_size);
}

void lv_free_core(void * p)
{
	return kest_free(p);
}

void *kest_lv_malloc(size_t size)
{
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void kest_lv_free(void *ptr)
{
    heap_caps_free(ptr);
}
#endif
#endif

void print_memory_report()
{
    KEST_PRINTF("Memory usage: %d alloc'd, %d at peak\n", total_current, total_peak);
}

void *kest_allocator_alloc(kest_allocator *a, size_t n)
{
    if (!a || !a->alloc)
        return kest_alloc(n);

    return a->alloc(a->data, n);
}

void *kest_allocator_realloc(kest_allocator *a, void *p, size_t n)
{
    if (!a || !a->realloc)
        return kest_realloc(p, n);

    return a->realloc(a->data, p, n);
}

void *kest_allocator_strndup(kest_allocator *a, const char *str, int n)
{
	if (!str) return NULL;
	
    size_t len = strnlen(str, n);
    char *new_str = kest_allocator_alloc(a, len + 1);
    
    if (!new_str) return NULL;
	
	memcpy(new_str, str, len);
    new_str[len] = '\0';
    
	return new_str;
}

void kest_allocator_free(kest_allocator *a, void *p)
{
    if (!p)
        return;

    if (!a || !a->free)
    {
        kest_free(p);
        return;
    }

    a->free(a->data, p);
}
