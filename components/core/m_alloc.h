#ifndef M_MALLOC_WRAPPER_H_
#define M_MALLOC_WRAPPER_H_

//#define M_LOG_ALLOCS

typedef struct {
    void *(*alloc)(void *data, size_t);
    void *(*realloc)(void *data, void *, size_t);
    void (*free)(void *data, void *);
    
    void *data;
} m_allocator;

void *m_alloc(size_t size);
void *m_realloc(void *ptr, size_t size);
char *m_strndup(const char *str, size_t n);
void m_free(void *ptr);

void *m_alloc_default(void *data, size_t size);
void *m_realloc_default(void *data, void *ptr, size_t size);
void m_free_default(void *data, void *ptr);

void *m_allocator_alloc(m_allocator *a, size_t n);
void *m_allocator_realloc(m_allocator *a, void *p, size_t n);
void *m_allocator_strndup(m_allocator *a, const char *str, int n);
void  m_allocator_free(m_allocator *a, void *p);

void *m_lv_malloc(size_t size);
void m_lv_free(void *ptr);

void m_mem_init();
void print_memory_report();

#endif
