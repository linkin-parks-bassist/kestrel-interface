#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 1
#endif

static const char *FNAME = "kest_dictionary.c";

IMPLEMENT_LIST(kest_dictionary_entry);

const char *kest_dict_entry_type_to_string(int type)
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

const char *kest_dict_entry_type_to_string_nice(int type)
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

kest_string *kest_dict_entry_to_string(kest_dictionary_entry *entry)
{
	if (!entry)
		return NULL;
	
	kest_string *result = kest_allocator_alloc(NULL, sizeof(kest_string));
	kest_string *str;
	kest_string_init(result);
	
	kest_string_appendf(result, "%s: ", entry->name ? entry->name : "(NULL)");
	
	kest_dictionary *dict = entry->value.val_dict;
	kest_dictionary_entry_list *list = entry->value.val_list;
	
	int any = 0;
	
	switch (entry->type)
	{
		case DICT_ENTRY_TYPE_INT:
			kest_string_appendf(result, "(int) %d", entry->value.val_int);
			break;
		case DICT_ENTRY_TYPE_FLOAT:
			kest_string_appendf(result, "(float) %.04f", entry->value.val_float);
			break;
		case DICT_ENTRY_TYPE_STR:
			kest_string_appendf(result, "(string) \"%s\"", entry->value.val_string);
			break;
		case DICT_ENTRY_TYPE_EXPR:
			kest_string_appendf(result, "(expr) %s", kest_expression_to_string(entry->value.val_expr));
			break;
		case DICT_ENTRY_TYPE_SUBDICT:
			kest_string_append(result, '(');
			if (!dict)
			{
				kest_string_appendf(result, "(NULL))");
			}
			else
			{
				for (int i = 0; i < KEST_DICTIONARY_N_BUCKETS; i++)
				{
					for (int j = 0; j < dict->buckets[i].n_entries; j++)
					{
						if (any)
						{							
							kest_string_append(result, ',');
							kest_string_append(result, ' ');
						}
						str = kest_dict_entry_to_string(&dict->buckets[i].entries[j]);
						kest_string_concat(result, str);
						kest_string_destroy(str);
						
						any = 1;
					}
				}
				
				kest_string_append(result, ')');
			}
			break;
		case DICT_ENTRY_TYPE_LIST:
			kest_string_append(result, '{');
			if (!list)
			{
				kest_string_appendf(result, "(NULL)}");
			}
			else
			{
				for (int i = 0; i < list->count; i++)
				{
					if (i != 0)
					{
						kest_string_append(result, ',');
						kest_string_append(result, ' ');
					}
					str = kest_dict_entry_to_string(&list->entries[i]);
					kest_string_concat(result, str);
					kest_string_destroy(str);
				}
				
				kest_string_append(result, '}');
			}
			break;
	}
	
	return result;
}

int kest_dictionary_bucket_init(kest_dictionary_bucket *bucket)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	memset(bucket, 0, sizeof(kest_dictionary_bucket));
	
	return NO_ERROR;
}

int kest_dictionary_bucket_ensure_capacity(kest_dictionary_bucket *bucket)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	kest_dictionary_entry *na;
	
	if (bucket->entry_array_length == 0)
	{
		bucket->entries = kest_parser_alloc(sizeof(kest_dictionary_entry) * 2);
	
		if (!bucket->entries)
		{
			bucket->entry_array_length = 0;
			return ERR_ALLOC_FAIL;
		}
		
		bucket->entry_array_length = 2;
	}
	else if (bucket->n_entries == bucket->entry_array_length)
	{
		na = kest_parser_alloc(sizeof(kest_dictionary_entry) * bucket->entry_array_length * 2);
		
		if (!na) return ERR_ALLOC_FAIL;
		
		for (int i = 0; i < bucket->n_entries; i++)
			na[i] = bucket->entries[i];
		
		bucket->entries = na;
		bucket->entry_array_length *= 2;
	}
	
	return NO_ERROR;
}

int kest_dictionary_bucket_add_entry(kest_dictionary_bucket *bucket, kest_dictionary_entry entry)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name = strndup(entry.name, 32);
	bucket->entries[bucket->n_entries].type = entry.type;
	bucket->entries[bucket->n_entries].value = entry.value;

	bucket->n_entries++;
	
	return ret_val;
}

