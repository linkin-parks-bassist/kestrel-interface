#ifndef KEST_POOL_H_
#define KEST_POOL_H_

#ifdef KEST_USE_FREERTOS
#define KEST_POOL_MUTEX SemaphoreHandle_t mutex;
#define KEST_POOL_MUTEX_INIT() (pool->mutex = xSemaphoreCreateMutex())
#define KEST_POOL_MUTEX_LOCK() ((xSemaphoreTake(pool->mutex, portMAX_DELAY) == pdTRUE))
#define KEST_POOL_MUTEX_UNLOCK() do {if (pool) xSemaphoreGive(pool->mutex);} while (0)
#else
#define KEST_POOL_MUTEX
#define KEST_POOL_MUTEX_INIT() 1
#define KEST_POOL_MUTEX_LOCK() 1
#define KEST_POOL_MUTEX_UNLOCK()
#endif



#define DECLARE_POOL(X) \
typedef struct {\
	size_t size;\
	size_t free_count;\
	\
	size_t obtain_head;\
	size_t return_head;\
	\
	X *entries;\
	X **buffer;\
	\
	KEST_POOL_MUTEX\
	\
	void (*init_function)(X*);\
	void (*deinit_function)(X*);\
	void (*copy_function)(X *dest, X *src);\
} X##_pool;\
\
int X##_pool_init(X##_pool *pool);\
int X##_pool_reserve(X##_pool *pool, size_t size);\
\
X     *X##_pool_obtain(X##_pool *pool);\
size_t X##_pool_obtain_n(X##_pool *pool, size_t n, X **buf);\
\
int X##_pool_return(X##_pool *pool, X *x);\
\
int X##_pool_init_allocator(X##_pool *pool, kest_allocator *a);\
\
void *X##_pool_alloc  (void *p, size_t n);\
void *X##_pool_realloc(void *p, void *x, size_t n);\
void  X##_pool_free	  (void *p, void *x);\
\
int  X##_pool_lock  (X##_pool *pool);\
void X##_pool_unlock(X##_pool *pool);

