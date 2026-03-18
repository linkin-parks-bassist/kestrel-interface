#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 1
#endif

static const char *FNAME = "m_dictionary.c";

IMPLEMENT_LIST(m_dictionary_entry);

const char *m_dict_entry_type_to_string(int type)
{
	switch (type)
	{
		case DICT_ENTRY_TYPE_INT: 		return "DICT_ENTRY_TYPE_INT";
		case DICT_ENTRY_TYPE_FLOAT: 	return "DICT_ENTRY_TYPE_FLOAT";
		case DICT_ENTRY_TYPE_STR: 		return "DICT_ENTRY_TYPE_STR";
		case DICT_ENTRY_TYPE_EXPR: 		return "DICT_ENTRY_TYPE_EXPR";
		case DICT_ENTRY_TYPE_SUBDICT: 	return "DICT_ENTRY_TYPE_SUBDICT";
		case DICT_ENTRY_TYPE_LIST: 		return "DICT_ENTRY_TYPE_LIST";
		default: return "DICT_ENTRY_TYPE_UNKNOWN";
	}
}

const char *m_dict_entry_type_to_string_nice(int type)
{
	switch (type)
	{
		case DICT_ENTRY_TYPE_INT: 		return "int";
		case DICT_ENTRY_TYPE_FLOAT: 	return "float";
		case DICT_ENTRY_TYPE_STR: 		return "string";
		case DICT_ENTRY_TYPE_EXPR: 		return "expression";
		case DICT_ENTRY_TYPE_SUBDICT: 	return "dictionary";
		case DICT_ENTRY_TYPE_LIST: 		return "list";
		default: return "unknown";
	}
}

m_string *m_dict_entry_to_string(m_dictionary_entry *entry)
{
	if (!entry)
		return NULL;
	
	m_string *result = m_allocator_alloc(NULL, sizeof(m_string));
	m_string *str;
	m_string_init(result);
	
	m_string_appendf(result, "%s: ", entry->name ? entry->name : "(NULL)");
	
	m_dictionary *dict = entry->value.val_dict;
	m_dictionary_entry_list *list = entry->value.val_list;
	
	int any = 0;
	
	switch (entry->type)
	{
		case DICT_ENTRY_TYPE_INT:
			m_string_appendf(result, "(int) %d", entry->value.val_int);
			break;
		case DICT_ENTRY_TYPE_FLOAT:
			m_string_appendf(result, "(float) %.04f", entry->value.val_float);
			break;
		case DICT_ENTRY_TYPE_STR:
			m_string_appendf(result, "(string) \"%s\"", entry->value.val_string);
			break;
		case DICT_ENTRY_TYPE_EXPR:
			m_string_appendf(result, "(expr) %s", m_expression_to_string(entry->value.val_expr));
			break;
		case DICT_ENTRY_TYPE_SUBDICT:
			m_string_append(result, '(');
			if (!dict)
			{
				m_string_appendf(result, "(NULL))");
			}
			else
			{
				for (int i = 0; i < M_DICTIONARY_N_BUCKETS; i++)
				{
					for (int j = 0; j < dict->buckets[i].n_entries; j++)
					{
						if (any)
						{							
							m_string_append(result, ',');
							m_string_append(result, ' ');
						}
						str = m_dict_entry_to_string(&dict->buckets[i].entries[j]);
						m_string_concat(result, str);
						m_string_destroy(str);
						
						any = 1;
					}
				}
				
				m_string_append(result, ')');
			}
			break;
		case DICT_ENTRY_TYPE_LIST:
			m_string_append(result, '{');
			if (!list)
			{
				m_string_appendf(result, "(NULL)}");
			}
			else
			{
				for (int i = 0; i < list->count; i++)
				{
					if (i != 0)
					{
						m_string_append(result, ',');
						m_string_append(result, ' ');
					}
					str = m_dict_entry_to_string(&list->entries[i]);
					m_string_concat(result, str);
					m_string_destroy(str);
				}
				
				m_string_append(result, '}');
			}
			break;
	}
	
	return result;
}