int kest_dictionary_bucket_add_entry_str(kest_dictionary_bucket *bucket, const char *name, const char *value)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name  = name;
	bucket->entries[bucket->n_entries].type = DICT_ENTRY_TYPE_STR;
	bucket->entries[bucket->n_entries].value.val_string = value;

	bucket->n_entries++;
	
	return NO_ERROR;
}

int kest_dictionary_bucket_add_entry_int(kest_dictionary_bucket *bucket, const char *name, int value)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name  = name;
	bucket->entries[bucket->n_entries].type = DICT_ENTRY_TYPE_INT;
	bucket->entries[bucket->n_entries].value.val_int = value;

	bucket->n_entries++;
	
	return NO_ERROR;
}

int kest_dictionary_bucket_add_entry_float(kest_dictionary_bucket *bucket, const char *name, float value)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name  = name;
	bucket->entries[bucket->n_entries].type = DICT_ENTRY_TYPE_FLOAT;
	bucket->entries[bucket->n_entries].value.val_float = value;

	bucket->n_entries++;
	
	return NO_ERROR;
}

int kest_dictionary_bucket_add_entry_expr(kest_dictionary_bucket *bucket, const char *name, kest_expression *value)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name  = name;
	bucket->entries[bucket->n_entries].type = DICT_ENTRY_TYPE_EXPR;
	bucket->entries[bucket->n_entries].value.val_expr = value;

	bucket->n_entries++;
	
	return NO_ERROR;
}

int kest_dictionary_bucket_add_entry_dict(kest_dictionary_bucket *bucket, const char *name, kest_dictionary *value)
{
	if (!bucket)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_bucket_ensure_capacity(bucket)) != NO_ERROR)
		return ret_val;
	
	bucket->entries[bucket->n_entries].name  = name;
	bucket->entries[bucket->n_entries].type = DICT_ENTRY_TYPE_SUBDICT;
	bucket->entries[bucket->n_entries].value.val_dict = value;

	bucket->n_entries++;
	
	return NO_ERROR;
}

int kest_dictionary_bucket_lookup(kest_dictionary_bucket *bucket, const char *name, void *result, int type)
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
						*((kest_expression**)result) = bucket->entries[i].value.val_expr;
						break;
					case DICT_ENTRY_TYPE_SUBDICT:
						*((kest_dictionary**)result) = bucket->entries[i].value.val_dict;
						break;
					case DICT_ENTRY_TYPE_LIST:
						*((kest_dictionary_entry_list**)result) = bucket->entries[i].value.val_list;
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

int kest_dictionary_bucket_lookup_str(kest_dictionary_bucket *bucket, const char *name, const char **result)
{
	return kest_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_STR);
}

int kest_dictionary_bucket_lookup_float(kest_dictionary_bucket *bucket, const char *name, float *result)
{
	return kest_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_FLOAT);
}

int kest_dictionary_bucket_lookup_int(kest_dictionary_bucket *bucket, const char *name, int *result)
{
	return kest_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_INT);
}

int kest_dictionary_bucket_lookup_expr(kest_dictionary_bucket *bucket, const char *name, kest_expression **result)
{
	return kest_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_EXPR);
}

int kest_dictionary_bucket_lookup_dict(kest_dictionary_bucket *bucket, const char *name, kest_dictionary **result)
{
	return kest_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_SUBDICT);
}

int kest_dictionary_bucket_lookup_list(kest_dictionary_bucket *bucket, const char *name, kest_dictionary_entry_list **result)
{
	return kest_dictionary_bucket_lookup(bucket, name, result, DICT_ENTRY_TYPE_LIST);
}


