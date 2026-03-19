#include "kest_test.h"

typedef struct {
    int value;
} dummy;

DECLARE_PTR_LIST(dummy);
IMPLEMENT_PTR_LIST(dummy)

static dummy a = {1};
static dummy b = {2};
static dummy c = {3};

/* ================= BASIC PTR LIST ================= */

KEST_TEST(test_ptr_list_init_basic)
{
    dummy_ptr_list list;

    int r = dummy_ptr_list_init(&list);

    assert(r == NO_ERROR);
    assert(list.entries == NULL);
    assert(list.count == 0);
    assert(list.capacity == 0);
}

KEST_TEST(test_ptr_list_init_with_allocator_null)
{
    dummy_ptr_list list;

    int r = dummy_ptr_list_init_with_allocator(&list, NULL);

    assert(r == NO_ERROR);
    assert(list.entries == NULL);
}

/* ================= RESERVE ================= */

KEST_TEST(test_ptr_list_reserve_basic)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    int r = dummy_ptr_list_reserve(&list, 4);

    assert(r == NO_ERROR);
    assert(list.entries != NULL);
    assert(list.capacity >= 4);
}

KEST_TEST(test_ptr_list_reserve_noop)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    dummy_ptr_list_reserve(&list, 8);
    int old_cap = list.capacity;

    int r = dummy_ptr_list_reserve(&list, 1);

    assert(r == NO_ERROR);
    assert(list.capacity == old_cap);
}

/* ================= APPEND ================= */

KEST_TEST(test_ptr_list_append_single)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    int r = dummy_ptr_list_append(&list, &a);

    assert(r == NO_ERROR);
    assert(list.count == 1);
    assert(list.entries[0] == &a);
}

KEST_TEST(test_ptr_list_append_multiple)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    dummy_ptr_list_append(&list, &a);
    dummy_ptr_list_append(&list, &b);
    dummy_ptr_list_append(&list, &c);

    assert(list.count == 3);
    assert(list.entries[2] == &c);
}

KEST_TEST(test_ptr_list_append_null_list)
{
    assert(dummy_ptr_list_append(NULL, &a) == ERR_NULL_PTR);
}

KEST_TEST(test_ptr_list_append_null_entry)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    int r = dummy_ptr_list_append(&list, NULL);

    assert(r == NO_ERROR);
    assert(list.entries[0] == NULL);
}

/* ================= DESTROY ================= */

static int destructor_call_count = 0;

static void dummy_destructor(dummy *x)
{
    (void)x;
    destructor_call_count++;
}

KEST_TEST(test_ptr_list_destroy_all_basic)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    dummy *x1 = kest_alloc(sizeof(dummy));
    dummy *x2 = kest_alloc(sizeof(dummy));

    dummy_ptr_list_append(&list, x1);
    dummy_ptr_list_append(&list, x2);

    destructor_call_count = 0;

    int r = dummy_ptr_list_destroy_all(&list, dummy_destructor);

    assert(r == NO_ERROR);
    assert(destructor_call_count == 2);
    assert(list.entries == NULL);
}

KEST_TEST(test_ptr_list_destroy_all_no_destructor)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    dummy *x = kest_alloc(sizeof(dummy));
    dummy_ptr_list_append(&list, x);

    int r = dummy_ptr_list_destroy_all(&list, NULL);

    assert(r == NO_ERROR);
    assert(list.entries == NULL);
}

/* ================= CONTAINS ================= */

static dummy d1 = {1};
static dummy d2 = {2};
static dummy d3 = {3};

KEST_TEST(test_ptr_list_contains_basic)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    dummy_ptr_list_append(&list, &d1);
    dummy_ptr_list_append(&list, &d2);

    assert(dummy_ptr_list_contains(&list, &d2) == 1);
    assert(dummy_ptr_list_index_of(&list, &d2) == 1);
}

/* ================= VALUE LIST ================= */

typedef struct {
    int a;
    int b;
} pair;

DECLARE_LIST(pair)
IMPLEMENT_LIST(pair)

static int pair_cmp(const pair *a, const pair *b)
{
    return (a->a == b->a && a->b == b->b) ? 0 : 1;
}

KEST_TEST(test_list_basic)
{
    pair_list list;
    pair_list_init(&list);

    pair x = {1,2};
    pair_list_append(&list, x);

    assert(list.count == 1);
}

KEST_TEST(test_list_memcmp_fallback)
{
    pair_list list;
    pair_list_init(&list);

    pair a = {3,4};
    pair b = {3,4};

    pair_list_append(&list, a);

    assert(pair_list_contains(&list, b, NULL) == 1);
}

