#include "kest_test.h"
#include "kest_pool.h"

typedef struct {
    int value;
    int init_count;
    int deinit_count;
} pool_dummy;

DECLARE_POOL(pool_dummy)
IMPLEMENT_POOL(pool_dummy)

static void pool_dummy_init(pool_dummy *x)
{
    x->value = 123;
    x->init_count++;
}

static void pool_dummy_deinit(pool_dummy *x)
{
    x->deinit_count++;
}

static void pool_dummy_copy(pool_dummy *dest, pool_dummy *src)
{
    dest->value = src->value;
    dest->init_count = src->init_count;
    dest->deinit_count = src->deinit_count;
}

/* ================= INIT / RESERVE ================= */

KEST_TEST(test_pool_init_basic)
{
    pool_dummy_pool pool;

    int r = pool_dummy_pool_init(&pool);

    assert(r == NO_ERROR);
    assert(pool.size == 0);
    assert(pool.free_count == 0);
    assert(pool.obtain_head == 0);
    assert(pool.return_head == 0);
    assert(pool.entries == NULL);
    assert(pool.buffer == NULL);
    assert(pool.init_function == NULL);
    assert(pool.deinit_function == NULL);
    assert(pool.copy_function == NULL);
}

KEST_TEST(test_pool_init_null)
{
    assert(pool_dummy_pool_init(NULL) == ERR_NULL_PTR);
}

KEST_TEST(test_pool_reserve_basic)
{
    pool_dummy_pool pool;
    pool_dummy_pool_init(&pool);

    int r = pool_dummy_pool_reserve(&pool, 4);

    assert(r == NO_ERROR);
    assert(pool.size == 4);
    assert(pool.free_count == 4);
    assert(pool.obtain_head == 0);
    assert(pool.return_head == 3);
    assert(pool.entries != NULL);
    assert(pool.buffer != NULL);

    for (size_t i = 0; i < 4; i++)
        assert(pool.buffer[i] == &pool.entries[i]);
}

KEST_TEST(test_pool_reserve_zero)
{
    pool_dummy_pool pool;
    pool_dummy_pool_init(&pool);

    int r = pool_dummy_pool_reserve(&pool, 0);

    assert(r == NO_ERROR);
    assert(pool.size == 0);
    assert(pool.free_count == 0);
    assert(pool.entries == NULL);
    assert(pool.buffer == NULL);
}

/* ================= OBTAIN ================= */

KEST_TEST(test_pool_obtain_single)
{
    pool_dummy_pool pool;
    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 2);

    pool_dummy *x = pool_dummy_pool_obtain(&pool);

    assert(x != NULL);
    assert(x == &pool.entries[0]);
    assert(pool.free_count == 1);
    assert(pool.obtain_head == 1);
}

KEST_TEST(test_pool_obtain_all_then_empty)
{
    pool_dummy_pool pool;
    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 3);

    pool_dummy *x0 = pool_dummy_pool_obtain(&pool);
    pool_dummy *x1 = pool_dummy_pool_obtain(&pool);
    pool_dummy *x2 = pool_dummy_pool_obtain(&pool);
    pool_dummy *x3 = pool_dummy_pool_obtain(&pool);

    assert(x0 == &pool.entries[0]);
    assert(x1 == &pool.entries[1]);
    assert(x2 == &pool.entries[2]);
    assert(x3 == NULL);

    assert(pool.free_count == 0);
    assert(pool.obtain_head == 0);
}

KEST_TEST(test_pool_obtain_unreserved)
{
    pool_dummy_pool pool;
    pool_dummy_pool_init(&pool);

    assert(pool_dummy_pool_obtain(&pool) == NULL);
}

KEST_TEST(test_pool_obtain_null)
{
    assert(pool_dummy_pool_obtain(NULL) == NULL);
}

/* ================= RETURN ================= */

KEST_TEST(test_pool_return_single)
{
    pool_dummy_pool pool;
    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 2);

    pool_dummy *x = pool_dummy_pool_obtain(&pool);

    int r = pool_dummy_pool_return(&pool, x);

    assert(r == NO_ERROR);
    assert(pool.free_count == 2);
    assert(pool.return_head == 0);
    assert(pool.buffer[0] == x);
}

