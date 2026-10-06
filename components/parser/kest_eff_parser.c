#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "kest_int.h"

#define PRINTLINES_ALLOWED 0

static const char *FNAME = "kest_eff_parser.c";

const char *ver_str = "v1.0";

kest_bump_arena kest_eff_parser_mempool;
int kest_parser_mempool_initialised = 0;

kest_allocator *kest_parser_allocator = NULL;

int kest_eff_parser_init_mempool()
{
	KEST_PRINTF("kest_eff_parser_init_mempool\n");
	
	if (kest_parser_mempool_initialised)
		return NO_ERROR;
	
	int ret_val = kest_bump_arena_init(&kest_eff_parser_mempool, KEST_EFF_PARSER_MEM_POOL_SIZE_KB * 1024);
	kest_parser_mempool_initialised = (ret_val == NO_ERROR);
	kest_parser_allocator = &kest_eff_parser_mempool.alloc;
	return ret_val;
}

int kest_eff_parser_reset_mempool()
{
	KEST_PRINTF("kest_eff_parser_reset_mempool\n");
	if (!kest_parser_mempool_initialised)
		return ERR_BAD_ARGS;
	
	kest_bump_arena_reset(&kest_eff_parser_mempool);
	
	return NO_ERROR;
}

int kest_eff_parser_deinit_mempool()
{
	KEST_PRINTF("kest_eff_parser_deinit_mempool\n");
	kest_parser_mempool_initialised = 0;
	return kest_bump_arena_destroy(&kest_eff_parser_mempool);
}

#define PRINTLINES_ALLOWED 0

void *kest_parser_alloc(size_t size)
{
	return kest_bump_arena_alloc(&kest_eff_parser_mempool, size);
}

char *kest_parser_strndup(const char *str, int n)
{
	size_t len = strnlen(str, n);
    
    char *new_str = kest_parser_alloc(len + 1);
    
    if (!new_str) return NULL;
    memcpy(new_str, str, len);
    new_str[len] = '\0';
    
    return new_str;
}

char *kest_cname_from_name(char *name)
{
	if (!name) return NULL;
	
	char *cname = kest_parser_strndup(name, 128);
	
	if (!cname) return NULL;
	
	int i = 0;
	while (cname[i])
	{
		if ('A' <= cname[i] && cname[i] <= 'Z')
			cname[i] -= 'A' - 'a';
		
		if (cname[i] == ' ')
			cname[i] = '_';
		
		i++;
	}
	
	return cname;
}

static int kest_validate_discovery_info(kest_eff_parsing_state *ps, kest_ast_node *info);