/* ================= ALLOCATOR TEST ================= */

typedef struct {
    int allocs;
    int reallocs;
    int frees;
} counting_state;

static void *count_alloc(void *data, size_t sz)
{
    counting_state *s = data;
    s->allocs++;
    return kest_alloc(sz);
}

static void *count_realloc(void *data, void *ptr, size_t sz)
{
    counting_state *s = data;
    s->reallocs++;
    return kest_realloc(ptr, sz);
}

static void count_free(void *data, void *ptr)
{
    counting_state *s = data;
    s->frees++;
    kest_free(ptr);
}

KEST_TEST(test_allocator_usage_basic)
{
    counting_state s = {0};

    kest_allocator alloc = {
        .alloc = count_alloc,
        .realloc = count_realloc,
        .free = count_free,
        .data = &s
    };

    pair_list list;
    pair_list_init_with_allocator(&list, &alloc);

    pair a = {1,1};
    pair b = {2,2};

    pair_list_append(&list, a);
    pair_list_append(&list, b);

    assert(s.allocs >= 1);
    assert(s.reallocs >= 0);

    pair_list_destroy(&list);

    assert(s.frees >= 1);
}

KEST_TEST(test_allocator_realloc_path)
{
    counting_state s = {0};

    kest_allocator alloc = {
        .alloc = count_alloc,
        .realloc = count_realloc,
        .free = count_free,
        .data = &s
    };

    pair_list list;
    pair_list_init_reserved_with_allocator(&list, 1, &alloc);

    pair a = {1,1};
    pair b = {2,2};

    pair_list_append(&list, a);
    pair_list_append(&list, b); /* forces realloc */

    assert(s.reallocs >= 1);

    pair_list_destroy(&list);
}

typedef struct {
    int alloc_calls;
    int realloc_calls;
    int free_calls;
    int fail_alloc_after;
    int fail_realloc_after;
} fail_state;

static void *fail_alloc(void *data, size_t sz)
{
    fail_state *s = data;
    s->alloc_calls++;

    if (s->alloc_calls > s->fail_alloc_after) return NULL;

    return kest_alloc(sz);
}

static void *fail_realloc(void *data, void *ptr, size_t sz)
{
    fail_state *s = data;
    s->realloc_calls++;

    if (s->realloc_calls > s->fail_realloc_after) return NULL;

    return kest_realloc(ptr, sz);
}

static void fail_free(void *data, void *ptr)
{
    fail_state *s = data;
    s->free_calls++;
    kest_free(ptr);
}

/* ================= FAIL ON INITIAL ALLOC ================= */

KEST_TEST(test_allocator_fail_initial_alloc)
{
    fail_state s = {
        .fail_alloc_after = 0,        /* fail immediately */
        .fail_realloc_after = 1000
    };

    kest_allocator alloc = {
        .alloc = fail_alloc,
        .realloc = fail_realloc,
        .free = fail_free,
        .data = &s
    };

    pair_list list;
    pair_list_init_with_allocator(&list, &alloc);

    pair x = {1,1};

    int r = pair_list_append(&list, x);

    assert(r == ERR_ALLOC_FAIL);

    /* state must remain clean */
    assert(list.entries == NULL);
    assert(list.count == 0);
    assert(list.capacity == 0);

    /* verify path */
    assert(s.alloc_calls == 1);
    assert(s.realloc_calls == 0);
}

/* ================= FAIL ON REALLOC ================= */

KEST_TEST(test_allocator_fail_realloc_growth)
{
    fail_state s = {
        .fail_alloc_after = 1000,
        .fail_realloc_after = 0   /* first realloc fails */
    };

    kest_allocator alloc = {
        .alloc = fail_alloc,
        .realloc = fail_realloc,
        .free = fail_free,
        .data = &s
    };

    pair_list list;
    pair_list_init_reserved_with_allocator(&list, 1, &alloc);

    pair a = {1,1};
    pair b = {2,2};

    int r1 = pair_list_append(&list, a);
    assert(r1 == NO_ERROR);

    int old_count = list.count;
    int old_capacity = list.capacity;
    pair old_value = list.entries[0];

    int r2 = pair_list_append(&list, b); /* triggers realloc */

    assert(r2 == ERR_ALLOC_FAIL);

    /* critical invariants: nothing corrupted */
    assert(list.count == old_count);
    assert(list.capacity == old_capacity);
    assert(list.entries[0].a == old_value.a);
    assert(list.entries[0].b == old_value.b);

    /* verify path */
    assert(s.alloc_calls >= 1);
    assert(s.realloc_calls == 1);
}