KEST_TEST(test_pool_obtain_return_obtain_same_pointer)
{
    pool_dummy_pool pool;
    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 1);

    pool_dummy *x = pool_dummy_pool_obtain(&pool);
    assert(x != NULL);

    assert(pool_dummy_pool_return(&pool, x) == NO_ERROR);

    pool_dummy *y = pool_dummy_pool_obtain(&pool);

    assert(y == x);
    assert(pool.free_count == 0);
}

KEST_TEST(test_pool_fifo_reuse_order)
{
    pool_dummy_pool pool;
    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 3);

    pool_dummy *a = pool_dummy_pool_obtain(&pool);
    pool_dummy *b = pool_dummy_pool_obtain(&pool);
    pool_dummy *c = pool_dummy_pool_obtain(&pool);

    assert(pool_dummy_pool_return(&pool, b) == NO_ERROR);
    assert(pool_dummy_pool_return(&pool, a) == NO_ERROR);
    assert(pool_dummy_pool_return(&pool, c) == NO_ERROR);

    assert(pool_dummy_pool_obtain(&pool) == b);
    assert(pool_dummy_pool_obtain(&pool) == a);
    assert(pool_dummy_pool_obtain(&pool) == c);
    assert(pool_dummy_pool_obtain(&pool) == NULL);
}

KEST_TEST(test_pool_return_null_pool)
{
    pool_dummy x = {0};

    assert(pool_dummy_pool_return(NULL, &x) == ERR_NULL_PTR);
}

KEST_TEST(test_pool_return_null_entry)
{
    pool_dummy_pool pool;
    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 1);

    assert(pool_dummy_pool_return(&pool, NULL) == ERR_NULL_PTR);
}

KEST_TEST(test_pool_return_unreserved)
{
    pool_dummy_pool pool;
    pool_dummy x = {0};

    pool_dummy_pool_init(&pool);

    assert(pool_dummy_pool_return(&pool, &x) == ERR_BAD_ARGS);
}

KEST_TEST(test_pool_return_when_full)
{
    pool_dummy_pool pool;
    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 1);

    assert(pool_dummy_pool_return(&pool, &pool.entries[0]) == ERR_BAD_ARGS);
}

/* ================= OBTAIN N ================= */

KEST_TEST(test_pool_obtain_n_basic)
{
    pool_dummy_pool pool;
    pool_dummy *buf[3];

    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 4);

    size_t n = pool_dummy_pool_obtain_n(&pool, 3, buf);

    assert(n == 3);
    assert(buf[0] == &pool.entries[0]);
    assert(buf[1] == &pool.entries[1]);
    assert(buf[2] == &pool.entries[2]);
    assert(pool.free_count == 1);
}

KEST_TEST(test_pool_obtain_n_partial)
{
    pool_dummy_pool pool;
    pool_dummy *buf[4];

    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 2);

    size_t n = pool_dummy_pool_obtain_n(&pool, 4, buf);

    assert(n == 2);
    assert(buf[0] == &pool.entries[0]);
    assert(buf[1] == &pool.entries[1]);
    assert(pool.free_count == 0);
}

KEST_TEST(test_pool_obtain_n_nulls)
{
    pool_dummy_pool pool;
    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 2);

    assert(pool_dummy_pool_obtain_n(NULL, 1, NULL) == 0);
    assert(pool_dummy_pool_obtain_n(&pool, 1, NULL) == 0);
}

/* ================= ALLOCATOR WRAPPER ================= */

KEST_TEST(test_pool_init_allocator_basic)
{
    pool_dummy_pool pool;
    kest_allocator alloc;

    pool_dummy_pool_init(&pool);

    int r = pool_dummy_pool_init_allocator(&pool, &alloc);

    assert(r == NO_ERROR);
    assert(alloc.data == &pool);
    assert(alloc.alloc == pool_dummy_pool_alloc);
    assert(alloc.realloc == pool_dummy_pool_realloc);
    assert(alloc.free == pool_dummy_pool_free);
}

