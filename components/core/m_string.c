#include "m_int.h"

IMPLEMENT_LIST(char);

int char_cmp(const char *a, const char *b)
{
	if (!a || !b) return -1;
	if (*a == *b) return 0;
	return 1;
}

int m_string_init(m_string *string)
{
	return char_list_init(string);
}

int m_string_init_from_str(m_string *string, char *str)
{
	if (!str)
		return m_string_init(string);
	
	int len = strlen(str);
	
	int ret_val = m_string_init_reserved(string, len);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	
	memcpy(string->entries, str, len);
	
	string->count = len;
	
	return NO_ERROR;
}

int m_string_init_with_allocator(m_string *string, const m_allocator *alloc)
{
	return char_list_init_with_allocator(string, alloc);
}

int m_string_init_reserved(m_string *string, size_t n)
{
	return char_list_init_reserved(string, n);
}

int m_string_init_reserved_with_allocator(m_string *string, size_t n, const m_allocator *alloc)
{
	return char_list_init_reserved_with_allocator(string, n, alloc);
}

int m_string_reserve(m_string *string, size_t n)
{
	return char_list_reserve(string, n);
}

int m_string_append(m_string *string, char x)
{
	return char_list_append(string, x);
}

int m_string_append_int(m_string *string, int x)
{
	if (!string)
		return ERR_NULL_PTR;
	
	char buf[128];
	
	snprintf(buf, 128, "%d", x);
	
	return m_string_append_str(string, buf);
}

int m_string_append_str(m_string *string, const char *str)
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

int m_string_concat(m_string *a, m_string *b)
{
	return char_list_append_list(a, b);
}

int m_string_destroy(m_string *string)
{
	return char_list_destroy(string);
}

int m_string_contains(m_string *string, char c)
{
	return char_list_contains(string, c, char_cmp);
}

int m_string_index_of(m_string *string, char x)
{
	return char_list_index_of(string, x, char_cmp);
}

char *m_string_head(m_string *string)
{
	return char_list_head(string);
}

char *m_string_tail(m_string *string)
{
	return char_list_tail(string);
}

int m_string_pop_tail(m_string *string)
{
	return char_list_pop_tail(string);
}

int m_string_drain(m_string *string)
{
	return char_list_drain(string);
}

int m_string_len(m_string *string)
{
	return string->count;
}

int m_string_write_out(m_string *string, char *buf, size_t buf_len)
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

char *m_string_to_native(m_string *string)
{
	if (!string)
		return NULL;
	
	size_t len = string->count;
	
	char *result = m_alloc(len + 1);
	
	if (!result)
		return NULL;
	
	if (string->entries)
		memcpy(result, string->entries, len);
	else
		result[0] = 0;
	
	result[len] = 0;
	
	return result;
}

char *m_string_to_native_with_allocator(m_string *string, m_allocator *alloc)
{
	if (!string)
		return NULL;
	
	size_t len = string->count;
	
	char *result = m_allocator_alloc(alloc, len + 1);
	
	if (!result)
		return NULL;
	
	if (string->entries)
		memcpy(result, string->entries, len);
	else
		result[0] = 0;
	
	result[len] = 0;
	
	return result;
}

int m_string_appendf(m_string *string, const char *fmt, ...)
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
	
	m_string_append_str(string, buf);
	
	return NO_ERROR;
}