int m_dictionary_bucket_init(m_dictionary_bucket *bucket)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	memset(bucket, 0, sizeof(m_dictionary_bucket));
	
	return NO_ERROR;
}

int m_dictionary_bucket_ensure_capacity(m_dictionary_bucket *bucket)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	m_dictionary_entry *na;
	
	if (bucket->entry_array_length == 0)
	{
		bucket->entries = m_parser_alloc(sizeof(m_dictionary_entry) * 2);
	
		if (!bucket->entries)
		{
			bucket->entry_array_length = 0;
			return ERR_ALLOC_FAIL;
		}
		
		bucket->entry_array_length = 2;
	}
	else if (bucket->n_entries == bucket->entry_array_length)
	{
		na = m_parser_alloc(sizeof(m_dictionary_entry) * bucket->entry_array_length * 2);
		
		if (!na) return ERR_ALLOC_FAIL;
		
		for (int i = 0; i < bucket->n_entries; i++)
			na[i] = bucket->entries[i];
		
		bucket->entries = na;
		bucket->entry_array_length *= 2;
	}
	
	return NO_ERROR;
}

int m_dictionary_bucket_add_entry(m_dictionary_bucket *bucket, m_dictionary_entry entry)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name = strndup(entry.name, 32);
	bucket->entries[bucket->n_entries].type = entry.type;
	bucket->entries[bucket->n_entries].value = entry.value;

	bucket->n_entries++;
	
	return ret_val;
}

int m_dictionary_bucket_add_entry_str(m_dictionary_bucket *bucket, const char *name, const char *value)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name  = name;
	bucket->entries[bucket->n_entries].type = DICT_ENTRY_TYPE_STR;
	bucket->entries[bucket->n_entries].value.val_string = value;

	bucket->n_entries++;
	
	return NO_ERROR;
}

int m_dictionary_bucket_add_entry_int(m_dictionary_bucket *bucket, const char *name, int value)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name  = name;
	bucket->entries[bucket->n_entries].type = DICT_ENTRY_TYPE_INT;
	bucket->entries[bucket->n_entries].value.val_int = value;

	bucket->n_entries++;
	
	return NO_ERROR;
}

int m_dictionary_bucket_add_entry_float(m_dictionary_bucket *bucket, const char *name, float value)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name  = name;
	bucket->entries[bucket->n_entries].type = DICT_ENTRY_TYPE_FLOAT;
	bucket->entries[bucket->n_entries].value.val_float = value;

	bucket->n_entries++;
	
	return NO_ERROR;
}

int m_dictionary_bucket_add_entry_expr(m_dictionary_bucket *bucket, const char *name, m_expression *value)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name  = name;
	bucket->entries[bucket->n_entries].type = DICT_ENTRY_TYPE_EXPR;
	bucket->entries[bucket->n_entries].value.val_expr = value;

	bucket->n_entries++;
	
	return NO_ERROR;
}

int m_dictionary_bucket_add_entry_dict(m_dictionary_bucket *bucket, const char *name, m_dictionary *value)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name  = name;
	bucket->entries[bucket->n_entries].type = DICT_ENTRY_TYPE_SUBDICT;
	bucket->entries[bucket->n_entries].value.val_dict = value;

	bucket->n_entries++;
	
	return NO_ERROR;
}