kest_dictionary *kest_new_dictionary()
{
	kest_dictionary *dict = kest_parser_alloc(sizeof(kest_dictionary));
	
	if (!dict)
		return NULL;
	
	dict->name = NULL;
	dict->n_entries = 0;
	dict->entries = kest_parser_alloc(sizeof(kest_dictionary_entry) * 8);
	
	if (!dict->entries)
	{
		dict->entry_array_length = 0;
		return NULL;
	}
	
	dict->entry_array_length = 8;
	
	for (int i = 0; i < KEST_DICTIONARY_N_BUCKETS; i++)
		kest_dictionary_bucket_init(&dict->buckets[i]);
	
	return dict;
}

int kest_dictionary_ensure_capacity(kest_dictionary *dict)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	kest_dictionary_entry *na;
	
	if (dict->n_entries == dict->entry_array_length)
	{
		na = kest_parser_alloc(sizeof(kest_dictionary_entry) * dict->entry_array_length * 2);
		
		if (!na) return ERR_ALLOC_FAIL;
		
		for (int i = 0; i < dict->n_entries; i++)
			na[i] = dict->entries[i];
		
		dict->entries = na;
		dict->entry_array_length *= 2;
	}
	
	return NO_ERROR;
}

int kest_dictionary_add_entry(kest_dictionary *dict, kest_dictionary_entry entry)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name = strndup(entry.name, 32);
	dict->entries[dict->n_entries].type = entry.type;
	dict->entries[dict->n_entries].value = entry.value;

	dict->n_entries++;
	
	uint32_t bucket = hash(entry.name) & (KEST_DICTIONARY_N_BUCKETS - 1);
	
	return kest_dictionary_bucket_add_entry(&dict->buckets[bucket], entry);
}

int kest_dictionary_add_entry_str(kest_dictionary *dict, const char *name, const char *value)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name  = name;
	dict->entries[dict->n_entries].type = DICT_ENTRY_TYPE_STR;
	dict->entries[dict->n_entries].value.val_string = value;

	dict->n_entries++;
	
	uint32_t bucket = hash(name) & (KEST_DICTIONARY_N_BUCKETS - 1);
	
	return kest_dictionary_bucket_add_entry_str(&dict->buckets[bucket], name, value);
}

int kest_dictionary_add_entry_int(kest_dictionary *dict, const char *name, int value)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name  = name;
	dict->entries[dict->n_entries].type = DICT_ENTRY_TYPE_INT;
	dict->entries[dict->n_entries].value.val_int = value;

	dict->n_entries++;
	
	uint32_t bucket = hash(name) & (KEST_DICTIONARY_N_BUCKETS - 1);
	
	return kest_dictionary_bucket_add_entry_int(&dict->buckets[bucket], name, value);
}

int kest_dictionary_add_entry_float(kest_dictionary *dict, const char *name, float value)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name  = name;
	dict->entries[dict->n_entries].type = DICT_ENTRY_TYPE_FLOAT;
	dict->entries[dict->n_entries].value.val_float = value;

	dict->n_entries++;
	
	uint32_t bucket = hash(name) & (KEST_DICTIONARY_N_BUCKETS - 1);
	
	return kest_dictionary_bucket_add_entry_float(&dict->buckets[bucket], name, value);
}

int kest_dictionary_add_entry_expr(kest_dictionary *dict, const char *name, kest_expression *value)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name  = name;
	dict->entries[dict->n_entries].type = DICT_ENTRY_TYPE_EXPR;
	dict->entries[dict->n_entries].value.val_expr = value;

	dict->n_entries++;
	
	uint32_t bucket = hash(name) & (KEST_DICTIONARY_N_BUCKETS - 1);
	
	return kest_dictionary_bucket_add_entry_expr(&dict->buckets[bucket], name, value);
}

int kest_dictionary_add_entry_dict(kest_dictionary *dict, const char *name, kest_dictionary *value)
{
	if (!dict)
		return ERR_NULL_PTR;
	
	if (!name || !value)
		return ERR_BAD_ARGS;
	
	int ret_val = NO_ERROR;
	
	if ((ret_val = kest_dictionary_ensure_capacity(dict)) != NO_ERROR)
		return ret_val;
	
	dict->entries[dict->n_entries].name  = name;
	dict->entries[dict->n_entries].type = DICT_ENTRY_TYPE_SUBDICT;
	dict->entries[dict->n_entries].value.val_dict = value;

	dict->n_entries++;
	
	uint32_t bucket = hash(name) & (KEST_DICTIONARY_N_BUCKETS - 1);
	
	return kest_dictionary_bucket_add_entry_dict(&dict->buckets[bucket], name, value);
}

