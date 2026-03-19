#ifndef KEST_MALLOC_WRAPPER_H_
#define KEST_MALLOC_WRAPPER_H_

//#define KEST_LOG_ALLOCS

typedef struct {
    void *(*alloc)(void *data, size_t);
    void *(*realloc)(void *data, void *, size_t);
    void (*free)(void *data, void *);
    
    void *data;
} kest_allocator;

void *kest_alloc(size_t size);
void *kest_realloc(void *ptr, size_t size);
char *kest_strndup(const char *str, size_t n);
void kest_free(void *ptr);

void *kest_alloc_default(void *data, size_t size);
void *kest_realloc_default(void *data, void *ptr, size_t size);
void kest_free_default(void *data, void *ptr);

void *kest_allocator_alloc(kest_allocator *a, size_t n);
void *kest_allocator_realloc(kest_allocator *a, void *p, size_t n);
void *kest_allocator_strndup(kest_allocator *a, const char *str, int n);
void  kest_allocator_free(kest_allocator *a, void *p);

void *kest_lv_malloc(size_t size);
void kest_lv_free(void *ptr);

void kest_mem_init();
void print_memory_report();

#endif
