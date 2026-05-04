#include "kest_int.h"

#define PRINTLINES_ALLOWED 0

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
	if (!str)
		return NULL;
	
    size_t len = strnlen(str, n);
    
    char *new_str = kest_alloc(len + 1);
    
    if (!new_str)
		return NULL;
	
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

#define INIT_MEM_POOL(X, n) do {\
		if ((ret_val = X##_pool_init(&X##_mem_pool)) != NO_ERROR)\
		{\
			KEST_PRINTF("Failed to init effect descriptor pool\n");\
			return ret_val;\
		}\
		\
		if ((ret_val = X##_pool_reserve(&X##_mem_pool, n)) != NO_ERROR)\
		{\
			KEST_PRINTF("Failed to reserve effect descriptor pool\n");\
			return ERR_ALLOC_FAIL;\
		}\
		\
		X##_pool_init_allocator(&X##_mem_pool, &X##_allocator);\
	} while (0)

//#define PRINT_MEMORY_USAGE

int kest_mem_init()
{
	KEST_PRINTF("kest_mem_init\n");
	
	int ret_val;
	
	INIT_MEM_POOL(kest_effect_desc, KEST_EFFECT_DESC_POOL_SIZE);
	INIT_MEM_POOL(kest_parameter, 	KEST_PARAMETER_POOL_SIZE);
	INIT_MEM_POOL(kest_sequence, 	KEST_SEQUENCE_POOL_SIZE);
	INIT_MEM_POOL(kest_setting, 	KEST_SETTING_POOL_SIZE);
	INIT_MEM_POOL(kest_effect, 		KEST_EFFECT_POOL_SIZE);
	INIT_MEM_POOL(kest_preset, 		KEST_PRESET_POOL_SIZE);
	
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
	
	return NO_ERROR;
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
    KEST_PRINTF("Memory usage: %d bytes (%d kb, %d mb) alloc'd, %d bytes (%d kb, %d mb) at peak\n",
		total_current, total_current / 1024, total_current / (1024*1024),
		total_peak, total_peak / 1024, total_peak / (1024*1024));
}

int kest_allocator_init(kest_allocator *a)
{
	if (!a) return ERR_NULL_PTR;
	memset(a, 0, sizeof(kest_allocator));
	return NO_ERROR;
}

void *kest_allocator_alloc(kest_allocator *a, size_t n)
{
	if (a)
	{
		if ((a->flags & KEST_ALLOCATOR_FLAG_SINGULAR) && n > 1)
			return NULL;
		
		if (a->alloc)
		{
			return a->alloc(a->data, n);
		}
	}
	
    return kest_alloc(n);
}

void *kest_allocator_realloc(kest_allocator *a, void *p, size_t n)
{
	if (a)
	{
		if (a->flags & KEST_ALLOCATOR_FLAG_DISALLOW_REALLOC)
			return NULL;
		
		if (a->realloc)
			return a->realloc(a->data, p, n);
	}

	return kest_realloc(p, n);
}

void *kest_allocator_strndup(kest_allocator *a, const char *str, int n)
{
	if (!str) return NULL;
	
	if (a)
	{
		if (a->flags & KEST_ALLOCATOR_FLAG_DISALLOW_STRNDUP)
			return NULL;	
	
		size_t len = strnlen(str, n);
		char *new_str = kest_allocator_alloc(a, len + 1);
		
		if (!new_str) return NULL;
		
		memcpy(new_str, str, len);
		new_str[len] = '\0';
		
		return new_str;
	}
	
	return (void*)kest_strndup(str, n);
}

void kest_allocator_free(kest_allocator *a, void *p)
{
    if (!p) return;

	if (a)
	{
		if (a->flags & KEST_ALLOCATOR_FLAG_DISALLOW_FREE)
			return;
		
		if (a->free)
		{
			a->free(a->data, p);
			return;
		}
	}

    kest_free(p);
}