int kest_parse_tokens(kest_eff_parsing_state *ps)
{
	if (!ps)
		return ERR_NULL_PTR;
	
	kest_token_ll *tokens = ps->tokens;
	kest_ast_node **root_ptr = &ps->ast;
	
	kest_ast_node *root = kest_parser_alloc(sizeof(kest_ast_node));
	
	if (!root)
		return ERR_ALLOC_FAIL;
	
	*root_ptr = root;
	
	root->type  = KEST_AST_NODE_ROOT;
	root->line  = 1;
	root->data  = NULL;
	root->child = NULL;
	root->next  = NULL;
	
	// Let's do a first pass to find the sections
	
	kest_ast_node *current_section = NULL;
	kest_ast_node *next_section	= NULL;
	kest_eff_desc_file_section *cs	= NULL;
	kest_eff_desc_file_section *ns	= NULL;
	kest_token_ll *current_token 	= tokens;
	kest_token_ll *prev_tokens[4]  = {NULL, NULL, NULL, NULL};
	
	kest_token_ll *current_sec_start = NULL;
	
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
					next_section = kest_parser_alloc(sizeof(kest_ast_node));
					
					if (!next_section)
						return ERR_ALLOC_FAIL;
					
					next_section->type  = KEST_AST_NODE_SECTION;
					next_section->line  = current_token->line;
					next_section->data  = NULL;
					next_section->next  = NULL;
					next_section->child = NULL;
					
					ns = kest_parser_alloc(sizeof(kest_eff_desc_file_section));
					
					if (!ns)
					{
						return ERR_ALLOC_FAIL;
					}
					
					ns->name = kest_parser_strndup(current_token->data, 16);
					
					if (!ns->name)
					{
						return ERR_ALLOC_FAIL;
					}
					
					ns->dict = NULL;
					ns->dict_ = (kest_eff_entry_dict){0};
					
					next_section->data = (void*)ns;
					
					if (current_section && cs)
					{
						cs->tokens = kest_token_span_to_ll(current_sec_start->next, prev_tokens[3]);
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
					kest_parser_error_at(ps, current_token, "Invalid section name \"%s\"", current_token->data);
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
		cs->tokens = kest_token_span_to_ll(current_sec_start->next, NULL);
	}
	
	// Now let's actually parse those sections
	
	current_section = root->child;
	
	// We need to do the code section last
	kest_ast_node *info_section = NULL;
	kest_ast_node *code_section = NULL;
	kest_ast_node *resources_section = NULL;
	kest_ast_node *parameters_section = NULL;
	kest_ast_node *settings_section = NULL;
	kest_ast_node *defs_section = NULL;
	
	kest_eff_desc_file_section *sect;
	
	int ret_val;
	while (current_section)
	{
		sect = (kest_eff_desc_file_section*)current_section->data;
		
		if (!sect)
		{
			current_section = current_section->next;
			continue;
		}
		
		if (strcmp(sect->name, "INFO") == 0)
		{
			if ((ret_val = kest_parse_entry_section(ps, current_section)) != NO_ERROR)
			{
				goto parse_fresh_sections_fail;
			}
			info_section = current_section;
		}
		else if (strcmp(sect->name, "RESOURCES") == 0)
		{
			if ((ret_val = kest_parse_entry_section(ps, current_section)) != NO_ERROR)
			{
				goto parse_fresh_sections_fail;
			}
			resources_section = current_section;
		}
		else if (strcmp(sect->name, "PARAMETERS") == 0)
		{
			if ((ret_val = kest_parse_entry_section(ps, current_section)) != NO_ERROR)
			{
				goto parse_fresh_sections_fail;
			}
			parameters_section = current_section;
		}
		else if (strcmp(sect->name, "SETTINGS") == 0)
		{
			if ((ret_val = kest_parse_entry_section(ps, current_section)) != NO_ERROR)
			{
				goto parse_fresh_sections_fail;
			}
			settings_section = current_section;
		}
		else if (strcmp(sect->name, "DEFS") == 0)
		{
			if ((ret_val = kest_parse_entry_section(ps, current_section)) != NO_ERROR)
			{
				goto parse_fresh_sections_fail;
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
			kest_parser_error(ps, "Invalid section name \"%s\"", sect->name);
			ret_val = ERR_BAD_ARGS;
			goto parse_fresh_sections_fail;
		}
	
		if (ret_val != NO_ERROR)
			goto parse_fresh_sections_fail;
		current_section = current_section->next;
	}
	
	kest_eff_entry *entry;
	
	if (info_section)
	{
		ret_val = kest_validate_discovery_info(ps, info_section);
		if (ret_val != NO_ERROR) goto parse_fresh_sections_fail;
		ps->info = info_section;
		entry = kest_eff_section_lookup(info_section, "name");
		
		if (!entry)
		{
			kest_parser_error(ps, "Effect name missing");
			ret_val = ERR_BAD_ARGS;
			goto parse_fresh_sections_fail;
		}
		else if (entry->type != KEST_EFF_ENTRY_TYPE_STR)
		{
			kest_parser_error(ps, "Effect name must be a string");
			ret_val = ERR_BAD_ARGS;
			goto parse_fresh_sections_fail;
		}
		else
		{
			KEST_PRINTF("Found name: \"%s\"\n", entry->value.val_string);
			ps->name = entry->value.val_string;
		}
		
		entry = kest_eff_section_lookup(info_section, "cname");
		
		if (entry && entry->type != KEST_EFF_ENTRY_TYPE_STR)
		{
			kest_parser_error(ps, "Effect cname must be a string");
			ret_val = ERR_BAD_ARGS;
			goto parse_fresh_sections_fail;
		}
		else if (entry)
		{
			KEST_PRINTF("Found cname: \"%s\"\n", entry->value.val_string);
			ps->cname = entry->value.val_string;
		}
		else
		{
			ps->cname = kest_cname_from_name(ps->name);
			KEST_PRINTF("Generated cname \"%s\"\n", ps->cname);
		}
	}
	else
	{
		kest_parser_error(ps, "INFO section missing");
		ret_val = ERR_BAD_ARGS;
		goto parse_fresh_sections_fail;
	}
	
	ps->scope = kest_parser_alloc(sizeof(kest_scope));
	
	if (!ps->scope)
	{
		ret_val = ERR_ALLOC_FAIL;
		goto parse_fresh_sections_fail;
	}
	if ((ret_val = kest_scope_init(ps->scope)) != NO_ERROR)
		goto parse_fresh_sections_fail;
	
	if (resources_section)
	{
		if ((ret_val = kest_resources_section_extract(ps, &ps->resources, resources_section)) != NO_ERROR)
		{
			return ret_val;
		}
		
		kest_resources_assign_handles(ps->resources);
	}
	
	if (parameters_section)
	{
		if ((ret_val = kest_parameters_section_extract(ps, &ps->parameters, parameters_section)) != NO_ERROR)
		{
			return ret_val;
		}
		
		kest_parameters_assign_ids(ps->parameters);
		kest_scope_add_params(ps->scope, ps->parameters);
	}
	
	if (settings_section)
	{
		if ((ret_val = kest_settings_section_extract(ps, &ps->settings, settings_section)) != NO_ERROR)
		{
			return ret_val;
		}
		
		kest_settings_assign_ids(ps->settings);
		KEST_PRINTF("Adding settings to scope...\n");
		kest_scope_add_settings(ps->scope, ps->settings);
	}
	
	if (defs_section)
	{
		KEST_PRINTF("Adding defs to scope...\n");
		if ((ret_val = kest_defs_section_extract(ps, ps->scope, defs_section)) != NO_ERROR)
		{
			return ret_val;
		}
	}
	
	if (code_section)
	{
		if ((ret_val = kest_parse_code_section(ps, code_section)) != NO_ERROR)
		{
			kest_parser_error_at(ps, ps->current_token, "Failed to parse .CODE section: %s", kest_error_code_to_string(ret_val));
			return ret_val;
		}
		
		KEST_PRINTF("Sucessfully parsed .CODE section\n");
	}
	
	return NO_ERROR;

parse_fresh_sections_fail:
	// Extraction has not begun; every section value is still a fresh parser tree.
	for (kest_ast_node *node = root->child; node; node = node->next)
	{
		kest_eff_desc_file_section *sec = node->data;
		if (!sec) continue;
		for (size_t i = 0; i < sec->dict_.count; i++)
			kest_free_parsed_eff_entry(&sec->dict_.entries[i]);
		sec->dict_.count = 0;
	}
	return ret_val;
}

static const char *discovery_list_names[] = { "keywords", "instruments", "types", "genres" };

static int kest_validate_discovery_info(kest_eff_parsing_state *ps, kest_ast_node *info)
{
	kest_eff_entry *entry = kest_eff_section_lookup(info, "description");
	if (entry && entry->type != KEST_EFF_ENTRY_TYPE_STR)
	{
		kest_parser_error_at_line(ps, entry->line, "Effect description must be a string");
		return ERR_BAD_ARGS;
	}
	for (size_t i = 0; i < sizeof(discovery_list_names) / sizeof(*discovery_list_names); i++)
	{
		entry = kest_eff_section_lookup(info, discovery_list_names[i]);
		if (!entry) continue;
		if (entry->type != KEST_EFF_ENTRY_TYPE_LIST || !entry->value.val_list)
			goto bad_list;
		for (size_t j = 0; j < entry->value.val_list->count; j++)
			if (entry->value.val_list->entries[j].type != KEST_EFF_ENTRY_TYPE_STR)
				goto bad_list;
		continue;
	bad_list:
		kest_parser_error_at_line(ps, entry->line, "Effect %s must be a string list", discovery_list_names[i]);
		return ERR_BAD_ARGS;
	}
	return NO_ERROR;
}

static int kest_copy_discovery_info(kest_effect_desc *desc, kest_ast_node *info)
{
	kest_eff_entry *entry = kest_eff_section_lookup(info, "description");
	if (entry)
	{
		desc->description = kest_strndup(entry->value.val_string, strlen(entry->value.val_string));
		if (!desc->description) return ERR_ALLOC_FAIL;
	}
	string_list *lists[] = { &desc->keywords, &desc->instruments, &desc->types, &desc->genres };
	for (size_t i = 0; i < sizeof(lists) / sizeof(*lists); i++)
	{
		entry = kest_eff_section_lookup(info, discovery_list_names[i]);
		if (!entry) continue;
		for (size_t j = 0; j < entry->value.val_list->count; j++)
		{
			const char *text = entry->value.val_list->entries[j].value.val_string;
			char *copy = kest_strndup(text, strlen(text));
			if (!copy) return ERR_ALLOC_FAIL;
			int ret = char_ptr_list_append(lists[i], copy);
			if (ret != NO_ERROR)
			{
				kest_free(copy);
				return ret;
			}
		}
	}
	return NO_ERROR;
}

int init_parsing_state(kest_eff_parsing_state *ps)
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
	ps->info = NULL;
	
	ps->asm_lines = NULL;
	ps->def_exprs = NULL;
	
	ps->errors = 0;
	ps->scope = NULL;
	ps->lines = NULL;
	ps->n_lines = 0;
	
	kest_driver_list_init(&ps->drivers);
	
	return NO_ERROR;
}