KEST_TEST(test_pool_init_allocator_nulls)
{
    pool_dummy_pool pool;
    kest_allocator alloc;

    pool_dummy_pool_init(&pool);

    assert(pool_dummy_pool_init_allocator(NULL, &alloc) == ERR_NULL_PTR);
    assert(pool_dummy_pool_init_allocator(&pool, NULL) == ERR_NULL_PTR);
}

KEST_TEST(test_pool_allocator_alloc_basic)
{
    pool_dummy_pool pool;
    kest_allocator alloc;

    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 1);
    pool_dummy_pool_init_allocator(&pool, &alloc);

    pool_dummy *x = kest_allocator_alloc(&alloc, 1);

    assert(x != NULL);
    assert(x == &pool.entries[0]);
    assert(pool.free_count == 0);
}

KEST_TEST(test_pool_allocator_alloc_bad_count)
{
    pool_dummy_pool pool;
    kest_allocator alloc;

    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 1);
    pool_dummy_pool_init_allocator(&pool, &alloc);

    assert(kest_allocator_alloc(&alloc, 2) == NULL);
    assert(pool.free_count == 1);
}

KEST_TEST(test_pool_allocator_free_basic)
{
    pool_dummy_pool pool;
    kest_allocator alloc;

    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 1);
    pool_dummy_pool_init_allocator(&pool, &alloc);

    pool_dummy *x = kest_allocator_alloc(&alloc, 1);

    kest_allocator_free(&alloc, x);

    assert(pool.free_count == 1);
}

KEST_TEST(test_pool_allocator_realloc_basic_memcpy_fallback)
{
    pool_dummy_pool pool;
    kest_allocator alloc;

    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 2);
    pool_dummy_pool_init_allocator(&pool, &alloc);

    pool_dummy *x = kest_allocator_alloc(&alloc, 1);
    x->value = 777;
    x->init_count = 3;
    x->deinit_count = 4;

    pool_dummy *y = kest_allocator_realloc(&alloc, x, 1);

    assert(y != NULL);
    assert(y != x);
    assert(y->value == 777);
    assert(y->init_count == 3);
    assert(y->deinit_count == 4);

    assert(pool.free_count == 1);
}

KEST_TEST(test_pool_allocator_realloc_copy_function)
{
    pool_dummy_pool pool;
    kest_allocator alloc;

    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 2);

    pool.copy_function = pool_dummy_copy;

    pool_dummy_pool_init_allocator(&pool, &alloc);

    pool_dummy *x = kest_allocator_alloc(&alloc, 1);
    x->value = 555;
    x->init_count = 6;
    x->deinit_count = 7;

    pool_dummy *y = kest_allocator_realloc(&alloc, x, 1);

    assert(y != NULL);
    assert(y != x);
    assert(y->value == 555);
    assert(y->init_count == 6);
    assert(y->deinit_count == 7);

    assert(pool.free_count == 1);
}

KEST_TEST(test_pool_allocator_realloc_bad_count)
{
    pool_dummy_pool pool;
    kest_allocator alloc;

    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 2);
    pool_dummy_pool_init_allocator(&pool, &alloc);

    pool_dummy *x = kest_allocator_alloc(&alloc, 1);

    assert(kest_allocator_realloc(&alloc, x, 2) == NULL);
}

KEST_TEST(test_pool_allocator_realloc_full_pool_fails)
{
    pool_dummy_pool pool;
    kest_allocator alloc;

    pool_dummy_pool_init(&pool);
    pool_dummy_pool_reserve(&pool, 1);
    pool_dummy_pool_init_allocator(&pool, &alloc);

    pool_dummy *x = kest_allocator_alloc(&alloc, 1);
    x->value = 42;

    pool_dummy *y = kest_allocator_realloc(&alloc, x, 1);

    assert(y == NULL);
    assert(pool.free_count == 0);
    assert(x->value == 42);
}
