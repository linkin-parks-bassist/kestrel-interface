#include "kest_int.h"

IMPLEMENT_LIST(char);

int char_cmp(const char *a, const char *b)
{
	if (!a || !b) return -1;
	if (*a == *b) return 0;
	return 1;
}

int kest_string_init(kest_string *string)
{
	return char_list_init(string);
}

int kest_string_init_from_str(kest_string *string, char *str)
{
	if (!str)
		return kest_string_init(string);
	
	int len = strlen(str);
	
	int ret_val = kest_string_init_reserved(string, len);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	memcpy(string->entries, str, len);
	
	string->count = len;
	
	return NO_ERROR;
}

int kest_string_init_with_allocator(kest_string *string, const kest_allocator *alloc)
{
	return char_list_init_with_allocator(string, alloc);
}

int kest_string_init_reserved(kest_string *string, size_t n)
{
	return char_list_init_reserved(string, n);
}

int kest_string_init_reserved_with_allocator(kest_string *string, size_t n, const kest_allocator *alloc)
{
	return char_list_init_reserved_with_allocator(string, n, alloc);
}

int kest_string_reserve(kest_string *string, size_t n)
{
	return char_list_reserve(string, n);
}

int kest_string_append(kest_string *string, char x)
{
	return char_list_append(string, x);
}

int kest_string_append_int(kest_string *string, int x)
{
	if (!string)
		return ERR_NULL_PTR;
	
	char buf[128];
	
	snprintf(buf, 128, "%d", x);
	
	return kest_string_append_str(string, buf);
}

int kest_string_append_str(kest_string *string, const char *str)
{
	if (!string)
		return ERR_NULL_PTR;
	
	if (!str)
		return NO_ERROR;
	
	int len = strlen(str);
	
	char_list_reserve(string, len);
	
	memcpy(&string->entries[string->count], str, sizeof(char) * len);
	
	string->count += len;
	
	return NO_ERROR;
}

int kest_string_concat(kest_string *a, kest_string *b)
{
	return char_list_append_list(a, b);
}

int kest_string_destroy(kest_string *string)
{
	return char_list_destroy(string);
}

int kest_string_contains(kest_string *string, char c)
{
	return char_list_contains(string, c, char_cmp);
}

int kest_string_index_of(kest_string *string, char x)
{
	return char_list_index_of(string, x, char_cmp);
}

char *kest_string_head(kest_string *string)
{
	return char_list_head(string);
}

char *kest_string_tail(kest_string *string)
{
	return char_list_tail(string);
}

int kest_string_pop_tail(kest_string *string)
{
	return char_list_pop_tail(string);
}

int kest_string_drain(kest_string *string)
{
	return char_list_drain(string);
}

int kest_string_len(kest_string *string)
{
	return string->count;
}

int kest_string_write_out(kest_string *string, char *buf, size_t buf_len)
{
	if (!buf)
		return ERR_NULL_PTR;
	
	if (buf_len == 0)
		return ERR_BAD_ARGS;
	
	buf[0] = 0;
	
	if (!string)
		return ERR_NULL_PTR;
	
	if (!string->count)
		return NO_ERROR;
	
	if (!string->entries)
		return ERR_BAD_ARGS;
	
	size_t len = string->count;
	
	if (len > buf_len - 1)
		len = buf_len - 1;
	
	memcpy(buf, string->entries, len);
	buf[len] = 0;
	
	return NO_ERROR;
}

char *kest_string_to_native(kest_string *string)
{
	if (!string)
		return NULL;
	
	size_t len = string->count;
	
	char *result = kest_alloc(len + 1);
	
	if (!result)
		return NULL;
	
	if (string->entries)
		memcpy(result, string->entries, len);
	else
		result[0] = 0;
	
	result[len] = 0;
	
	return result;
}

char *kest_string_to_native_with_allocator(kest_string *string, kest_allocator *alloc)
{
	if (!string)
		return NULL;
	
	size_t len = string->count;
	
	char *result = kest_allocator_alloc(alloc, len + 1);
	
	if (!result)
		return NULL;
	
	if (string->entries)
		memcpy(result, string->entries, len);
	else
		result[0] = 0;
	
	result[len] = 0;
	
	return result;
}

int kest_string_appendf(kest_string *string, const char *fmt, ...)
{
	if (!string || !fmt)
		return ERR_NULL_PTR;
	
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	int n = vsnprintf(buf, 1024, fmt, args);
	va_end(args);
	
	if (n < 0)
		return ERR_BAD_ARGS;
	
	kest_string_append_str(string, buf);
	
	return NO_ERROR;
}
