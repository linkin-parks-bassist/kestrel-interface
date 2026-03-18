#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "m_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 1
#endif

static const char *FNAME = "m_eff_parser.c";

const char *ver_str = "v1.0";

m_bump_arena m_eff_parser_mempool;
int m_parser_mempool_initialised = 0;

m_allocator *m_parser_allocator = NULL;

int m_eff_parser_init_mempool()
{
	if (m_parser_mempool_initialised)
		return NO_ERROR;
	
	int ret_val = m_bump_arena_init(&m_eff_parser_mempool, M_EFF_PARSER_MEM_POOL_SIZE_KB * 1024);
	m_parser_mempool_initialised = (ret_val == NO_ERROR);
	m_parser_allocator = &m_eff_parser_mempool.alloc;
	return ret_val;
}

int m_eff_parser_reset_mempool()
{
	if (!m_parser_mempool_initialised)
		return ERR_BAD_ARGS;
	
	m_bump_arena_reset(&m_eff_parser_mempool);
	
	return NO_ERROR;
}

int m_eff_parser_deinit_mempool()
{
	m_parser_mempool_initialised = 0;
	return m_bump_arena_destroy(&m_eff_parser_mempool);
}

void *m_parser_alloc(size_t size)
{
	return m_bump_arena_alloc(&m_eff_parser_mempool, size);
}

char *m_parser_strndup(const char *str, int n)
{
	size_t len = strnlen(str, n);
    
    char *new_str = m_parser_alloc(len + 1);
    
    if (!new_str) return NULL;
    memcpy(new_str, str, len);
    new_str[len] = '\0';
    
    return new_str;
}