int m_dictionary_bucket_lookup(m_dictionary_bucket *bucket, const char *name, void *result, int type)
{
	if (!bucket || !result || !name)
		return ERR_NULL_PTR;
	
	for (int i = 0; i < bucket->n_entries; i++)
	{
		if (strcmp(bucket->entries[i].name, name) == 0)
		{
			if (bucket->entries[i].type == type)
			{
				switch (type)
				{
					case DICT_ENTRY_TYPE_STR:
						*((const char**)result) = bucket->entries[i].value.val_string;
						break;
					case DICT_ENTRY_TYPE_FLOAT:
						*((float*)result) = bucket->entries[i].value.val_float;
						break;
					case DICT_ENTRY_TYPE_INT:
						*((int*)result) = bucket->entries[i].value.val_int;
						break;
					case DICT_ENTRY_TYPE_EXPR:
						*((m_expression**)result) = bucket->entries[i].value.val_expr;
						break;
					case DICT_ENTRY_TYPE_SUBDICT:
						*((m_dictionary**)result) = bucket->entries[i].value.val_dict;
						break;
					case DICT_ENTRY_TYPE_LIST:
						*((m_dictionary_entry_list**)result) = bucket->entries[i].value.val_list;
						break;
				}
			}
			else if (type == DICT_ENTRY_TYPE_FLOAT && bucket->entries[i].type == DICT_ENTRY_TYPE_INT)
			{
				*((float*)result) = (float)bucket->entries[i].value.val_int;
			}
			else if (type == DICT_ENTRY_TYPE_INT && bucket->entries[i].type == DICT_ENTRY_TYPE_FLOAT)
			{
				*((int*)result) = (int)roundf(bucket->entries[i].value.val_float);
			}
			else
			{
				return ERR_WRONG_TYPE;
			}
			
			return NO_ERROR;
		}
	}
	
	return ERR_NOT_FOUND;
}

int m_dictionary_bucket_lookup_str(m_dictionary_bucket *bucket, const char *name, const char **result)
{
	return m_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_STR);
}

int m_dictionary_bucket_lookup_float(m_dictionary_bucket *bucket, const char *name, float *result)
{
	return m_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_FLOAT);
}

int m_dictionary_bucket_lookup_int(m_dictionary_bucket *bucket, const char *name, int *result)
{
	return m_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_INT);
}

int m_dictionary_bucket_lookup_expr(m_dictionary_bucket *bucket, const char *name, m_expression **result)
{
	return m_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_EXPR);
}

int m_dictionary_bucket_lookup_dict(m_dictionary_bucket *bucket, const char *name, m_dictionary **result)
{
	return m_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_SUBDICT);
}

int m_dictionary_bucket_lookup_list(m_dictionary_bucket *bucket, const char *name, m_dictionary_entry_list **result)
{
	return m_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_LIST);
}


m_dictionary *m_new_dictionary()
{
	m_dictionary *dict = m_parser_alloc(sizeof(m_dictionary));
	
	if (!dict)
		return NULL;
	
	dict->name = NULL;
	dict->n_entries = 0;
	dict->entries = m_parser_alloc(sizeof(m_dictionary_entry) * 8);
	
	if (!dict->entries)
	{
		dict->entry_array_length = 0;
		return NULL;
	}
	
	dict->entry_array_length = 8;
	
	for (int i = 0; i < M_DICTIONARY_N_BUCKETS; i++)
		m_dictionary_bucket_init(&dict->buckets[i]);
	
	return dict;
}

int m_dictionary_ensure_capacity(m_dictionary *dict)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	m_dictionary_entry *na;
	
	if (dict->n_entries == dict->entry_array_length)
	{
		na = m_parser_alloc(sizeof(m_dictionary_entry) * dict->entry_array_length * 2);
		
		if (!na) return ERR_ALLOC_FAIL;
		
		for (int i = 0; i < dict->n_entries; i++)
			na[i] = dict->entries[i];
		
		dict->entries = na;
		dict->entry_array_length *= 2;
	}
	
	return NO_ERROR;
}

int m_dictionary_add_entry(m_dictionary *dict, m_dictionary_entry entry)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name = strndup(entry.name, 32);
	dict->entries[dict->n_entries].type = entry.type;
	dict->entries[dict->n_entries].value = entry.value;

	dict->n_entries++;
	
	uint32_t bucket = hash(entry.name) & (M_DICTIONARY_N_BUCKETS - 1);
	
	return m_dictionary_bucket_add_entry(&dict->buckets[bucket], entry);
}

