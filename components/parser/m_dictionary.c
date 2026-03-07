#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "m_int.h"

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "m_dictionary.c";

m_dictionary *m_new_dictionary()
{
	m_dictionary *dict = m_parser_alloc(sizeof(m_dictionary));
	
	if (!dict)
		return NULL;
	
	dict->name = NULL;
	dict->entries = m_parser_alloc(sizeof(m_dictionary_entry) * 8);
	
	if (!dict->entries)
	{
		return NULL;
	}
	
	dict->n_entries = 0;
	dict->entry_array_length = 8;
	
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
	
	return ret_val;
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
	
	return NO_ERROR;
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
	
	return NO_ERROR;
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
	
	return NO_ERROR;
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
	
	return NO_ERROR;
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
	
	return NO_ERROR;
}

int m_dictionary_lookup(m_dictionary *dict, const char *name, void *result, int type)
{
	if (!dict || !result || !name)
		return ERR_NULL_PTR;
	
	for (int i = 0; i < dict->n_entries; i++)
	{
		if (strcmp(dict->entries[i].name, name) == 0)
		{
			if (dict->entries[i].type == type)
			{
				switch (type)
				{
					case DICT_ENTRY_TYPE_STR:
						*((const char**)result) = dict->entries[i].value.val_string;
						break;
					case DICT_ENTRY_TYPE_FLOAT:
						*((float*)result) = dict->entries[i].value.val_float;
						break;
					case DICT_ENTRY_TYPE_INT:
						*((int*)result) = dict->entries[i].value.val_int;
						break;
					case DICT_ENTRY_TYPE_EXPR:
						*((m_expression**)result) = dict->entries[i].value.val_expr;
						break;
					case DICT_ENTRY_TYPE_SUBDICT:
						*((m_dictionary**)result) = dict->entries[i].value.val_dict;
						break;
				}
			}
			else if (type == DICT_ENTRY_TYPE_FLOAT && dict->entries[i].type == DICT_ENTRY_TYPE_INT)
			{
				*((float*)result) = (float)dict->entries[i].value.val_int;
			}
			else if (type == DICT_ENTRY_TYPE_INT && dict->entries[i].type == DICT_ENTRY_TYPE_FLOAT)
			{
				*((int*)result) = (int)roundf(dict->entries[i].value.val_float);
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
	
	M_PRINTF("\"%s\" (%d entries):\n", dict->name, dict->n_entries);
	
	for (int i = 0; i < dict->n_entries; i++)
	{
		M_PRINTF("\t");
		print_dict_entry(&dict->entries[i]);
		M_PRINTF("\n");
	}
}

int m_parse_dict_val(m_eff_parsing_state *ps, m_dictionary_entry *result)
{
	if (!ps || !result)
		return ERR_NULL_PTR;
	
	m_token_ll **next_token = &ps->current_token;
	m_token_ll *current = ps->current_token;
	
	if (!current)
		return ERR_BAD_ARGS;
	
	if (!current->data)
		return ERR_BAD_ARGS;
	
	m_token_ll *end = current;
	
	int ret_val = NO_ERROR;
	int len;
	
	int paren_cnt = 0;
	int n_tokens = 0;
	
	while (end)
	{
		if (strcmp(end->data, "(") == 0)
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
		
		goto parse_dict_val_fin;
	}
	else if (strcmp(current->data, "(") == 0)
	{
		result->type = DICT_ENTRY_TYPE_SUBDICT;
		ps->current_token = current->next;
		ret_val = m_parse_dictionary(ps, &result->value.val_dict, result->name);
		
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
		
		goto parse_dict_val_fin;
	}
	
parse_dict_val_fin:
	
	if (end)
		end = end->next;
	
	if (next_token)
		*next_token = end;
	
	return ret_val;
}

int m_parse_dictionary(m_eff_parsing_state *ps, m_dictionary **result, const char *name)
{
	if (!ps || !result)
		return ERR_NULL_PTR;
	
	m_token_ll **next_token = &ps->current_token;
	m_token_ll *current = ps->current_token;
	m_token_ll *nt = NULL;
	
	if (!current)
		return ERR_BAD_ARGS;
	
	m_dictionary_entry centry;
	
	int ret_val = NO_ERROR;

	m_dictionary *dict = NULL;
	
	dict = m_new_dictionary();
	
	if (!dict)
	{
		ret_val = ERR_ALLOC_FAIL;
		goto parse_dict_fin;
	}
	
	char *cname;
	dict->name = m_parser_strndup(name, 128);
	
	m_token_ll_skip_ws(&current);
	
	while (current)
	{
		if (!token_is_name(current->data))
		{
			m_parser_error_at(ps, current, "Expected name, got \"%s\"", current->data);
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
		if ((ret_val = m_parse_dict_val(ps, &centry)) != NO_ERROR)
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
	}
	
	*result = dict;

parse_dict_fin:
	if (current)
		current = current->next;
	
	if (next_token)
		*next_token = current;
	
	return ret_val;
}