#define IMPLEMENT_POOL(X)\
int X##_pool_init(X##_pool *pool)\
{\
	if (!pool)\
		return ERR_NULL_PTR;\
	\
	memset(pool, 0, sizeof(X##_pool));\
	\
	if (!KEST_POOL_MUTEX_INIT()) return ERR_MUTEX_UNAVAILABLE;\
	\
	return NO_ERROR;\
}\
\
int X##_pool_reserve(X##_pool *pool, size_t size)\
{\
	if (!pool)\
		return ERR_NULL_PTR;\
	\
	if (X##_pool_lock(pool) != NO_ERROR) return ERR_MUTEX_UNAVAILABLE;\
	\
	if (size == 0)\
	{\
		pool->size = 0;\
		pool->free_count = 0;\
		pool->entries = NULL;\
		pool->buffer  = NULL;\
		pool->obtain_head = 0;\
		pool->return_head = 0;\
		\
		X##_pool_unlock(pool);\
		return NO_ERROR;\
	}\
	\
	pool->entries = kest_alloc(size * sizeof(X));\
	\
	if (!pool->entries)\
	{\
		X##_pool_unlock(pool);\
		return ERR_ALLOC_FAIL;\
	}\
	\
	pool->buffer = kest_alloc(size * sizeof(X*));\
	\
	if (!pool->buffer)\
	{\
		kest_free(pool->entries);\
		pool->entries = NULL;\
		X##_pool_unlock(pool);\
		return ERR_ALLOC_FAIL;\
	}\
	\
	for (size_t i = 0; i < size; i++)\
		pool->buffer[i] = &pool->entries[i];\
	\
	pool->size = size;\
	pool->free_count = size;\
	pool->obtain_head = 0;\
	pool->return_head = size - 1;\
	\
	X##_pool_unlock(pool);\
	\
	return NO_ERROR;\
}\
\
\
X *X##_pool_obtain(X##_pool *pool)\
{\
	if (!pool)\
		return NULL;\
	\
	if (!pool->entries || !pool->buffer)\
		return NULL;\
	\
	X *x = NULL;\
	\
	if (X##_pool_lock(pool) != NO_ERROR) return NULL;\
	if (pool->free_count)\
	{\
		x = pool->buffer[pool->obtain_head];\
		pool->obtain_head = (pool->obtain_head + 1) % pool->size;\
		\
		pool->free_count = pool->free_count - 1;\
		\
		X##_pool_unlock(pool);\
		\
		memset(x, 0, sizeof(X));\
		if (pool->init_function)\
			pool->init_function(x);\
	}\
	else \
	{\
		X##_pool_unlock(pool);\
		return NULL;\
	}\
	\
	return x;\
}\
\
size_t X##_pool_obtain_n(X##_pool *pool, size_t n, X **buf)\
{\
	if (!pool || !buf)\
		return 0;\
	\
	if (!pool->entries || !pool->buffer)\
		return 0;\
	\
	X *x;\
	\
	for (size_t i = 0; i < n; i++)\
	{\
		x = X##_pool_obtain(pool);\
		\
		if (!x) return i;\
		\
		buf[i] = x;\
	}\
	\
	return n;\
}\
\
int X##_pool_return(X##_pool *pool, X *x)\
{\
	if (!pool || !x)\
		return ERR_NULL_PTR;\
	\
	if (pool->deinit_function)\
		pool->deinit_function(x);\
	if (!pool->entries || !pool->buffer)\
		return ERR_BAD_ARGS;\
	\
	if (X##_pool_lock(pool) != NO_ERROR) return ERR_MUTEX_UNAVAILABLE;\
	if (pool->free_count == pool->size)\
	{\
		X##_pool_unlock(pool);\
		return ERR_BAD_ARGS;\
	}\
	\
	pool->return_head = (pool->return_head + 1) % pool->size;\
	pool->buffer[pool->return_head] = x;\
	\
	pool->free_count = pool->free_count + 1;\
	\
	X##_pool_unlock(pool);\
	\
	return NO_ERROR;\
}\
\
int X##_pool_init_allocator(X##_pool *pool, kest_allocator *a)\
{\
	if (!pool || !a)\
		return ERR_NULL_PTR;\
	\
	a->data = pool;\
	\
	a->alloc 	= X##_pool_alloc;\
	a->realloc 	= X##_pool_realloc;\
	a->free 	= X##_pool_free;\
	\
	a->flags = KEST_ALLOCATOR_FLAG_SINGULAR | KEST_ALLOCATOR_FLAG_DISALLOW_STRNDUP;\
	\
	return NO_ERROR;\
}\
\
void *X##_pool_alloc(void *p, size_t n)\
{\
	if (!p || n != 1) return NULL;\
	\
	return (void*)X##_pool_obtain((X##_pool*)p);\
}\
\
void *X##_pool_realloc(void *p, void *x, size_t n)\
{\
	X##_pool *pool = p;\
	\
	if (!x || !p || n != 1) return NULL;\
	\
	X *y = (X*)X##_pool_obtain(pool);\
	\
	if (!y)\
		return NULL;\
	\
	if (pool->copy_function)\
		pool->copy_function(y, x);\
	else\
		memcpy(y, x, sizeof(X));\
	\
	X##_pool_return(pool, x);\
	\
	return y;\
}\
\
void X##_pool_free(void *p, void *x)\
{\
	X##_pool_return((X##_pool*)p, (X*)x);\
}\
\
int X##_pool_lock(X##_pool *pool)\
{\
	if (!pool)\
		return ERR_NULL_PTR;\
	\
	if (!KEST_POOL_MUTEX_LOCK()) return ERR_MUTEX_UNAVAILABLE;\
	\
	return NO_ERROR;\
	\
}\
\
void X##_pool_unlock(X##_pool *pool)\
{\
	KEST_POOL_MUTEX_UNLOCK();\
}

#endif