int kest_parser_lineize_content(kest_eff_parsing_state *ps)
{
	if (!ps)
		return ERR_NULL_PTR;
	
	if (!ps->content)
		return ERR_BAD_ARGS;
	
	if (!ps->n_lines)
		return NO_ERROR;
	
	int line = 1;
	ps->lines = kest_parser_alloc(sizeof(char*) * ps->n_lines);
	
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

int kest_parser_compute_formats(kest_eff_parsing_state *ps)
{
	if (!ps)
		return ERR_NULL_PTR;
	
	int format_result = kest_compute_register_formats(ps->blocks, ps->scope);
	if (format_result != NO_ERROR)
		return format_result;
	
	kest_dsp_resource_pll *res = ps->resources;
	kest_filter *filter;
	
	while (res)
	{
		if (res->data && res->data->type == KEST_DSP_RESOURCE_FILTER)
		{
			filter = (kest_filter*)res->data->data;
			
			if (!filter)
				break;
			
			kest_filter_compute_format(filter, ps->scope);
		}
		
		res = res->next;
	}
	
	return NO_ERROR;
}

kest_effect_desc *kest_read_eff_desc_from_file(char *fname)
{
	if (!fname)
		return NULL;
	
	KEST_PRINTF("kest_read_eff_desc_from_file\n");
	
	kest_effect_desc *result = NULL;
	kest_eff_parsing_state ps;
	kest_expression_ptr_list expressions;
	int capturing = 0;
	
	int ret_val;
	
	if (!kest_parser_mempool_initialised)
	{
		if ((ret_val = kest_eff_parser_init_mempool()) != NO_ERROR)
		{
			KEST_PRINTF("Error initialising parser mempool: %s\n", kest_error_code_to_string(ret_val));
			return NULL;
		}
	}
	
	init_parsing_state(&ps);
	
	FILE *src = fopen(fname, "r");
	
	if (!src)
	{
		KEST_PRINTF("Failed to open file \"%s\"!\n", fname);
		return NULL;
	}
	
	fseek(src, 0, SEEK_END);
	ps.file_size = ftell(src);
	fseek(src, SEEK_SET, 0);
	
	ps.content = kest_parser_alloc(ps.file_size * sizeof(char) + 1);
	
	if (!ps.content)
	{
		kest_parser_error(&ps, "File \"%s\" size %d too large\n", fname, ps.file_size);
		fclose(src);
		return NULL;
	}
	
	fread(ps.content, 1, ps.file_size, src);
	
	ps.content[ps.file_size] = 0;
	
	fclose(src);
	src = NULL;
	
	ps.fname = kest_parser_strndup(fname, 128);
	
	kest_tokenize_content(&ps);
	
	int j = 0;
	
	if (ps.errors != 0)
	{
		KEST_PRINTF("File \"%s\" ignored due to errors.\n", fname);
		return NULL;
	}
	
	kest_parser_lineize_content(&ps);
	if (kest_expression_capture_begin(&expressions) != NO_ERROR) return NULL;
	capturing = 1;
	
	ret_val = kest_parse_tokens(&ps);
	
	if (ps.errors != 0 || ret_val != NO_ERROR)
	{
		KEST_PRINTF("File \"%s\" ignored due to errors.\n", fname);
		goto read_eff_done;
	}
	
	if (ret_val == NO_ERROR)
	{
		KEST_PRINTF("File \"%s\" parsed sucessfully\n", fname);
	}
	else
	{
		KEST_PRINTF("File \"%s\" parsing failed. Error code: %s\n", fname, kest_error_code_to_string(ret_val));
	}
	
	ret_val = kest_process_asm_lines(&ps);
	
	if (ret_val != NO_ERROR)
	{
		goto read_eff_done;
	}
	
	ret_val = kest_parser_compute_formats(&ps);
	if (ret_val != NO_ERROR)
	{
		kest_parser_error_at(&ps, ps.current_token, "No permissible numeric formats: %s", kest_error_code_to_string(ret_val));
		goto read_eff_done;
	}
	
	char *name = kest_strndup(ps.name, 128);
	char *cname = kest_strndup(ps.cname, 128);
	if (!name || !cname)
	{
		kest_free(name);
		kest_free(cname);
		goto read_eff_done;
	}
	result = kest_allocator_alloc(&kest_effect_desc_allocator, sizeof(kest_effect_desc));
	if (!result)
	{
		kest_free(name);
		kest_free(cname);
	}
	
	if (result)
	{
		kest_init_effect_desc(result);
		
		result->cname = cname;
		result->name = name;
		if (kest_copy_discovery_info(result, ps.info) != NO_ERROR)
		{
			kest_effect_desc_retire(result);
			result = NULL;
			goto read_eff_done;
		}
		result->parameters = ps.parameters;
		result->resources = ps.resources;
		result->settings = ps.settings;
		result->blocks = ps.blocks;
		result->drivers = ps.drivers;
		
		result->expressions = expressions;
		
		result->def_exprs = ps.def_exprs;
		
		kest_effect_desc_generate_res_rpt(result);
	}
	
read_eff_done:
	// INFO is never extracted into the scope; copied discovery strings no longer borrow it.
	if (ps.info)
	{
		kest_eff_desc_file_section *info = ps.info->data;
		for (size_t i = 0; i < info->dict_.count; i++)
			kest_free_parsed_eff_entry(&info->dict_.entries[i]);
		info->dict_.count = 0;
	}
	if (ps.scope)
		kest_scope_entry_dict_destroy(&ps.scope->dict, NULL);
	if (!result)
	{
		kest_block_pll_free(ps.blocks);
		// Parameter metadata is owned here; bounds and driver keys borrow expression graphs.
		for (kest_parameter_pll *current = ps.parameters; current; current = current->next)
		{
			kest_free((void *)current->data->name_internal);
			kest_free((void *)current->data->name);
			kest_free((void *)current->data->units);
		}
		kest_parameter_pll_destroy(ps.parameters, kest_parameter_free);
		for (size_t i = 0; i < ps.drivers.count; i++)
			kest_free(ps.drivers.entries[i].data);
		kest_driver_list_destroy(&ps.drivers);
		// No descriptor received these resource containers; expressions remain separately owned.
		for (kest_dsp_resource_pll *current = ps.resources; current; current = current->next)
		{
			kest_dsp_resource *resource = current->data;
			kest_free(resource->name);
		}
		kest_dsp_resource_pll_destroy(ps.resources, kest_dsp_resource_free);
		// Definition metadata owns names/wrappers; expression graphs need separate ownership.
		for (kest_named_expression_pll *current = ps.def_exprs; current; current = current->next)
			kest_free((void *)current->data->name);
		kest_named_expression_pll_free(ps.def_exprs);
		// Parser-created settings own copied strings; no descriptor received this list.
		for (kest_setting_pll *current = ps.settings; current; current = current->next)
		{
			kest_free((void *)current->data->name_internal);
			kest_free((void *)current->data->name);
			kest_free((void *)current->data->units);
		}
		kest_setting_pll_destroy(ps.settings, kest_setting_free);
	}
	if (capturing)
	{
		kest_expression_capture_end();
		if (!result) kest_expression_capture_destroy(&expressions);
	}
	return result;
}

#define KEST_PARSER_PRINT_BUFLEN 1024
#define KEST_PARSER_PRINT_LOC_BUFLEN 128

const char *err_colour   = "\e[01;31m";
const char *info_colour  = "\e[01;36m";
const char *warn_colour  = "\e[01;32m";
const char *reset_colour = "\e[0m";

#define PR_LINE_INDENT 4

int kest_parser_format_offending_section(char *line, int index, int length, char *buf, int buf_len, const char *colour)
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
	
	for (int i = 0; i <= index + length / 2 + PR_LINE_INDENT - 1 && buf_pos + 1 < KEST_PARSER_PRINT_BUFLEN; i++)
		buf[buf_pos++] = '~';
	
	buf[buf_pos++] = '^';
	buf[buf_pos++] = 0;
	
	return NO_ERROR;
}