int kest_dictionary_lookup(kest_dictionary *dict, const char *name, void *result, int type)
{
	if (!dict || !result || !name)
		return ERR_NULL_PTR;
	
	uint32_t bucket = hash(name) & (KEST_DICTIONARY_N_BUCKETS - 1);
	
	return kest_dictionary_bucket_lookup(&dict->buckets[bucket], name, result, type);
}

int kest_dictionary_lookup_str(kest_dictionary *dict, const char *name, const char **result)
{
	return kest_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_STR);
}

int kest_dictionary_lookup_float(kest_dictionary *dict, const char *name, float *result)
{
	return kest_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_FLOAT);
}

int kest_dictionary_lookup_int(kest_dictionary *dict, const char *name, int *result)
{
	return kest_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_INT);
}

int kest_dictionary_lookup_expr(kest_dictionary *dict, const char *name, kest_expression **result)
{
	return kest_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_EXPR);
}

int kest_dictionary_lookup_dict(kest_dictionary *dict, const char *name, kest_dictionary **result)
{
	return kest_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_SUBDICT);
}

int kest_dictionary_lookup_list(kest_dictionary *dict, const char *name, kest_dictionary_entry_list **result)
{
	return kest_dictionary_lookup(dict, name, result, DICT_ENTRY_TYPE_LIST);
}

void print_dict_entry(kest_dictionary_entry *entry)
{
	if (!entry)
	{
		KEST_PRINTF("(null)");
		return;
	}
	KEST_PRINTF("%s: ", entry->name);
	
	switch (entry->type)
	{
		case DICT_ENTRY_TYPE_STR:
			KEST_PRINTF("\"%s\"", entry->value.val_string);
			break;
		
		case DICT_ENTRY_TYPE_INT:
			KEST_PRINTF("%d", entry->value.val_int);
			break;
		
		case DICT_ENTRY_TYPE_FLOAT:
			KEST_PRINTF("%f", entry->value.val_float);
			break;
		
		case DICT_ENTRY_TYPE_EXPR:
			KEST_PRINTF("(expression)");
			break;
			
		default:
			KEST_PRINTF("mangled !");
			break;
	}
}

void print_dict(kest_dictionary *dict)
{
	KEST_PRINTF("Dictionary ");
	if (!dict)
	{
		KEST_PRINTF("(null)\n");
		return;
	}
	
	kest_string full_string;
	kest_string_init(&full_string);
	
	kest_string_appendf(&full_string, "\"%s\" (%d entries):\n", dict->name, dict->n_entries);
	
	kest_string *string;
	char *str = NULL;
	
	for (int i = 0; i < dict->n_entries; i++)
	{
		kest_string_append(&full_string, '\t');
		string = kest_dict_entry_to_string(&dict->entries[i]);
		kest_string_concat(&full_string, string);
		kest_string_destroy(string);
		kest_string_append(&full_string, '\n');
	}
	
	str = kest_string_to_native(&full_string);
	KEST_PRINTF("%s", str);
	kest_free(str);
	kest_string_destroy(&full_string);
}

#define LIST_ENTRY_NAME_BUF_LEN 128