int m_parse_tokens(m_eff_parsing_state *ps)
{
	if (!ps)
		return ERR_NULL_PTR;
	
	m_token_ll *tokens = ps->tokens;
	m_ast_node **root_ptr = &ps->ast;
	
	m_ast_node *root = m_parser_alloc(sizeof(m_ast_node));
	
	if (!root)
		return ERR_ALLOC_FAIL;
	
	*root_ptr = root;
	
	root->type  = M_AST_NODE_ROOT;
	root->data  = NULL;
	root->child = NULL;
	root->next  = NULL;
	
	// Let's do a first pass to find the sections
	
	m_ast_node *current_section = NULL;
	m_ast_node *next_section	= NULL;
	m_eff_desc_file_section *cs	= NULL;
	m_eff_desc_file_section *ns	= NULL;
	m_token_ll *current_token 	= tokens;
	m_token_ll *prev_tokens[4]  = {NULL, NULL, NULL, NULL};
	
	m_token_ll *current_sec_start = NULL;
	
	int section_start_score = 0;
	int next_section_start_score;
	
	int token_n = 0;
	int line = 1;
	
	while (current_token)
	{
		token_n++;
		
		if (current_token->data)
		{
			if (token_is_newline(current_token->data))
			{
				line++;
				token_n = 1;
			}
			
			next_section_start_score = get_section_start_score(current_token->data, section_start_score);
			
			if (section_start_score == 2)
			{
				if (next_section_start_score == 3)
				{
					next_section = m_parser_alloc(sizeof(m_ast_node));
					
					if (!next_section)
						return ERR_ALLOC_FAIL;
					
					next_section->type  = M_AST_NODE_SECTION;
					next_section->data  = NULL;
					next_section->next  = NULL;
					next_section->child = NULL;
					
					ns = m_parser_alloc(sizeof(m_eff_desc_file_section));
					
					if (!ns)
					{
						return ERR_ALLOC_FAIL;
					}
					
					ns->name = m_parser_strndup(current_token->data, 16);
					
					if (!ns->name)
					{
						return ERR_ALLOC_FAIL;
					}
					
					ns->dict = NULL;
					
					next_section->data = (void*)ns;
					
					if (current_section && cs)
					{
						cs->tokens = m_token_span_to_ll(current_sec_start->next, prev_tokens[3]);
						current_section->next = next_section;
					}
					else
					{
						root->child = next_section;
					}
					
					current_section = next_section;
					current_sec_start = current_token;
					cs = ns;
				}
				else
				{
					m_parser_error_at(ps, current_token, "Invalid section name \"%s\"", current_token->data);
					return ERR_BAD_ARGS;
				}
				
				section_start_score = 0;
			}
			else
			{
				section_start_score = next_section_start_score;
			}
		}
		
		prev_tokens[0] = current_token;
		prev_tokens[1] = prev_tokens[0];
		prev_tokens[2] = prev_tokens[1];
		prev_tokens[3] = prev_tokens[2];
		
		current_token = current_token->next;
	}
	
	if (current_section)
	{
		cs->tokens = m_token_span_to_ll(current_sec_start->next, NULL);
	}
	
	// Now let's actually parse those sections
	
	current_section = root->child;
	
	// We need to do the code section last
	m_ast_node *info_section = NULL;
	m_ast_node *code_section = NULL;
	m_ast_node *resources_section = NULL;
	m_ast_node *parameters_section = NULL;
	m_ast_node *settings_section = NULL;
	m_ast_node *defs_section = NULL;
	
	m_eff_desc_file_section *sect;
	
	int ret_val;
	while (current_section)
	{
		sect = (m_eff_desc_file_section*)current_section->data;
		
		if (!sect)
		{
			current_section = current_section->next;
			continue;
		}
		
		if (strcmp(sect->name, "INFO") == 0)
		{
			if ((ret_val = m_parse_dictionary_section(ps, current_section)) != NO_ERROR)
			{
				return ret_val;
			}
			info_section = current_section;
		}
		else if (strcmp(sect->name, "RESOURCES") == 0)
		{
			if ((ret_val = m_parse_dictionary_section(ps, current_section)) != NO_ERROR)
			{
				return ret_val;
			}
			resources_section = current_section;
		}
		else if (strcmp(sect->name, "PARAMETERS") == 0)
		{
			if ((ret_val = m_parse_dictionary_section(ps, current_section)) != NO_ERROR)
			{
				return ret_val;
			}
			parameters_section = current_section;
		}
		else if (strcmp(sect->name, "SETTINGS") == 0)
		{
			if ((ret_val = m_parse_dictionary_section(ps, current_section)) != NO_ERROR)
			{
				return ret_val;
			}
			settings_section = current_section;
		}
		else if (strcmp(sect->name, "DEFS") == 0)
		{
			if ((ret_val = m_parse_dictionary_section(ps, current_section)) != NO_ERROR)
			{
				return ret_val;
			}
			defs_section = current_section;
		}
		else if (strcmp(sect->name, "CODE") == 0)
		{
			code_section = current_section;
			ret_val = NO_ERROR;
		}
		else
		{
			m_parser_error(ps, "Invalid section name \"%s\"", sect->name);
			return ERR_BAD_ARGS;
		}
	
		if (ret_val != NO_ERROR)
			return ret_val;
		current_section = current_section->next;
	}
	
	if (info_section)
	{
		if ((ret_val = m_dictionary_section_lookup_str(info_section, "name", &ps->name)) != NO_ERROR)
		{
			m_parser_error(ps, "Effect name missing");
			return ret_val;
		}
		else
		{
			M_PRINTF("Found name: \"%s\"\n", ps->name ? ps->name : "(NULL)");
		}
		if ((ret_val = m_dictionary_section_lookup_str(info_section, "cname", &ps->cname)) != NO_ERROR)
		{
			m_parser_error(ps, "Effect cname missing");
			return ret_val;
		}
		else
		{
			M_PRINTF("Found cname: \"%s\"\n", ps->cname ? ps->cname : "(NULL)");
		}
	}
	else
	{
		m_parser_error(ps, "INFO section missing");
		return ERR_BAD_ARGS;
	}
	
	ps->scope = m_parser_alloc(sizeof(m_expr_scope));
	m_expr_scope_init(ps->scope);
	
	if (!ps->scope)
		return ERR_ALLOC_FAIL;
	
	if (resources_section)
	{
		if ((ret_val = m_resources_section_extract(ps, &ps->resources, resources_section)) != NO_ERROR)
		{
			return ret_val;
		}
		
		m_resources_assign_handles(ps->resources);
	}
	
	if (parameters_section)
	{
		if ((ret_val = m_parameters_section_extract(ps, &ps->parameters, parameters_section)) != NO_ERROR)
		{
			return ret_val;
		}
		
		m_parameters_assign_ids(ps->parameters);
		m_expr_scope_add_params(ps->scope, ps->parameters);
	}
	
	if (settings_section)
	{
		if ((ret_val = m_settings_section_extract(ps, &ps->settings, settings_section)) != NO_ERROR)
		{
			return ret_val;
		}
		
		m_settings_assign_ids(ps->settings);
		M_PRINTF("Adding settings to scope...\n");
		m_expr_scope_add_settings(ps->scope, ps->settings);
	}
	
	if (defs_section)
	{
		M_PRINTF("Adding defs to scope...\n");
		if ((ret_val = m_defs_section_extract(ps, ps->scope, defs_section)) != NO_ERROR)
		{
			return ret_val;
		}
	}
	
	if (code_section)
	{
		if ((ret_val = m_parse_code_section(ps, code_section)) != NO_ERROR)
		{
			return ret_val;
		}
	}
	
	return NO_ERROR;
}

