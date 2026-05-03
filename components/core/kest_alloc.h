#ifndef KEST_MALLOC_WRAPPER_H_
#define KEST_MALLOC_WRAPPER_H_

#define KEST_EFFECT_DESC_POOL_SIZE 	4096
#define KEST_PARAMETER_POOL_SIZE 	4096
#define KEST_SEQUENCE_POOL_SIZE 	128
#define KEST_SETTING_POOL_SIZE 		4096
#define KEST_EFFECT_POOL_SIZE 		4096
#define KEST_PRESET_POOL_SIZE 		4096

#define KEST_ALLOCATOR_FLAG_DISALLOW_FREE 	 0b0000001
#define KEST_ALLOCATOR_FLAG_DISALLOW_REALLOC 0b0000010
#define KEST_ALLOCATOR_FLAG_DISALLOW_STRNDUP 0b0000100
#define KEST_ALLOCATOR_FLAG_SINGULAR		 0b0001000

typedef struct {
    void *(*alloc)(void *data, size_t);
    void *(*realloc)(void *data, void *, size_t);
    void (*free)(void *data, void *);
    
    void *data;
    int flags;
} kest_allocator;

void *kest_alloc(size_t size);
void *kest_realloc(void *ptr, size_t size);
char *kest_strndup(const char *str, size_t n);
void  kest_free(void *ptr);

int kest_allocator_init(kest_allocator *a);

void *kest_allocator_alloc(kest_allocator *a, size_t n);
void *kest_allocator_realloc(kest_allocator *a, void *p, size_t n);
void *kest_allocator_strndup(kest_allocator *a, const char *str, int n);
void  kest_allocator_free(kest_allocator *a, void *p);

void *kest_lv_malloc(size_t size);
void kest_lv_free(void *ptr);

int kest_mem_init();
void print_memory_report();

#endif