int m_dictionary_add_entry_str(m_dictionary *dict, const char *name, const char *value)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name  = name;
	dict->entries[dict->n_entries].type = DICT_ENTRY_TYPE_STR;
	dict->entries[dict->n_entries].value.val_string = value;

	dict->n_entries++;
	
	uint32_t bucket = hash(name) & (M_DICTIONARY_N_BUCKETS - 1);
	
	return m_dictionary_bucket_add_entry_str(&dict->buckets[bucket], name, value);
}

int m_dictionary_add_entry_int(m_dictionary *dict, const char *name, int value)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name  = name;
	dict->entries[dict->n_entries].type = DICT_ENTRY_TYPE_INT;
	dict->entries[dict->n_entries].value.val_int = value;

	dict->n_entries++;
	
	uint32_t bucket = hash(name) & (M_DICTIONARY_N_BUCKETS - 1);
	
	return m_dictionary_bucket_add_entry_int(&dict->buckets[bucket], name, value);
}

int m_dictionary_add_entry_float(m_dictionary *dict, const char *name, float value)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name  = name;
	dict->entries[dict->n_entries].type = DICT_ENTRY_TYPE_FLOAT;
	dict->entries[dict->n_entries].value.val_float = value;

	dict->n_entries++;
	
	uint32_t bucket = hash(name) & (M_DICTIONARY_N_BUCKETS - 1);
	
	return m_dictionary_bucket_add_entry_float(&dict->buckets[bucket], name, value);
}

int m_dictionary_add_entry_expr(m_dictionary *dict, const char *name, m_expression *value)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name  = name;
	dict->entries[dict->n_entries].type = DICT_ENTRY_TYPE_EXPR;
	dict->entries[dict->n_entries].value.val_expr = value;

	dict->n_entries++;
	
	uint32_t bucket = hash(name) & (M_DICTIONARY_N_BUCKETS - 1);
	
	return m_dictionary_bucket_add_entry_expr(&dict->buckets[bucket], name, value);
}

int m_dictionary_add_entry_dict(m_dictionary *dict, const char *name, m_dictionary *value)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = m_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name  = name;
	dict->entries[dict->n_entries].type = DICT_ENTRY_TYPE_SUBDICT;
	dict->entries[dict->n_entries].value.val_dict = value;

	dict->n_entries++;
	
	uint32_t bucket = hash(name) & (M_DICTIONARY_N_BUCKETS - 1);
	
	return m_dictionary_bucket_add_entry_dict(&dict->buckets[bucket], name, value);
}

int m_dictionary_lookup(m_dictionary *dict, const char *name, void *result, int type)
{
	if (!dict || !result || !name)
		return ERR_NULL_PTR;
	
	uint32_t bucket = hash(name) & (M_DICTIONARY_N_BUCKETS - 1);
	
	return m_dictionary_bucket_lookup(&dict->buckets[bucket], name, result, type);
}

int m_dictionary_lookup_str(m_dictionary *dict, const char *name, const char **result)
{
	return m_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_STR);
}

int m_dictionary_lookup_float(m_dictionary *dict, const char *name, float *result)
{
	return m_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_FLOAT);
}

int m_dictionary_lookup_int(m_dictionary *dict, const char *name, int *result)
{
	return m_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_INT);
}

int m_dictionary_lookup_expr(m_dictionary *dict, const char *name, m_expression **result)
{
	return m_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_EXPR);
}

int m_dictionary_lookup_dict(m_dictionary *dict, const char *name, m_dictionary **result)
{
	return m_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_SUBDICT);
}

int m_dictionary_lookup_list(m_dictionary *dict, const char *name, m_dictionary_entry_list **result)
{
	return m_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_LIST);
}

