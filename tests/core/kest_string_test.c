#include "kest_test.h"

KEST_TEST(test_string_init_from_str_basic)
{
    kest_string s;

    int r = kest_string_init_from_str(&s, "hello");

    assert(r == NO_ERROR);
    assert(s.count == 5);
    assert(memcmp(s.entries, "hello", 5) == 0);
}

KEST_TEST(test_string_init_from_str_null)
{
    kest_string s;

    int r = kest_string_init_from_str(&s, NULL);

    assert(r == NO_ERROR);
    assert(s.count == 0);
    assert(s.entries == NULL);
}

KEST_TEST(test_string_append_str_basic)
{
    kest_string s;
    kest_string_init(&s);

    kest_string_append_str(&s, "abc");
    kest_string_append_str(&s, "def");

    assert(s.count == 6);
    assert(memcmp(s.entries, "abcdef", 6) == 0);
}

KEST_TEST(test_string_append_str_null)
{
    kest_string s;
    kest_string_init(&s);

    int r = kest_string_append_str(&s, NULL);

    assert(r == NO_ERROR);
    assert(s.count == 0);
}

KEST_TEST(test_string_append_int_basic)
{
    kest_string s;
    kest_string_init(&s);

    kest_string_append_int(&s, 123);

    assert(s.count == 3);
    assert(memcmp(s.entries, "123", 3) == 0);
}

KEST_TEST(test_string_append_int_negative)
{
    kest_string s;
    kest_string_init(&s);

    kest_string_append_int(&s, -42);

    assert(memcmp(s.entries, "-42", 3) == 0);
}

KEST_TEST(test_string_concat_basic)
{
    kest_string a, b;

    kest_string_init_from_str(&a, "foo");
    kest_string_init_from_str(&b, "bar");

    kest_string_concat(&a, &b);

    assert(a.count == 6);
    assert(memcmp(a.entries, "foobar", 6) == 0);
}

KEST_TEST(test_string_concat_empty)
{
    kest_string a, b;

    kest_string_init_from_str(&a, "abc");
    kest_string_init(&b);

    kest_string_concat(&a, &b);

    assert(a.count == 3);
    assert(memcmp(a.entries, "abc", 3) == 0);
}

KEST_TEST(test_string_len_basic)
{
    kest_string s;
    kest_string_init_from_str(&s, "hello");

    assert(kest_string_len(&s) == 5);
}

KEST_TEST(test_string_contains_index)
{
    kest_string s;
    kest_string_init_from_str(&s, "abcde");

    assert(kest_string_contains(&s, 'c') == 1);
    assert(kest_string_index_of(&s, 'c') == 2);

    assert(kest_string_contains(&s, 'z') == 0);
    assert(kest_string_index_of(&s, 'z') == -1);
}

KEST_TEST(test_string_head_tail)
{
    kest_string s;
    kest_string_init_from_str(&s, "xyz");

    assert(*kest_string_head(&s) == 'x');
    assert(*kest_string_tail(&s) == 'z');
}

KEST_TEST(test_string_head_tail_empty)
{
    kest_string s;
    kest_string_init(&s);

    assert(kest_string_head(&s) == NULL);
    assert(kest_string_tail(&s) == NULL);
}

KEST_TEST(test_string_pop_tail)
{
    kest_string s;
    kest_string_init_from_str(&s, "abc");

    kest_string_pop_tail(&s);

    assert(s.count == 2);
    assert(memcmp(s.entries, "ab", 2) == 0);
}

KEST_TEST(test_string_drain)
{
    kest_string s;
    kest_string_init_from_str(&s, "abc");

    kest_string_drain(&s);

    assert(s.count == 0);
    assert(s.entries != NULL);
}

KEST_TEST(test_m_string_write_out_basic)
{
    kest_string s;
    kest_string_init_from_str(&s, "hello");

    char buf[16];

    int r = kest_string_write_out(&s, buf, sizeof(buf));

    assert(r == NO_ERROR);
    assert(strcmp(buf, "hello") == 0);
}

KEST_TEST(test_m_string_write_out_truncate)
{
    kest_string s;
    kest_string_init_from_str(&s, "abcdef");

    char buf[4]; // can hold 3 chars + null

    int r = kest_string_write_out(&s, buf, sizeof(buf));

    assert(r == NO_ERROR);
    assert(strcmp(buf, "abc") == 0);
}

KEST_TEST(test_m_string_write_out_empty)
{
    kest_string s;
    kest_string_init(&s);

    char buf[8];

    int r = kest_string_write_out(&s, buf, sizeof(buf));

    assert(r == NO_ERROR);
    assert(buf[0] == '\0');
}

KEST_TEST(test_m_string_write_out_null_string)
{
    char buf[8];

    int r = kest_string_write_out(NULL, buf, sizeof(buf));

    assert(r == ERR_NULL_PTR);
}

KEST_TEST(test_m_string_write_out_null_buf)
{
    kest_string s;
    kest_string_init(&s);

    int r = kest_string_write_out(&s, NULL, 8);

    assert(r == ERR_NULL_PTR);
}

KEST_TEST(test_m_string_write_out_zero_buf_len)
{
    kest_string s;
    kest_string_init_from_str(&s, "hi");

    char buf[1];

    int r = kest_string_write_out(&s, buf, 0);

    assert(r == ERR_BAD_ARGS);
}

KEST_TEST(test_m_string_to_native_basic)
{
    kest_string s;
    kest_string_init_from_str(&s, "hello");

    char *out = kest_string_to_native(&s);

    assert(out != NULL);
    assert(strcmp(out, "hello") == 0);

    kest_free(out);
}

KEST_TEST(test_m_string_to_native_empty)
{
    kest_string s;
    kest_string_init(&s);

    char *out = kest_string_to_native(&s);

    assert(strcmp(out, "") == 0);
}

KEST_TEST(test_m_string_to_native_null_string)
{
    char *out = kest_string_to_native(NULL);

    assert(out == NULL);
}

KEST_TEST(test_m_string_appendf_basic)
{
    kest_string s;
    kest_string_init(&s);

    int r = kest_string_appendf(&s, "hello %d", 42);

    assert(r == NO_ERROR);

    char buf[32];
    kest_string_write_out(&s, buf, sizeof(buf));

    assert(strcmp(buf, "hello 42") == 0);
}

KEST_TEST(test_m_string_appendf_multiple_calls)
{
    kest_string s;
    kest_string_init(&s);

    kest_string_appendf(&s, "a=%d ", 1);
    kest_string_appendf(&s, "b=%d", 2);

    char buf[32];
    kest_string_write_out(&s, buf, sizeof(buf));

    assert(strcmp(buf, "a=1 b=2") == 0);
}

KEST_TEST(test_m_string_appendf_null_string)
{
    int r = kest_string_appendf(NULL, "x=%d", 1);
    assert(r == ERR_NULL_PTR);
}

KEST_TEST(test_m_string_appendf_null_fmt)
{
    kest_string s;
    kest_string_init(&s);

    int r = kest_string_appendf(&s, NULL);
    assert(r == ERR_NULL_PTR);
}
