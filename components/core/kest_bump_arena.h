#ifndef KEST_BUMP_ARENA_H_
#define KEST_BUMP_ARENA_H_

#define KEST_BUMP_ARENA_ALLOC_ALIGN 4
#define KEST_BUMP_ARENA_INIT_ALIGN  64

#if (KEST_BUMP_ARENA_ALLOC_ALIGN & (KEST_BUMP_ARENA_ALLOC_ALIGN - 1)) != 0
#error KEST_BUMP_ARENA_ALLOC_ALIGN must be power of two
#endif
#if (KEST_BUMP_ARENA_INIT_ALIGN & (KEST_BUMP_ARENA_INIT_ALIGN - 1)) != 0
#error KEST_BUMP_ARENA_INIT_ALIGN must be power of two
#endif

typedef struct {
	void *base_ua;
	void *arena;
	size_t pos;
	size_t capacity;
	
	kest_allocator alloc;
} kest_bump_arena;

int kest_bump_arena_init_empty(kest_bump_arena *arena);
int kest_bump_arena_init(kest_bump_arena *arena, size_t capacity);

void *kest_bump_arena_alloc(kest_bump_arena *arena, size_t size);
void *kest_bump_arena_realloc(kest_bump_arena *arena, void *p, size_t size);
void kest_bump_arena_free(kest_bump_arena *arena);

int   kest_bump_arena_reset(kest_bump_arena *arena);
int   kest_bump_arena_destroy(kest_bump_arena *arena);

#endif