void print_dict_entry(m_dictionary_entry *entry)
{
	if (!entry)
	{
		M_PRINTF("(null)");
		return;
	}
	M_PRINTF("%s: ", entry->name);
	
	switch (entry->type)
	{
		case DICT_ENTRY_TYPE_STR:
			M_PRINTF("\"%s\"", entry->value.val_string);
			break;
		
		case DICT_ENTRY_TYPE_INT:
			M_PRINTF("%d", entry->value.val_int);
			break;
		
		case DICT_ENTRY_TYPE_FLOAT:
			M_PRINTF("%f", entry->value.val_float);
			break;
		
		case DICT_ENTRY_TYPE_EXPR:
			M_PRINTF("(expression)");
			break;
			
		default:
			M_PRINTF("mangled !");
			break;
	}
}

void print_dict(m_dictionary *dict)
{
	M_PRINTF("Dictionary ");
	if (!dict)
	{
		M_PRINTF("(null)\n");
		return;
	}
	
	m_string full_string;
	m_string_init(&full_string);
	
	m_string_appendf(&full_string, "\"%s\" (%d entries):\n", dict->name, dict->n_entries);
	
	m_string *string;
	char *str = NULL;
	
	for (int i = 0; i < dict->n_entries; i++)
	{
		m_string_append(&full_string, '\t');
		string = m_dict_entry_to_string(&dict->entries[i]);
		m_string_concat(&full_string, string);
		m_string_destroy(string);
		m_string_append(&full_string, '\n');
	}
	
	str = m_string_to_native(&full_string);
	M_PRINTF("%s", str);
	m_free(str);
	m_string_destroy(&full_string);
}

#define LIST_ENTRY_NAME_BUF_LEN 128

int m_parse_dict_list(m_eff_parsing_state *ps, m_dictionary *dict, m_dictionary_entry *result)
{
	M_PRINTF("m_parse_dict_list\n");
	
	if (!ps || !dict || !result)
		return ERR_NULL_PTR;
	
	char *str;
	
	m_dictionary_entry item;
	
	m_token_ll *current = ps->current_token;
	
	if (!current)
		return ERR_BAD_ARGS;
	
	if (!result->name)
		return ERR_BAD_ARGS;
	int name_len = strlen(result->name);
	
	if (name_len > LIST_ENTRY_NAME_BUF_LEN - 4)
		return ERR_BAD_ARGS;

	result->line = current->line;

	int base_len;
	
	m_dictionary_entry centry;
	
	int ret_val = NO_ERROR;
	
	result->value.val_list = m_parser_alloc(sizeof(m_dictionary_entry_list));
	
	if (!result->value.val_list)
		return ERR_ALLOC_FAIL;
	
	m_dictionary_entry_list_init(result->value.val_list);
	
	m_string name;
	m_string *s;
	
	if ((ret_val = m_string_init_with_allocator(&name, m_parser_allocator)) != NO_ERROR)
		goto parse_dict_list_fin;
	
	m_string_append_str(&name, dict->name);
	m_string_append(&name, '.');
	m_string_append_str(&name, result->name);
	
	m_string_append(&name, '[');
	
	base_len = m_string_len(&name);
	
	m_token_ll_skip_ws(&current);
	
	int i = 0;
	
	while (current)
	{
		while (m_string_len(&name) > base_len)
			m_string_pop_tail(&name);
		
		m_string_appendf(&name, "%d]", i);
		
		str = m_string_to_native_with_allocator(&name, m_parser_allocator);
		
		M_PRINTF("Current entry: %s, current token: \"%s\"\n", str, current->data);
		
		centry.name = str;
		
		ps->current_token = current;
		
		if ((ret_val = m_parse_dict_val(ps, dict, &centry)) != NO_ERROR)
			goto parse_dict_list_fin;

		m_dictionary_add_entry(dict, centry);
		m_dictionary_entry_list_append(result->value.val_list, centry);
		
		current = ps->current_token;
		
		s = m_dict_entry_to_string(&centry);
		str = m_string_to_native(s);
		M_PRINTF("Adding entry to list; %s. Current token is \"%s\"\n", str, current ? (current->data[0] == '\n' ? "\\n" : current->data) : "(NULL)");
		m_free(str);
		m_string_destroy(s);
		
		m_token_ll_skip_ws(&current);
		if (!current || strcmp(current->data, "}") == 0)
			break;
		
		if (strcmp(current->data, ",") == 0)
			m_token_ll_advance(&current);
		
		i++;
	}
	
	M_PRINTF("m_parse_dict_list done\n");

parse_dict_list_fin:

	m_token_ll_skip_ws(&current);
	
	ps->current_token = current;
	
	s = m_dict_entry_to_string(result);
	str = m_string_to_native(s);
	
	M_PRINTF("m_parse_dict_list: result: %s\n", str);
	m_free(str);
	m_string_destroy(s);
	
	
	M_PRINTF("m_parse_dict_list done\n");
	return ret_val;
}