void kest_parser_print_info_at(kest_eff_parsing_state *ps, kest_token_ll *token, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && token)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, token->line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	KEST_PRINTF_FORCE_("%sINFO%s%s: %s\n", info_colour, loc_string, reset_colour, buf);
	
	int ret_val;
	if (token && token->data && token->line > 0 && token->line <= ps->n_lines
		&& ps->lines && ps->lines[token->line - 1])
	{
		ret_val = kest_parser_format_offending_section(ps->lines[token->line - 1], token->index, strlen(token->data), buf, KEST_PARSER_PRINT_BUFLEN, info_colour);
		
		if (ret_val == NO_ERROR)
			KEST_PRINTF_FORCE_("%s\n", buf);
	}
}

void kest_parser_warn_at(kest_eff_parsing_state *ps, kest_token_ll *token, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && token)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, token->line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	KEST_PRINTF_FORCE_("\e[01;32mWARNING%s\e[0m: %s\n", loc_string, buf);
	
	int ret_val;
	if (token && token->data && token->line > 0 && token->line <= ps->n_lines
		&& ps->lines && ps->lines[token->line - 1])
	{
		ret_val = kest_parser_format_offending_section(ps->lines[token->line - 1], token->index, strlen(token->data), buf, KEST_PARSER_PRINT_BUFLEN, warn_colour);
		
		if (ret_val == NO_ERROR)
			KEST_PRINTF_FORCE_("%s\n", buf);
	}
}

