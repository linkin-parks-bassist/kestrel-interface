#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *FNAME = "kest_bump_arena.c";

int kest_bump_arena_init_empty(kest_bump_arena *arena)
{
	if (!arena)
		return ERR_NULL_PTR;
	
	arena->arena = NULL;
	arena->pos = 0;
	arena->capacity = 0;
	
	return NO_ERROR;
}

static void *kest_bump_arena_alloc_wrapper(void *data, size_t size)
{
    return kest_bump_arena_alloc((kest_bump_arena*)data, size);
}

static void *kest_bump_arena_realloc_wrapper(void *data, void *ptr, size_t size)
{
    return kest_bump_arena_realloc((kest_bump_arena*)data, ptr, size);
}

static void kest_bump_arena_free_wrapper(void *data, void *ptr)
{
    (void)data;
    (void)ptr;
    /* no-op */
}

int kest_bump_arena_init(kest_bump_arena *arena, size_t capacity)
{
	if (!arena)
		return ERR_NULL_PTR;
	
	size_t align = KEST_BUMP_ARENA_INIT_ALIGN;
	arena->base_ua = kest_alloc(capacity + align - 1);
	
	if (!arena->base_ua)
	{
		arena->pos = 0;
		arena->arena = NULL;
		arena->capacity = 0;
		return ERR_ALLOC_FAIL;
	}
	
	uintptr_t addr = (uintptr_t)arena->base_ua;
	uintptr_t aligned = (addr + (align - 1)) & ~(uintptr_t)(align - 1);
	
	arena->arena = (void*)aligned;
	arena->pos = 0;
	arena->capacity = capacity;
	
	arena->alloc.alloc 	 = kest_bump_arena_alloc_wrapper;
	arena->alloc.realloc = kest_bump_arena_realloc_wrapper;
	arena->alloc.free 	 = kest_bump_arena_free_wrapper;
	arena->alloc.data 	 = arena;
	
	return NO_ERROR;
}

void *kest_bump_arena_alloc(kest_bump_arena *arena, size_t size)
{
	if (!arena)
	{
		return NULL;
	}
	
	if (!arena->arena || arena->capacity == 0)
	{
		KEST_PRINTF("kest_bump_arena ERROR: arena has no memory!\n");
		return NULL;
	}
	
	if (size == 0)
		return NULL;
	
	size = (size + (KEST_BUMP_ARENA_ALLOC_ALIGN - 1)) & ~(KEST_BUMP_ARENA_ALLOC_ALIGN - 1);
	
	if (size > arena->capacity - arena->pos)
	{
		KEST_PRINTF("kest_bump_arena ERROR: Arena exhausted. Capacity: %d bytes; consumed: %d bytes. Requested: %d bytes\n",
			arena->capacity, arena->pos, size);
		return NULL;
	}
	
	uint8_t *ptr = (uint8_t*)arena->arena + arena->pos;
	arena->pos += size;
	
	return ptr;
}

void *kest_bump_arena_realloc(kest_bump_arena *arena, void *p, size_t size)
{
	if (!arena)
	{
		return NULL;
	}
	
	if (!p)
		return kest_bump_arena_alloc(arena, size);
	
	uint8_t *new_ptr = kest_bump_arena_alloc(arena, size);
	
	if (!new_ptr)
		return NULL;
	
	memcpy(new_ptr, p, size);
	
	return new_ptr;
}

int kest_bump_arena_reset(kest_bump_arena *arena)
{
	if (!arena)
		return ERR_NULL_PTR;
	
	arena->pos = 0;
	
	return NO_ERROR;
}

int kest_bump_arena_destroy(kest_bump_arena *arena)
{
	if (!arena)
		return ERR_NULL_PTR;
	
	if (!arena->base_ua)
	{
		arena->capacity = 0;
		arena->arena = NULL;
		arena->pos = 0;
		return NO_ERROR;
	}
	
	kest_free(arena->base_ua);
	arena->base_ua = NULL;
	arena->arena = NULL;
	arena->capacity = 0;
	arena->pos = 0;
	
	return NO_ERROR;
}