int kest_parse_dict_list(kest_eff_parsing_state *ps, kest_dictionary *dict, kest_dictionary_entry *result)
{
	KEST_PRINTF("kest_parse_dict_list\n");
	
	if (!ps || !dict || !result)
		return ERR_NULL_PTR;
	
	char *str;
	
	kest_dictionary_entry item;
	
	kest_token_ll *current = ps->current_token;
	
	if (!current)
		return ERR_BAD_ARGS;
	
	if (!result->name)
		return ERR_BAD_ARGS;
	int name_len = strlen(result->name);
	
	if (name_len > LIST_ENTRY_NAME_BUF_LEN - 4)
		return ERR_BAD_ARGS;

	result->line = current->line;

	int base_len;
	
	kest_dictionary_entry centry;
	
	int ret_val = NO_ERROR;
	
	result->value.val_list = kest_parser_alloc(sizeof(kest_dictionary_entry_list));
	
	if (!result->value.val_list)
		return ERR_ALLOC_FAIL;
	
	kest_dictionary_entry_list_init(result->value.val_list);
	
	kest_string name;
	kest_string *s;
	
	if ((ret_val = kest_string_init_with_allocator(&name, kest_parser_allocator)) != NO_ERROR)
		goto parse_dict_list_fin;
	
	kest_string_append_str(&name, dict->name);
	kest_string_append(&name, '.');
	kest_string_append_str(&name, result->name);
	
	kest_string_append(&name, '[');
	
	base_len = kest_string_len(&name);
	
	kest_token_ll_skip_ws(&current);
	
	int i = 0;
	
	while (current)
	{
		while (kest_string_len(&name) > base_len)
			kest_string_pop_tail(&name);
		
		kest_string_appendf(&name, "%d]", i);
		
		str = kest_string_to_native_with_allocator(&name, kest_parser_allocator);
		
		KEST_PRINTF("Current entry: %s, current token: \"%s\"\n", str, current->data);
		
		centry.name = str;
		
		ps->current_token = current;
		
		if ((ret_val = kest_parse_dict_val(ps, dict, &centry)) != NO_ERROR)
			goto parse_dict_list_fin;

		kest_dictionary_add_entry(dict, centry);
		kest_dictionary_entry_list_append(result->value.val_list, centry);
		
		current = ps->current_token;
		
		s = kest_dict_entry_to_string(&centry);
		str = kest_string_to_native(s);
		KEST_PRINTF("Adding entry to list; %s. Current token is \"%s\"\n", str, current ? (current->data[0] == '\n' ? "\\n" : current->data) : "(NULL)");
		kest_free(str);
		kest_string_destroy(s);
		
		kest_token_ll_skip_ws(&current);
		if (!current || strcmp(current->data, "}") == 0)
			break;
		
		if (strcmp(current->data, ",") == 0)
			kest_token_ll_advance(&current);
		
		i++;
	}
	
	KEST_PRINTF("kest_parse_dict_list done\n");

parse_dict_list_fin:

	kest_token_ll_skip_ws(&current);
	
	ps->current_token = current;
	
	s = kest_dict_entry_to_string(result);
	str = kest_string_to_native(s);
	
	KEST_PRINTF("kest_parse_dict_list: result: %s\n", str);
	kest_free(str);
	kest_string_destroy(s);
	
	
	KEST_PRINTF("kest_parse_dict_list done\n");
	return ret_val;
}

int kest_parse_dict_val(kest_eff_parsing_state *ps, kest_dictionary *dict, kest_dictionary_entry *result)
{
	KEST_PRINTF("kest_parse_dict_val\n");
	if (!ps || !result)
		return ERR_NULL_PTR;
	
	kest_token_ll *current = ps->current_token;
	
	if (!current)
	{
		return ERR_BAD_ARGS;
	}
	
	if (!current->data)
	{
		return ERR_BAD_ARGS;
	}
	
	kest_string *string;
	char *str;
	
	kest_token_ll *end = current;
	
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
			KEST_PRINTF("Syntax error (line %d): excess tokens following string %s\n", current->line, current->data);
			ret_val = ERR_BAD_ARGS;
			goto parse_dict_val_fin;
		}
		
		result->value.val_string = kest_parser_strndup(&current->data[1], len);
		
		kest_token_ll_advance(&current);
		
		goto parse_dict_val_fin;
	}
	else if (strcmp(current->data, "(") == 0 && contains_semicolon)
	{
		result->type = DICT_ENTRY_TYPE_SUBDICT;
		ps->current_token = current->next;
		ret_val = kest_parse_dictionary(ps, &result->value.val_dict, result->name);
		
		current = ps->current_token;
		
		if (current && strcmp(current->data, ")") != 0)
		{
			kest_parser_error_at(ps, current, "Expected \")\", got \"%s\"", (current->data[0] == '\n') ? "\\n" : current->data);
		}
		
		kest_token_ll_advance(&current);
		
		goto parse_dict_val_fin;
	}
	else if (strcmp(current->data, "{") == 0)
	{
		result->type = DICT_ENTRY_TYPE_LIST;
		ps->current_token = current->next;
		ret_val = kest_parse_dict_list(ps, dict, result);
		
		current = ps->current_token;
		
		if (current && strcmp(current->data, "}") != 0)
		{
			kest_parser_error_at(ps, current, "Expected \"}\", got \"%s\"", (current->data[0] == '\n') ? "\\n" : current->data);
		}
		
		kest_token_ll_advance(&current);
		
		goto parse_dict_val_fin;
	}
	else
	{
		result->type = DICT_ENTRY_TYPE_EXPR;
		result->value.val_expr = kest_parse_expression(ps, current, end);
		
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
		string = kest_dict_entry_to_string(result);
		str = kest_string_to_native(string);
		KEST_PRINTF("Obtained entry %s\n", str);
		kest_free(str);
		kest_string_destroy(string);
	}
	
	KEST_PRINTF("kest_parse_dict_val done\n");
	return ret_val;
}