void kest_parser_error_at(kest_eff_parsing_state *ps, kest_token_ll *token, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	int tok_len;
	int buf_pos;
	int i;
	int j;
	int k;
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && token)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, token->line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	ps->errors++;
	KEST_PRINTF_FORCE_("%sERROR%s%s: %s\n", err_colour, loc_string, reset_colour, buf);
	
	int ret_val;
	if (token && token->data && token->line > 0 && token->line <= ps->n_lines
		&& ps->lines && ps->lines[token->line - 1])
	{
		ret_val = kest_parser_format_offending_section(ps->lines[token->line - 1], token->index, strlen(token->data), buf, KEST_PARSER_PRINT_BUFLEN, err_colour);
		
		if (ret_val == NO_ERROR)
			KEST_PRINTF_FORCE_("%s\n", buf);
	}
}

void kest_parser_print_info_at_line(kest_eff_parsing_state *ps, int line, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	KEST_PRINTF_FORCE_("\e[01;36mINFO%s\e[0m: %s\n", loc_string, buf);
}

void kest_parser_warn_at_line(kest_eff_parsing_state *ps, int line, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	KEST_PRINTF_FORCE_("\e[01;32mWARNING%s\e[0m: %s\n", loc_string, buf);
}

void kest_parser_error_at_line(kest_eff_parsing_state *ps, int line, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	ps->errors++;
	KEST_PRINTF_FORCE_("\e[01;31mERROR%s\e[0m: %s\n", loc_string, buf);
}