int m_parse_dict_val(m_eff_parsing_state *ps, m_dictionary *dict, m_dictionary_entry *result)
{
	M_PRINTF("m_parse_dict_val\n");
	if (!ps || !result)
		return ERR_NULL_PTR;
	
	m_token_ll *current = ps->current_token;
	
	if (!current)
		return ERR_BAD_ARGS;
	
	if (!current->data)
		return ERR_BAD_ARGS;
	
	m_string *string;
	char *str;
	
	m_token_ll *end = current;
	
	int ret_val = NO_ERROR;
	int len;
	
	int paren_cnt = 0;
	int n_tokens = 0;
	
	int contains_semicolon = 0;
	
	result->line = current->line;
	
	while (end)
	{
		if (strcmp(end->data, ":") == 0)
		{
			contains_semicolon = 1;
		}
		else if (strcmp(end->data, "(") == 0)
		{
			paren_cnt++;
		}
		else if (strcmp(end->data, ")") == 0)
		{
			if (paren_cnt > 0)
				paren_cnt--;
			else
				break;
		}
		
		if (paren_cnt == 0 && token_is_dict_entry_seperator(end->data))
			break;
		
		end = end->next;
		n_tokens++;
	}
	
	if (current->data[0] == '"')
	{
		result->type = DICT_ENTRY_TYPE_STR;
		
		len = strlen(current->data) - 2;
		if (n_tokens > 1)
		{
			M_PRINTF("Syntax error (line %d): excess tokens following string %s\n", current->line, current->data);
			ret_val = ERR_BAD_ARGS;
			goto parse_dict_val_fin;
		}
		
		result->value.val_string = m_parser_strndup(&current->data[1], len);
		
		m_token_ll_advance(&current);
		
		goto parse_dict_val_fin;
	}
	else if (strcmp(current->data, "(") == 0 && contains_semicolon)
	{
		result->type = DICT_ENTRY_TYPE_SUBDICT;
		ps->current_token = current->next;
		ret_val = m_parse_dictionary(ps, &result->value.val_dict, result->name);
		
		current = ps->current_token;
		
		if (current && strcmp(current->data, ")") != 0)
		{
			m_parser_error_at(ps, current, "Expected \")\", got \"%s\"", (current->data[0] == '\n') ? "\\n" : current->data);
		}
		
		m_token_ll_advance(&current);
		
		goto parse_dict_val_fin;
	}
	else if (strcmp(current->data, "{") == 0)
	{
		result->type = DICT_ENTRY_TYPE_LIST;
		ps->current_token = current->next;
		ret_val = m_parse_dict_list(ps, dict, result);
		
		current = ps->current_token;
		
		if (current && strcmp(current->data, "}") != 0)
		{
			m_parser_error_at(ps, current, "Expected \"}\", got \"%s\"", (current->data[0] == '\n') ? "\\n" : current->data);
		}
		
		m_token_ll_advance(&current);
		
		goto parse_dict_val_fin;
	}
	else
	{
		result->type = DICT_ENTRY_TYPE_EXPR;
		result->value.val_expr = m_parse_expression(ps, current, end);
		
		if (!result->value.val_expr)
		{
			result->type = DICT_ENTRY_TYPE_NOTHING;
			ret_val = ERR_BAD_ARGS;
		}
		
		current = ps->current_token;
		
		goto parse_dict_val_fin;
	}
	
parse_dict_val_fin:
	
	ps->current_token = current;
	
	if (result && ret_val == NO_ERROR)
	{
		string = m_dict_entry_to_string(result);
		str = m_string_to_native(string);
		M_PRINTF("Obtained entry %s\n", str);
		m_free(str);
		m_string_destroy(string);
	}
	
	M_PRINTF("m_parse_dict_val done\n");
	return ret_val;
}

