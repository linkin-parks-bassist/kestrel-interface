#ifndef M_STRING_H_
#define M_STRING_H_

DECLARE_LIST(char);

typedef char_list m_string;

int m_string_init(m_string *string);
int m_string_init_from_str(m_string *string, char *str);
int m_string_init_with_allocator(m_string *string, const m_allocator *alloc);
int m_string_init_reserved(m_string *string, size_t n);
int m_string_init_reserved_with_allocator(m_string *string, size_t n, const m_allocator *alloc);
int m_string_reserve(m_string *string, size_t n);
int m_string_append(m_string *string, char x);
int m_string_append_int(m_string *string, int x);
int m_string_append_str(m_string *string, const char *str);
int m_string_concat(m_string *string, m_string *a);
int m_string_destroy(m_string *string);
int m_string_contains(m_string *string, char c);
int m_string_index_of(m_string *string, char x);
char *m_string_head(m_string *string);
char *m_string_tail(m_string *string);
int m_string_pop_tail(m_string *string);
int m_string_drain(m_string *string);
int m_string_len(m_string *string);
int m_string_write_out(m_string *string, char *buf, size_t buf_len);
char *m_string_to_native(m_string *string);
char *m_string_to_native_with_allocator(m_string *string, m_allocator *alloc);
int m_string_appendf(m_string *string, const char *fmt, ...);

#endif