void kest_parser_print_info_at_node(kest_eff_parsing_state *ps, kest_ast_node *node, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && node)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, node->line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	KEST_PRINTF_FORCE_("\e[01;36mINFO%s\e[0m: %s\n", loc_string, buf);
}

void kest_parser_warn_at_node(kest_eff_parsing_state *ps, kest_ast_node *node, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && node)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, node->line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	KEST_PRINTF_FORCE_("\e[01;32mWARNING%s\e[0m: %s\n", loc_string, buf);
}

void kest_parser_error_at_node(kest_eff_parsing_state *ps, kest_ast_node *node, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && node)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, node->line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	ps->errors++;
	KEST_PRINTF_FORCE_("\e[01;31mERROR%s\e[0m: %s\n", loc_string, buf);
}

void kest_parser_print_info(kest_eff_parsing_state *ps, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && ps->current_token)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, ps->current_token->line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	KEST_PRINTF_FORCE_("\e[01;36INFO%s\e[0m: %s\n", loc_string, buf);
}

void kest_parser_warn(kest_eff_parsing_state *ps, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && ps->current_token)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, ps->current_token->line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	KEST_PRINTF_FORCE_("\e[01;32mWARNING%s\e[0m: %s\n", loc_string, buf);
}

void kest_parser_error(kest_eff_parsing_state *ps, const char *msg, ...)
{
	char buf[KEST_PARSER_PRINT_BUFLEN];
	va_list args;
	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	
	char loc_string[KEST_PARSER_PRINT_LOC_BUFLEN];
	if (ps->fname && ps->current_token)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s:%d)", ps->fname, ps->current_token->line);
	else if (ps->fname)
		snprintf(loc_string, KEST_PARSER_PRINT_LOC_BUFLEN, " (%s)", ps->fname);
	else
		loc_string[0] = 0;
	
	ps->errors++;
	
	KEST_PRINTF_FORCE_("\e[01;31mERROR%s\e[0m: %s\n", loc_string, buf);
}