int init_parsing_state(m_eff_parsing_state *ps)
{
	if (!ps) return ERR_NULL_PTR;
	
	ps->tokens = NULL;
	ps->current_token = NULL;
	
	ps->parameters = NULL;
	ps->resources = NULL;
	ps->settings = NULL;
	ps->blocks = NULL;
	ps->cname = NULL;
	ps->name = NULL;
	
	ps->asm_lines = NULL;
	ps->def_exprs = NULL;
	
	ps->errors = 0;
	ps->scope = NULL;
	ps->lines = NULL;
	ps->n_lines = 0;
	
	return NO_ERROR;
}

int m_parser_lineize_content(m_eff_parsing_state *ps)
{
	if (!ps)
		return ERR_NULL_PTR;
	
	if (!ps->content)
		return ERR_BAD_ARGS;
	
	if (!ps->n_lines)
		return NO_ERROR;
	
	int line = 1;
	ps->lines = m_parser_alloc(sizeof(char*) * ps->n_lines);
	
	if (!ps->lines)
		return ERR_ALLOC_FAIL;
	
	for (int i = 0; i < ps->n_lines; i++)
		ps->lines[i] = NULL;
	
	ps->lines[0] = ps->content;
	
	int line_start = 0;
	
	for (int i = 0; i < ps->file_size && line < ps->n_lines; i++)
	{
		if (line_start)
			ps->lines[line++] = &ps->content[i];
		
		line_start = 0;
		
		if (ps->content[i] == '\n')
		{
			ps->content[i] = 0;
			line_start = 1;
		}
	}
	
	// It may be the case that, if the final line is just "\n", the loop will end one
	// line early. Then, there is a risk of segfault if one tries to deref ps->lines[n_lines-1]
	// therefore, decrement ps->n_lines to prevent this.
	ps->n_lines = line;
	
	return NO_ERROR;
}

int m_parser_compute_formats(m_eff_parsing_state *ps)
{
	if (!ps)
		return ERR_NULL_PTR;
	
	m_compute_register_formats(ps->blocks, ps->scope);
	
	m_dsp_resource_pll *res = ps->resources;
	m_filter *filter;
	
	while (res)
	{
		if (res->data && res->data->type == M_DSP_RESOURCE_FILTER)
		{
			filter = (m_filter*)res->data->data;
			
			if (!filter)
				break;
			
			m_filter_compute_format(filter, ps->scope);
		}
		
		res = res->next;
	}
	
	return NO_ERROR;
}

