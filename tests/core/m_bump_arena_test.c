#include "m_test.h"

M_TEST(m_test_bump_arena_init_null)
{
    int rc = m_bump_arena_init(NULL, 128);
    assert(rc == ERR_NULL_PTR);
}


M_TEST(m_test_bump_arena_init_empty_basic)
{
    m_bump_arena arena;

    int rc = m_bump_arena_init_empty(&arena);

    assert(rc == NO_ERROR);
    assert(arena.arena == NULL);
    assert(arena.pos == 0);
    assert(arena.capacity == 0);
}


M_TEST(m_test_bump_arena_alloc_from_empty)
{
    m_bump_arena arena;

    m_bump_arena_init_empty(&arena);

    void *p = m_bump_arena_alloc(&arena, 16);

    assert(p == NULL);
}


M_TEST(m_test_bump_arena_basic_alloc)
{
    m_bump_arena arena;

    int rc = m_bump_arena_init(&arena, 128);
    assert(rc == NO_ERROR);

    void *a = m_bump_arena_alloc(&arena, 16);
    void *b = m_bump_arena_alloc(&arena, 16);

    assert(a != NULL);
    assert(b != NULL);
    assert(a != b);

    m_bump_arena_destroy(&arena);
}


M_TEST(m_test_bump_arena_monotonic_growth)
{
    m_bump_arena arena;

    m_bump_arena_init(&arena, 128);

    void *a = m_bump_arena_alloc(&arena, 8);
    void *b = m_bump_arena_alloc(&arena, 8);
    void *c = m_bump_arena_alloc(&arena, 8);

    assert(a < b);
    assert(b < c);

    m_bump_arena_destroy(&arena);
}


M_TEST(m_test_bump_arena_alignment)
{
    m_bump_arena arena;

    m_bump_arena_init(&arena, 128);

    void *a = m_bump_arena_alloc(&arena, 1);
    void *b = m_bump_arena_alloc(&arena, 1);

    assert(((uintptr_t)a & (M_BUMP_ARENA_ALLOC_ALIGN - 1)) == 0);
    assert(((uintptr_t)b & (M_BUMP_ARENA_ALLOC_ALIGN - 1)) == 0);

    m_bump_arena_destroy(&arena);
}


M_TEST(m_test_bump_arena_exhaustion)
{
    m_bump_arena arena;

    m_bump_arena_init(&arena, 32);

    void *a = m_bump_arena_alloc(&arena, 16);
    void *b = m_bump_arena_alloc(&arena, 16);
    void *c = m_bump_arena_alloc(&arena, 16);

    assert(a != NULL);
    assert(b != NULL);
    assert(c == NULL);

    m_bump_arena_destroy(&arena);
}


M_TEST(m_test_bump_arena_reset)
{
    m_bump_arena arena;

    m_bump_arena_init(&arena, 64);

    void *a = m_bump_arena_alloc(&arena, 16);

    m_bump_arena_reset(&arena);

    void *b = m_bump_arena_alloc(&arena, 16);

    assert(a == b);

    m_bump_arena_destroy(&arena);
}

M_TEST(m_test_bump_arena_alloc_zero)
{
    m_bump_arena arena;

    m_bump_arena_init(&arena, 64);

    void *p = m_bump_arena_alloc(&arena, 0);

    assert(p == NULL);

    m_bump_arena_destroy(&arena);
}

M_TEST(m_test_bump_arena_destroy_twice)
{
    m_bump_arena arena;

    m_bump_arena_init(&arena, 64);

    int rc1 = m_bump_arena_destroy(&arena);
    int rc2 = m_bump_arena_destroy(&arena);

    assert(rc1 == NO_ERROR);
    assert(rc2 == NO_ERROR);
}

M_TEST(m_test_bump_arena_destroy_null)
{
    int rc = m_bump_arena_destroy(NULL);

    assert(rc == ERR_NULL_PTR);
}