/* ================= FAIL AFTER SOME SUCCESS ================= */

KEST_TEST(test_allocator_partial_success_then_fail)
{
    fail_state s = {
        .fail_alloc_after = 1000,
        .fail_realloc_after = 1   /* fail on second realloc */
    };

    kest_allocator alloc = {
        .alloc = fail_alloc,
        .realloc = fail_realloc,
        .free = fail_free,
        .data = &s
    };

    pair_list list;
    pair_list_init_with_allocator(&list, &alloc);

    pair a = {1,1};
    pair b = {2,2};
    pair c = {3,3};

    assert(pair_list_append(&list, a) == NO_ERROR);
    assert(pair_list_append(&list, b) == NO_ERROR);

    int r = pair_list_append(&list, c); /* triggers realloc failure */

    assert(r == ERR_ALLOC_FAIL);

    assert(list.count == 2);
    assert(list.entries[0].a == 1);
    assert(list.entries[1].a == 2);

    pair_list_destroy(&list);
}

KEST_TEST(test_ptr_list_head_tail_basic)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    dummy_ptr_list_append(&list, &a);
    dummy_ptr_list_append(&list, &b);

    assert(*dummy_ptr_list_head(&list) == &a);
    assert(*dummy_ptr_list_tail(&list) == &b);
}

KEST_TEST(test_ptr_list_head_tail_empty)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    assert(dummy_ptr_list_head(&list) == NULL);
    assert(dummy_ptr_list_tail(&list) == NULL);
}

KEST_TEST(test_ptr_list_pop_tail_basic)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    dummy_ptr_list_append(&list, &a);
    dummy_ptr_list_append(&list, &b);

    dummy_ptr_list_pop_tail(&list);

    assert(list.count == 1);
    assert(list.entries[0] == &a);
}

KEST_TEST(test_ptr_list_pop_destroy_tail_calls_destructor)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    dummy_ptr_list_append(&list, &a);

    destructor_call_count = 0;

    dummy_ptr_list_pop_destroy_tail(&list, dummy_destructor);

    assert(list.count == 0);
    assert(destructor_call_count == 1);
}

KEST_TEST(test_ptr_list_append_list_basic)
{
    dummy_ptr_list a_list, b_list;

    dummy_ptr_list_init(&a_list);
    dummy_ptr_list_init(&b_list);

    dummy_ptr_list_append(&a_list, &a);
    dummy_ptr_list_append(&a_list, &b);

    dummy_ptr_list_append(&b_list, &c);

    dummy_ptr_list_append_list(&b_list, &a_list);

    assert(b_list.count == 3);
    assert(b_list.entries[1] == &a);
    assert(b_list.entries[2] == &b);
}

KEST_TEST(test_ptr_list_drain_basic)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    dummy_ptr_list_append(&list, &a);
    dummy_ptr_list_append(&list, &b);

    dummy_ptr_list_drain(&list);

    assert(list.count == 0);
    assert(list.entries != NULL); /* capacity preserved */
}

KEST_TEST(test_ptr_list_drain_destroy_calls_destructor)
{
    dummy_ptr_list list;
    dummy_ptr_list_init(&list);

    dummy *x1 = kest_alloc(sizeof(dummy));
    dummy *x2 = kest_alloc(sizeof(dummy));

    dummy_ptr_list_append(&list, x1);
    dummy_ptr_list_append(&list, x2);

    destructor_call_count = 0;

    dummy_ptr_list_drain_destroy(&list, dummy_destructor);

    assert(list.count == 0);
    assert(destructor_call_count == 2);
}

KEST_TEST(test_list_pop_destroy_tail_basic)
{
    pair_list list;
    pair_list_init(&list);

    pair x = {1,2};
    pair_list_append(&list, x);

    int called = 0;
    void d(pair *p) { (void)p; called++; }

    pair_list_pop_destroy_tail(&list, d);

    assert(list.count == 0);
    assert(called == 1);
}

KEST_TEST(test_list_append_list_basic)
{
    pair_list a_list, b_list;
    pair_list_init(&a_list);
    pair_list_init(&b_list);

    pair a = {1,1};
    pair b = {2,2};

    pair_list_append(&a_list, a);
    pair_list_append(&b_list, b);

    pair_list_append_list(&b_list, &a_list);

    assert(b_list.count == 2);
    assert(b_list.entries[1].a == 1);
}

KEST_TEST(test_list_drain_basic)
{
    pair_list list;
    pair_list_init(&list);

    pair x = {1,1};
    pair_list_append(&list, x);

    pair_list_drain(&list);

    assert(list.count == 0);
    assert(list.entries != NULL);
}


