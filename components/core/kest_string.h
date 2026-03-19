#ifndef KEST_STRING_H_
#define KEST_STRING_H_

DECLARE_LIST(char);

typedef char_list kest_string;

int kest_string_init(kest_string *string);
int kest_string_init_from_str(kest_string *string, char *str);
int kest_string_init_with_allocator(kest_string *string, const kest_allocator *alloc);
int kest_string_init_reserved(kest_string *string, size_t n);
int kest_string_init_reserved_with_allocator(kest_string *string, size_t n, const kest_allocator *alloc);
int kest_string_reserve(kest_string *string, size_t n);
int kest_string_append(kest_string *string, char x);
int kest_string_append_int(kest_string *string, int x);
int kest_string_append_str(kest_string *string, const char *str);
int kest_string_concat(kest_string *string, kest_string *a);
int kest_string_destroy(kest_string *string);
int kest_string_contains(kest_string *string, char c);
int kest_string_index_of(kest_string *string, char x);
char *kest_string_head(kest_string *string);
char *kest_string_tail(kest_string *string);
int kest_string_pop_tail(kest_string *string);
int kest_string_drain(kest_string *string);
int kest_string_len(kest_string *string);
int kest_string_write_out(kest_string *string, char *buf, size_t buf_len);
char *kest_string_to_native(kest_string *string);
char *kest_string_to_native_with_allocator(kest_string *string, kest_allocator *alloc);
int kest_string_appendf(kest_string *string, const char *fmt, ...);

#endif