int m_parse_dictionary(m_eff_parsing_state *ps, m_dictionary **result, const char *name)
{
	M_PRINTF("m_parse_dictionary\n");
	if (!ps || !result)
		return ERR_NULL_PTR;
	
	m_token_ll *current = ps->current_token;
	m_token_ll *nt = NULL;
	
	if (!current)
		return ERR_BAD_ARGS;
	
	m_dictionary_entry centry;
	
	int ret_val = NO_ERROR;

	m_dictionary *dict = NULL;
	
	m_string *string = NULL;
	char *str;
	
	dict = m_new_dictionary();
	
	if (!dict)
	{
		ret_val = ERR_ALLOC_FAIL;
		goto parse_dict_fin;
	}
	
	char *cname;
	dict->name = m_parser_strndup(name, 128);
	
	M_PRINTF("Parsing dictionary with name \"%s\"\n", dict->name);
	
	m_token_ll_skip_ws(&current);
	
	while (current)
	{
		M_PRINTF("Looking for entry. Current token = \"%s\"\n", current->data[0] == '\n' ? "\\n" : current->data);
		if (!token_is_name(current->data))
		{
			m_parser_error_at(ps, current, "Expected name, got \"%s\"", current->data[0] == '\n' ? "\\n" : current->data);
			ret_val = ERR_BAD_ARGS;
			goto parse_dict_fin;
		}
		
		cname = m_parser_strndup(current->data, 64);
		
		if (!cname)
		{
			ret_val = ERR_ALLOC_FAIL;
			goto parse_dict_fin;
		}
		
		centry.name = cname;
		
		m_token_ll_advance(&current);
		
		if (!current || (strcmp(current->data, ":") != 0 && strcmp(current->data, "=") != 0))
		{
			m_parser_error_at(ps, current, "Expected \":\" or \"=\", got \"%s\"", current->data);
			ret_val = ERR_BAD_ARGS;
			goto parse_dict_fin;
		}
		m_token_ll_advance(&current);
		
		ps->current_token = current;
		if ((ret_val = m_parse_dict_val(ps, dict, &centry)) != NO_ERROR)
		{
			m_parser_error(ps, "Error parsing attribute %s.%s", dict->name, cname);
			goto parse_dict_fin;
		}
		else
		{
			m_dictionary_add_entry(dict, centry);
		}
		
		current = ps->current_token;
		m_token_ll_skip_ws(&current);
		
		if (!current || strcmp(current->data, ")") == 0)
			break;
		
		if (strcmp(current->data, ",") == 0)
			m_token_ll_advance(&current);
	}
	
	*result = dict;

parse_dict_fin:
	
	ps->current_token = current;
	M_PRINTF("Done parsing dictionary; next token: \"%s\"\n", current ? (current->data[0] == '\n' ? "\\n" : current->data) : "(NULL)");
	
	return ret_val;
}