m_effect_desc *m_read_eff_desc_from_file(char *fname)
{
	if (!fname)
		return NULL;
	
	printf("m_read_eff_desc_from_file\n");
	
	m_effect_desc *result = NULL;
	m_eff_parsing_state ps;
	
	int ret_val;
	
	if (!m_parser_mempool_initialised)
	{
		if ((ret_val = m_eff_parser_init_mempool()) != NO_ERROR)
		{
			M_PRINTF("Error initialising parser mempool: %s\n", m_error_code_to_string(ret_val));
			return NULL;
		}
	}
	
	init_parsing_state(&ps);
	
	FILE *src = fopen(fname, "r");
	
	if (!src)
	{
		M_PRINTF("Failed to open file \"%s\"!\n", fname);
		return NULL;
	}
	
	fseek(src, 0, SEEK_END);
	ps.file_size = ftell(src);
	fseek(src, SEEK_SET, 0);
	
	ps.content = m_parser_alloc(ps.file_size * sizeof(char) + 1);
	
	if (!ps.content)
	{
		m_parser_error(&ps, "File \"%s\" size %d too large\n", fname, ps.file_size);
		fclose(src);
		return NULL;
	}
	
	fread(ps.content, 1, ps.file_size, src);
	
	ps.content[ps.file_size] = 0;
	
	fclose(src);
	src = NULL;
	
	ps.fname = m_parser_strndup(fname, 128);
	
	m_tokenize_content(&ps);
	
	int j = 0;
	
	if (ps.errors != 0)
	{
		M_PRINTF("File \"%s\" ignored due to errors.\n", fname);
		return NULL;
	}
	
	m_parser_lineize_content(&ps);
	
	ret_val = m_parse_tokens(&ps);
	
	if (ps.errors != 0)
	{
		M_PRINTF("File \"%s\" ignored due to errors.\n", fname);
		if (ps.parameters)
		{
			free_m_parameter_pll(ps.parameters);
		}
		if (ps.resources)
		{
			free_m_dsp_resource_pll(ps.resources);
		}
		if (ps.blocks)
		{
			free_m_block_pll(ps.blocks);
		}
		return NULL;
	}
	
	if (ret_val == NO_ERROR)
	{
		M_PRINTF("File \"%s\" parsed sucessfully\n", fname);
	}
	else
	{
		M_PRINTF("File \"%s\" parsing failed. Error code: %s\n", fname, m_error_code_to_string(ret_val));
	}
	
	ret_val = m_process_asm_lines(&ps);
	
	if (ret_val != NO_ERROR)
	{
		return NULL;
	}
	
	m_parser_compute_formats(&ps);
	
	result = m_alloc(sizeof(m_effect_desc));
	
	if (result)
	{
		m_init_effect_desc(result);
		
		result->parameters = ps.parameters;
		result->resources = ps.resources;
		result->settings = ps.settings;
		result->blocks = ps.blocks;
		
		result->cname = m_strndup(ps.cname, 128);
		result->name = m_strndup(ps.name, 128);
		
		result->def_exprs = ps.def_exprs;
		
		m_effect_desc_generate_res_rpt(result);
	}
	
	return result;
}

#define M_PARSER_PRINT_BUFLEN 1024
#define M_PARSER_PRINT_LOC_BUFLEN 128

const char *err_colour   = "\e[01;31m";
const char *info_colour  = "\e[01;36m";
const char *warn_colour  = "\e[01;32m";
const char *reset_colour = "\e[0m";

#define PR_LINE_INDENT 4

int m_parser_format_offending_section(char *line, int index, int length, char *buf, int buf_len, char *colour)
{
	if (!line || !buf)
		return ERR_NULL_PTR;

	int i, j, buf_pos;
	
	buf_pos = 0;
		
	for (i = 0; i < PR_LINE_INDENT; i++)
		buf[buf_pos++] = ' ';
	
	for (i = 0; i < index && buf_pos + 1 < buf_len && line[i] != 0; i++)
		buf[buf_pos++] = line[i];
	
	if (colour)
	{
		for (j = 0; err_colour[j] != 0 && buf_pos + 1 < buf_len; j++)
			buf[buf_pos++] = colour[j];
	}
	
	for (j = 0; j < length && buf_pos < buf_len; j++)
		buf[buf_pos++] = line[i++];
	
	if (colour)
	{
		for (j = 0; reset_colour[j] != 0 && buf_pos + 1 < buf_len; j++)
			buf[buf_pos++] = reset_colour[j];
	}
	
	for (j = 0; line[i] != 0 && buf_pos + 1 < buf_len; j++)
		buf[buf_pos++] = line[i++];
	
	if (buf_pos + index + length / 2 + 1 + PR_LINE_INDENT >= buf_len)
	{
		buf[buf_pos < buf_len ? buf_pos : buf_len - 1] = 0;
		return NO_ERROR;
	}
	else
	{
		buf[buf_pos++] = '\n';
	}
	
	for (int i = 0; i <= index + length / 2 + PR_LINE_INDENT - 1 && buf_pos + 1 < M_PARSER_PRINT_BUFLEN; i++)
		buf[buf_pos++] = '~';
	
	buf[buf_pos++] = '^';
	buf[buf_pos++] = 0;
	
	return NO_ERROR;
}

