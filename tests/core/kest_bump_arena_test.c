#include "kest_test.h"

KEST_TEST(kest_test_bump_arena_init_null)
{
    int rc = kest_bump_arena_init(NULL, 128);
    assert(rc == ERR_NULL_PTR);
}


KEST_TEST(kest_test_bump_arena_init_empty_basic)
{
    kest_bump_arena arena;

    int rc = kest_bump_arena_init_empty(&arena);

    assert(rc == NO_ERROR);
    assert(arena.arena == NULL);
    assert(arena.pos == 0);
    assert(arena.capacity == 0);
}


KEST_TEST(kest_test_bump_arena_alloc_from_empty)
{
    kest_bump_arena arena;

    kest_bump_arena_init_empty(&arena);

    void *p = kest_bump_arena_alloc(&arena, 16);

    assert(p == NULL);
}


KEST_TEST(kest_test_bump_arena_basic_alloc)
{
    kest_bump_arena arena;

    int rc = kest_bump_arena_init(&arena, 128);
    assert(rc == NO_ERROR);

    void *a = kest_bump_arena_alloc(&arena, 16);
    void *b = kest_bump_arena_alloc(&arena, 16);

    assert(a != NULL);
    assert(b != NULL);
    assert(a != b);

    kest_bump_arena_destroy(&arena);
}


KEST_TEST(kest_test_bump_arena_monotonic_growth)
{
    kest_bump_arena arena;

    kest_bump_arena_init(&arena, 128);

    void *a = kest_bump_arena_alloc(&arena, 8);
    void *b = kest_bump_arena_alloc(&arena, 8);
    void *c = kest_bump_arena_alloc(&arena, 8);

    assert(a < b);
    assert(b < c);

    kest_bump_arena_destroy(&arena);
}


KEST_TEST(kest_test_bump_arena_alignment)
{
    kest_bump_arena arena;

    kest_bump_arena_init(&arena, 128);

    void *a = kest_bump_arena_alloc(&arena, 1);
    void *b = kest_bump_arena_alloc(&arena, 1);

    assert(((uintptr_t)a & (KEST_BUMP_ARENA_ALLOC_ALIGN - 1)) == 0);
    assert(((uintptr_t)b & (KEST_BUMP_ARENA_ALLOC_ALIGN - 1)) == 0);

    kest_bump_arena_destroy(&arena);
}


KEST_TEST(kest_test_bump_arena_exhaustion)
{
    kest_bump_arena arena;

    kest_bump_arena_init(&arena, 32);

    void *a = kest_bump_arena_alloc(&arena, 16);
    void *b = kest_bump_arena_alloc(&arena, 16);
    void *c = kest_bump_arena_alloc(&arena, 16);

    assert(a != NULL);
    assert(b != NULL);
    assert(c == NULL);

    kest_bump_arena_destroy(&arena);
}


KEST_TEST(kest_test_bump_arena_reset)
{
    kest_bump_arena arena;

    kest_bump_arena_init(&arena, 64);

    void *a = kest_bump_arena_alloc(&arena, 16);

    kest_bump_arena_reset(&arena);

    void *b = kest_bump_arena_alloc(&arena, 16);

    assert(a == b);

    kest_bump_arena_destroy(&arena);
}

KEST_TEST(kest_test_bump_arena_alloc_zero)
{
    kest_bump_arena arena;

    kest_bump_arena_init(&arena, 64);

    void *p = kest_bump_arena_alloc(&arena, 0);

    assert(p == NULL);

    kest_bump_arena_destroy(&arena);
}

KEST_TEST(kest_test_bump_arena_destroy_twice)
{
    kest_bump_arena arena;

    kest_bump_arena_init(&arena, 64);

    int rc1 = kest_bump_arena_destroy(&arena);
    int rc2 = kest_bump_arena_destroy(&arena);

    assert(rc1 == NO_ERROR);
    assert(rc2 == NO_ERROR);
}

KEST_TEST(kest_test_bump_arena_destroy_null)
{
    int rc = kest_bump_arena_destroy(NULL);

    assert(rc == ERR_NULL_PTR);
}