int kest_parse_dictionary(kest_eff_parsing_state *ps, kest_dictionary **result, const char *name)
{
	KEST_PRINTF("kest_parse_dictionary\n");
	if (!ps || !result)
		return ERR_NULL_PTR;
	
	kest_token_ll *current = ps->current_token;
	kest_token_ll *nt = NULL;
	
	if (!current)
		return ERR_BAD_ARGS;
	
	kest_dictionary_entry centry;
	
	int ret_val = NO_ERROR;

	kest_dictionary *dict = NULL;
	
	kest_string *string = NULL;
	char *str;
	
	dict = kest_new_dictionary();
	
	if (!dict)
	{
		ret_val = ERR_ALLOC_FAIL;
		goto parse_dict_fin;
	}
	
	char *cname;
	dict->name = kest_parser_strndup(name, 128);
	
	KEST_PRINTF("Parsing dictionary with name \"%s\"\n", dict->name);
	
	kest_token_ll_skip_ws(&current);
	
	while (current)
	{
		KEST_PRINTF("Looking for entry. Current token = \"%s\"\n", current->data[0] == '\n' ? "\\n" : current->data);
		if (!token_is_name(current->data))
		{
			kest_parser_error_at(ps, current, "Expected name, got \"%s\"", current->data[0] == '\n' ? "\\n" : current->data);
			ret_val = ERR_BAD_ARGS;
			goto parse_dict_fin;
		}
		
		cname = kest_parser_strndup(current->data, 64);
		
		if (!cname)
		{
			ret_val = ERR_ALLOC_FAIL;
			goto parse_dict_fin;
		}
		
		centry.name = cname;
		
		kest_token_ll_advance(&current);
		
		if (!current || (strcmp(current->data, ":") != 0 && strcmp(current->data, "=") != 0))
		{
			kest_parser_error_at(ps, current, "Expected \":\" or \"=\", got \"%s\"", current->data);
			ret_val = ERR_BAD_ARGS;
			goto parse_dict_fin;
		}
		kest_token_ll_advance(&current);
		
		ps->current_token = current;
		if ((ret_val = kest_parse_dict_val(ps, dict, &centry)) != NO_ERROR)
		{
			kest_parser_error_at(ps, ps->current_token, "Error parsing attribute %s.%s: %s", dict->name, cname, kest_error_code_to_string(ret_val));
			goto parse_dict_fin;
		}
		else
		{
			kest_dictionary_add_entry(dict, centry);
		}
		
		current = ps->current_token;
		kest_token_ll_skip_ws(&current);
		
		if (!current || strcmp(current->data, ")") == 0)
			break;
		
		if (strcmp(current->data, ",") == 0)
			kest_token_ll_advance(&current);
	}
	
	*result = dict;

parse_dict_fin:
	
	ps->current_token = current;
	KEST_PRINTF("Done parsing dictionary; next token: \"%s\"\n", current ? (current->data[0] == '\n' ? "\\n" : current->data) : "(NULL)");
	
	return ret_val;
}