void m_parser_print_info_at(m_eff_parsing_state *ps, m_token_ll *token, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && token)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, token->line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	M_PRINTF_("%sINFO%s%s: %s\n", info_colour, loc_string, reset_colour, buf);
	
	int ret_val;
	if (token && token->line < ps->n_lines && ps->lines && ps->lines[token->line])
	{
		ret_val = m_parser_format_offending_section(ps->lines[token->line - 1], token->index, strlen(token->data), buf, M_PARSER_PRINT_BUFLEN, info_colour);
		
		if (ret_val == NO_ERROR)
			M_PRINTF_("%s\n", buf);
	}
}

void m_parser_warn_at(m_eff_parsing_state *ps, m_token_ll *token, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && token)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, token->line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	M_PRINTF_("\e[01;32mWARNING%s\e[0m: %s\n", loc_string, buf);
	
	int ret_val;
	if (token && token->line < ps->n_lines && ps->lines && ps->lines[token->line])
	{
		ret_val = m_parser_format_offending_section(ps->lines[token->line - 1], token->index, strlen(token->data), buf, M_PARSER_PRINT_BUFLEN, warn_colour);
		
		if (ret_val == NO_ERROR)
			M_PRINTF_("%s\n", buf);
	}
}

void m_parser_error_at(m_eff_parsing_state *ps, m_token_ll *token, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	int tok_len;
	int buf_pos;
	int i;
	int j;
	int k;
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && token)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, token->line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	ps->errors++;
	M_PRINTF_("%sERROR%s%s: %s\n", err_colour, loc_string, reset_colour, buf);
	
	int ret_val;
	if (token && token->line < ps->n_lines && ps->lines && ps->lines[token->line])
	{
		ret_val = m_parser_format_offending_section(ps->lines[token->line - 1], token->index, strlen(token->data), buf, M_PARSER_PRINT_BUFLEN, err_colour);
		
		if (ret_val == NO_ERROR)
			M_PRINTF_("%s\n", buf);
	}
}

void m_parser_print_info_at_line(m_eff_parsing_state *ps, int line, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	M_PRINTF_("\e[01;36mINFO%s\e[0m: %s\n", loc_string, buf);
}

void m_parser_warn_at_line(m_eff_parsing_state *ps, int line, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	M_PRINTF_("\e[01;32mWARNING%s\e[0m: %s\n", loc_string, buf);
}

void m_parser_error_at_line(m_eff_parsing_state *ps, int line, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	ps->errors++;
	M_PRINTF_("\e[01;31mERROR%s\e[0m: %s\n", loc_string, buf);
}

void m_parser_print_info_at_node(m_eff_parsing_state *ps, m_ast_node *node, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && node)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, node->line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	M_PRINTF_("\e[01;36mINFO%s\e[0m: %s\n", loc_string, buf);
}

void m_parser_warn_at_node(m_eff_parsing_state *ps, m_ast_node *node, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && node)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, node->line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	M_PRINTF_("\e[01;32mWARNING%s\e[0m: %s\n", loc_string, buf);
}

void m_parser_error_at_node(m_eff_parsing_state *ps, m_ast_node *node, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && node)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, node->line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	ps->errors++;
	M_PRINTF_("\e[01;31mERROR%s\e[0m: %s\n", loc_string, buf);
}

void m_parser_print_info(m_eff_parsing_state *ps, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && ps->current_token)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, ps->current_token->line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	M_PRINTF_("\e[01;36INFO%s\e[0m: %s\n", loc_string, buf);
}

void m_parser_warn(m_eff_parsing_state *ps, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && ps->current_token)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, ps->current_token->line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	M_PRINTF_("\e[01;32mWARNING%s\e[0m: %s\n", loc_string, buf);
}

void m_parser_error(m_eff_parsing_state *ps, const char *msg, ...)
{
	char buf[M_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[M_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && ps->current_token)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, ps->current_token->line);
	else if (ps->fname)
		snprintf(loc_string, M_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	ps->errors++;
	
	M_PRINTF_("\e[01;31mERROR%s\e[0m: %s\n", loc_string, buf);
}
