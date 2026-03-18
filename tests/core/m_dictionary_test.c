#include "m_test.h"

typedef struct { int v; } dummy;
DECLARE_PTR_DICT(dummy);
IMPLEMENT_PTR_DICT(dummy);

M_TEST(test_ptr_dict_init_basic)
{
    dummy_ptr_dict dict;

    int r = dummy_ptr_dict_init(&dict, 8);

    assert(r == NO_ERROR);
    assert(dict.n_buckets == 8);
    assert(dict.buckets != NULL);

    for (size_t i = 0; i < dict.n_buckets; i++)
    {
        assert(dict.buckets[i].entries == NULL);
        assert(dict.buckets[i].count == 0);
        assert(dict.buckets[i].capacity == 0);
    }
}

M_TEST(test_ptr_dict_insert_duplicate)
{
    dummy_ptr_dict dict;
    dummy a = { .v = 1 };
    dummy b = { .v = 2 };

    dummy_ptr_dict_init(&dict, 8);

    int r1 = dummy_ptr_dict_insert(&dict, "key", &a);
    int r2 = dummy_ptr_dict_insert(&dict, "key", &b);

    assert(r1 == NO_ERROR);
    assert(r2 == ERR_DUPLICATE_KEY);

    dummy *out = dummy_ptr_dict_lookup(&dict, "key");
    assert(out == &a);
}

M_TEST(test_ptr_dict_insert_null_dict)
{
    dummy x = {0};

    int r = dummy_ptr_dict_insert(NULL, "k", &x);
    assert(r == ERR_NULL_PTR);
}

M_TEST(test_ptr_dict_insert_null_key)
{
    dummy_ptr_dict dict;
    dummy x = {0};

    dummy_ptr_dict_init(&dict, 8);

    int r = dummy_ptr_dict_insert(&dict, NULL, &x);
    assert(r == ERR_BAD_ARGS);
}

M_TEST(test_ptr_dict_insert_zero_buckets)
{
    dummy_ptr_dict dict;
    dummy x = {0};

    dummy_ptr_dict_init(&dict, 0);

    int r = dummy_ptr_dict_insert(&dict, "k", &x);
    assert(r == ERR_BAD_ARGS);
}

M_TEST(test_ptr_dict_count_empty)
{
    dummy_ptr_dict dict;
    dummy_ptr_dict_init(&dict, 8);

    size_t c = dummy_ptr_dict_count(&dict);
    assert(c == 0);
}

M_TEST(test_ptr_dict_count_multiple)
{
    dummy_ptr_dict dict;
    dummy a = {1}, b = {2}, c = {3};

    dummy_ptr_dict_init(&dict, 4);

    dummy_ptr_dict_insert(&dict, "a", &a);
    dummy_ptr_dict_insert(&dict, "b", &b);
    dummy_ptr_dict_insert(&dict, "c", &c);

    size_t count = dummy_ptr_dict_count(&dict);
    assert(count == 3);
}

M_TEST(test_ptr_dict_index_basic)
{
    dummy_ptr_dict dict;
    dummy a = {1}, b = {2};

    dummy_ptr_dict_init(&dict, 4);

    dummy_ptr_dict_insert(&dict, "a", &a);
    dummy_ptr_dict_insert(&dict, "b", &b);

    dummy *x0 = dummy_ptr_dict_index(&dict, 0);
    dummy *x1 = dummy_ptr_dict_index(&dict, 1);

    assert(x0 != NULL);
    assert(x1 != NULL);
    assert(x0 == &a || x0 == &b);
    assert(x1 == &a || x1 == &b);
    assert(x0 != x1);
}

M_TEST(test_ptr_dict_index_out_of_bounds)
{
    dummy_ptr_dict dict;
    dummy a = {1};

    dummy_ptr_dict_init(&dict, 4);
    dummy_ptr_dict_insert(&dict, "a", &a);

    assert(dummy_ptr_dict_index(&dict, 1) == NULL);
    assert(dummy_ptr_dict_index(&dict, 100) == NULL);
}

M_TEST(test_ptr_dict_index_empty)
{
    dummy_ptr_dict dict;
    dummy_ptr_dict_init(&dict, 4);

    assert(dummy_ptr_dict_index(&dict, 0) == NULL);
}

static int destroy_counter = 0;

static void dummy_destructor(dummy *x)
{
    (void)x;
    destroy_counter++;
}

M_TEST(test_ptr_dict_destroy_calls_destructor)
{
    dummy_ptr_dict dict;
    dummy a = {1}, b = {2};

    destroy_counter = 0;

    dummy_ptr_dict_init(&dict, 4);
    dummy_ptr_dict_insert(&dict, "a", &a);
    dummy_ptr_dict_insert(&dict, "b", &b);

    dummy_ptr_dict_destroy(&dict, dummy_destructor);

    assert(destroy_counter == 2);
    assert(dict.buckets == NULL);
    assert(dict.n_buckets == 0);
}

M_TEST(test_ptr_dict_destroy_null_destructor)
{
    dummy_ptr_dict dict;
    dummy a = {1};

    dummy_ptr_dict_init(&dict, 4);
    dummy_ptr_dict_insert(&dict, "a", &a);

    dummy_ptr_dict_destroy(&dict, NULL);

    assert(dict.buckets == NULL);
    assert(dict.n_buckets == 0);
}

M_TEST(test_ptr_dict_destroy_empty)
{
    dummy_ptr_dict dict;

    dummy_ptr_dict_init(&dict, 4);
    dummy_ptr_dict_destroy(&dict, NULL);

    assert(dict.buckets == NULL);
    assert(dict.n_buckets == 0);
}

M_TEST(test_ptr_dict_index_matches_count)
{
    dummy_ptr_dict dict;
    dummy a = {1}, b = {2}, c = {3};

    dummy_ptr_dict_init(&dict, 1); // force single bucket

    dummy_ptr_dict_insert(&dict, "a", &a);
    dummy_ptr_dict_insert(&dict, "b", &b);
    dummy_ptr_dict_insert(&dict, "c", &c);

    size_t count = dummy_ptr_dict_count(&dict);

    for (size_t i = 0; i < count; i++)
    {
        dummy *x = dummy_ptr_dict_index(&dict, i);
        assert(x != NULL);
    }

    assert(dummy_ptr_dict_index(&dict, count) == NULL);
}
