#include "m_test.h"

M_TEST(test_string_init_from_str_basic)
{
    m_string s;

    int r = m_string_init_from_str(&s, "hello");

    assert(r == NO_ERROR);
    assert(s.count == 5);
    assert(memcmp(s.entries, "hello", 5) == 0);
}

M_TEST(test_string_init_from_str_null)
{
    m_string s;

    int r = m_string_init_from_str(&s, NULL);

    assert(r == NO_ERROR);
    assert(s.count == 0);
    assert(s.entries == NULL);
}

M_TEST(test_string_append_str_basic)
{
    m_string s;
    m_string_init(&s);

    m_string_append_str(&s, "abc");
    m_string_append_str(&s, "def");

    assert(s.count == 6);
    assert(memcmp(s.entries, "abcdef", 6) == 0);
}

M_TEST(test_string_append_str_null)
{
    m_string s;
    m_string_init(&s);

    int r = m_string_append_str(&s, NULL);

    assert(r == NO_ERROR);
    assert(s.count == 0);
}

M_TEST(test_string_append_int_basic)
{
    m_string s;
    m_string_init(&s);

    m_string_append_int(&s, 123);

    assert(s.count == 3);
    assert(memcmp(s.entries, "123", 3) == 0);
}

M_TEST(test_string_append_int_negative)
{
    m_string s;
    m_string_init(&s);

    m_string_append_int(&s, -42);

    assert(memcmp(s.entries, "-42", 3) == 0);
}

M_TEST(test_string_concat_basic)
{
    m_string a, b;

    m_string_init_from_str(&a, "foo");
    m_string_init_from_str(&b, "bar");

    m_string_concat(&a, &b);

    assert(a.count == 6);
    assert(memcmp(a.entries, "foobar", 6) == 0);
}

M_TEST(test_string_concat_empty)
{
    m_string a, b;

    m_string_init_from_str(&a, "abc");
    m_string_init(&b);

    m_string_concat(&a, &b);

    assert(a.count == 3);
    assert(memcmp(a.entries, "abc", 3) == 0);
}

M_TEST(test_string_len_basic)
{
    m_string s;
    m_string_init_from_str(&s, "hello");

    assert(m_string_len(&s) == 5);
}

M_TEST(test_string_contains_index)
{
    m_string s;
    m_string_init_from_str(&s, "abcde");

    assert(m_string_contains(&s, 'c') == 1);
    assert(m_string_index_of(&s, 'c') == 2);

    assert(m_string_contains(&s, 'z') == 0);
    assert(m_string_index_of(&s, 'z') == -1);
}

M_TEST(test_string_head_tail)
{
    m_string s;
    m_string_init_from_str(&s, "xyz");

    assert(*m_string_head(&s) == 'x');
    assert(*m_string_tail(&s) == 'z');
}

M_TEST(test_string_head_tail_empty)
{
    m_string s;
    m_string_init(&s);

    assert(m_string_head(&s) == NULL);
    assert(m_string_tail(&s) == NULL);
}

M_TEST(test_string_pop_tail)
{
    m_string s;
    m_string_init_from_str(&s, "abc");

    m_string_pop_tail(&s);

    assert(s.count == 2);
    assert(memcmp(s.entries, "ab", 2) == 0);
}

M_TEST(test_string_drain)
{
    m_string s;
    m_string_init_from_str(&s, "abc");

    m_string_drain(&s);

    assert(s.count == 0);
    assert(s.entries != NULL);
}

M_TEST(test_m_string_write_out_basic)
{
    m_string s;
    m_string_init_from_str(&s, "hello");

    char buf[16];

    int r = m_string_write_out(&s, buf, sizeof(buf));

    assert(r == NO_ERROR);
    assert(strcmp(buf, "hello") == 0);
}

M_TEST(test_m_string_write_out_truncate)
{
    m_string s;
    m_string_init_from_str(&s, "abcdef");

    char buf[4]; // can hold 3 chars + null

    int r = m_string_write_out(&s, buf, sizeof(buf));

    assert(r == NO_ERROR);
    assert(strcmp(buf, "abc") == 0);
}

M_TEST(test_m_string_write_out_empty)
{
    m_string s;
    m_string_init(&s);

    char buf[8];

    int r = m_string_write_out(&s, buf, sizeof(buf));

    assert(r == NO_ERROR);
    assert(buf[0] == '\0');
}

M_TEST(test_m_string_write_out_null_string)
{
    char buf[8];

    int r = m_string_write_out(NULL, buf, sizeof(buf));

    assert(r == ERR_NULL_PTR);
}

M_TEST(test_m_string_write_out_null_buf)
{
    m_string s;
    m_string_init(&s);

    int r = m_string_write_out(&s, NULL, 8);

    assert(r == ERR_NULL_PTR);
}

M_TEST(test_m_string_write_out_zero_buf_len)
{
    m_string s;
    m_string_init_from_str(&s, "hi");

    char buf[1];

    int r = m_string_write_out(&s, buf, 0);

    assert(r == ERR_BAD_ARGS);
}

M_TEST(test_m_string_to_native_basic)
{
    m_string s;
    m_string_init_from_str(&s, "hello");

    char *out = m_string_to_native(&s, NULL);

    assert(out != NULL);
    assert(strcmp(out, "hello") == 0);

    m_free(out);
}

M_TEST(test_m_string_to_native_empty)
{
    m_string s;
    m_string_init(&s);

    char *out = m_string_to_native(&s, NULL);

    assert(strcmp(out, "") == 0);
}

M_TEST(test_m_string_to_native_null_string)
{
    char *out = m_string_to_native(NULL, NULL);

    assert(out == NULL);
}

M_TEST(test_m_string_appendf_basic)
{
    m_string s;
    m_string_init(&s);

    int r = m_string_appendf(&s, "hello %d", 42);

    assert(r == NO_ERROR);

    char buf[32];
    m_string_write_out(&s, buf, sizeof(buf));

    assert(strcmp(buf, "hello 42") == 0);
}

M_TEST(test_m_string_appendf_multiple_calls)
{
    m_string s;
    m_string_init(&s);

    m_string_appendf(&s, "a=%d ", 1);
    m_string_appendf(&s, "b=%d", 2);

    char buf[32];
    m_string_write_out(&s, buf, sizeof(buf));

    assert(strcmp(buf, "a=1 b=2") == 0);
}

M_TEST(test_m_string_appendf_null_string)
{
    int r = m_string_appendf(NULL, "x=%d", 1);
    assert(r == ERR_NULL_PTR);
}

M_TEST(test_m_string_appendf_null_fmt)
{
    m_string s;
    m_string_init(&s);

    int r = m_string_appendf(&s, NULL);
    assert(r == ERR_NULL_PTR);
}
