#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kest_int.h"

//#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
//#endif

static const char *FNAME = "kest_asm_parser.c";

#define CH_DELIMITER 'c'
#define KEST_EXPR_DELIMITER "["
#define RS_DELIMITER "$"
#define DQ_CODELIMITER "]"

IMPLEMENT_LINKED_PTR_LIST(kest_asm_line);

static const char *instrs [] = {
	"nop", "mov", "add", "sub", "mul",
	"madd", "arsh", "lsh", "rsh", "abs",
	"min", "max", "clamp",
	"mov_acc", "mov_lacc", "mov_uacc",
	"macz", "umacz", "mac", "umac",
	"delay_read", "delay_write", "delay_mwrite",
	"mem_read", "mem_write", "filter", "fcasc",
	"tanh4", "sin2pi", "svf", "svf_low", "svf_high",
	"svf_band", "poly"
};

static const int n_instrs = sizeof(instrs) / sizeof(instrs[0]);

static const kest_arg_format arg_format_std_0 = {
	.n_args = 0,
	
	.arg_a_pos = KEST_ARG_POS_NONE,
	.arg_b_pos = KEST_ARG_POS_NONE,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.dest_pos  = KEST_ARG_POS_NONE,
	.shift_pos = KEST_ARG_POS_NONE
};

static const kest_arg_format arg_format_std_1 = {
	.n_args = 2,
	
	.arg_a_pos = 0,
	.arg_b_pos = KEST_ARG_POS_NONE,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.res_pos   = KEST_ARG_POS_NONE,
	.dest_pos  = 1,
	.shift_pos = KEST_ARG_POS_NONE
};

static const kest_arg_format arg_format_std_2 = {
	.n_args = 3,
	
	.arg_a_pos = 0,
	.arg_b_pos = 1,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.res_pos   = KEST_ARG_POS_NONE,
	.dest_pos  = 2,
	.shift_pos = KEST_ARG_POS_NONE
};

static const kest_arg_format arg_format_mac = {
	.n_args = 2,
	
	.arg_a_pos = 0,
	.arg_b_pos = 1,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.res_pos   = KEST_ARG_POS_NONE,
	.dest_pos  = KEST_ARG_POS_NONE,
	.shift_pos = KEST_ARG_POS_NONE
};

static const kest_arg_format arg_format_add = {
	.n_args = 3,
	
	.arg_a_pos = 0,
	.arg_b_pos = KEST_ARG_POS_NONE,
	.arg_c_pos = 1,
	.res_pos   = KEST_ARG_POS_NONE,
	.dest_pos  = 2,
	.shift_pos = KEST_ARG_POS_NONE
};

static const kest_arg_format arg_format_std_3 = {
	.n_args = 4,
	
	.arg_a_pos = 0,
	.arg_b_pos = 1,
	.arg_c_pos = 2,
	.res_pos   = KEST_ARG_POS_NONE,
	.dest_pos  = 3,
	.shift_pos = KEST_ARG_POS_NONE
};

static const kest_arg_format arg_format_shift = {
	.n_args = 3,
	
	.arg_a_pos = 0,
	.arg_b_pos = 1,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.res_pos   = KEST_ARG_POS_NONE,
	.dest_pos  = 3,
	.shift_pos = 2
};

static const kest_arg_format arg_format_read = {
	.n_args = 1,
	
	.arg_a_pos = KEST_ARG_POS_NONE,
	.arg_b_pos = KEST_ARG_POS_NONE,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.res_pos   = KEST_ARG_POS_NONE,
	.dest_pos  = 0,
	.shift_pos = KEST_ARG_POS_NONE
};
/*
static const kest_arg_format arg_format_write = {
	.n_args = 1,
	
	.arg_a_pos = 0,
	.arg_b_pos = KEST_ARG_POS_NONE,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.res_pos   = KEST_ARG_POS_NONE,
	.dest_pos  = KEST_ARG_POS_NONE,
	.shift_pos = KEST_ARG_POS_NONE
};
*/
static const kest_arg_format arg_format_res_read = {
	.n_args = 2,
	
	.arg_a_pos = KEST_ARG_POS_NONE,
	.arg_b_pos = KEST_ARG_POS_NONE,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.res_pos   = 0,
	.dest_pos  = 1,
	.shift_pos = KEST_ARG_POS_NONE
};
/*
static const kest_arg_format arg_format_res_read_2 = {
	.n_args = 3,
	
	.arg_a_pos = 1,
	.arg_b_pos = KEST_ARG_POS_NONE,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.res_pos   = 0,
	.dest_pos  = 2,
	.shift_pos = KEST_ARG_POS_NONE
};
*/
static const kest_arg_format arg_format_res_write = {
	.n_args = 2,
	
	.arg_a_pos = 0,
	.arg_b_pos = KEST_ARG_POS_NONE,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.res_pos   = 1,
	.dest_pos  = KEST_ARG_POS_NONE,
	.shift_pos = KEST_ARG_POS_NONE
};
/*
static const kest_arg_format arg_format_res_write_2 = {
	.n_args = 3,
	
	.arg_a_pos = 0,
	.arg_b_pos = 1,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.res_pos   = 2,
	.dest_pos  = KEST_ARG_POS_NONE,
	.shift_pos = KEST_ARG_POS_NONE
};
*/
static const kest_arg_format arg_format_res_rw = {
	.n_args = 3,
	
	.arg_a_pos = 0,
	.arg_b_pos = KEST_ARG_POS_NONE,
	.arg_c_pos = KEST_ARG_POS_NONE,
	.res_pos   = 1,
	.dest_pos  = 2,
	.shift_pos = KEST_ARG_POS_NONE
};

const kest_arg_format *kest_instr_arg_format(const char *instr)
{
	if (!instr) return NULL;
	
	if (strcmp(instr, "nop"         ) == 0) return &arg_format_std_0;
	if (strcmp(instr, "mov"         ) == 0) return &arg_format_std_1;
	if (strcmp(instr, "add"         ) == 0) return &arg_format_add;
	if (strcmp(instr, "sub"         ) == 0) return &arg_format_std_2;
	if (strcmp(instr, "mul"         ) == 0) return &arg_format_std_2;
	if (strcmp(instr, "madd"        ) == 0) return &arg_format_std_3;
	if (strcmp(instr, "arsh"        ) == 0) return &arg_format_shift;
	if (strcmp(instr, "lsh"         ) == 0) return &arg_format_shift;
	if (strcmp(instr, "rsh"         ) == 0) return &arg_format_shift;
	if (strcmp(instr, "abs"         ) == 0) return &arg_format_std_1;
	if (strcmp(instr, "min"         ) == 0) return &arg_format_std_2;
	if (strcmp(instr, "max"         ) == 0) return &arg_format_std_2;
	if (strcmp(instr, "clamp"       ) == 0) return &arg_format_std_3;
	if (strcmp(instr, "mov_acc"     ) == 0) return &arg_format_read;
	if (strcmp(instr, "mov_lacc"    ) == 0) return &arg_format_read;
	if (strcmp(instr, "mov_uacc"    ) == 0) return &arg_format_read;
	if (strcmp(instr, "macz"        ) == 0) return &arg_format_mac;
	if (strcmp(instr, "umacz"       ) == 0) return &arg_format_mac;
	if (strcmp(instr, "mac"         ) == 0) return &arg_format_mac;
	if (strcmp(instr, "umac"        ) == 0) return &arg_format_mac;
	if (strcmp(instr, "delay_read"  ) == 0) return &arg_format_res_read;
	if (strcmp(instr, "delay_write" ) == 0) return &arg_format_res_write;
	if (strcmp(instr, "mem_read"    ) == 0) return &arg_format_res_read;
	if (strcmp(instr, "mem_write"   ) == 0) return &arg_format_res_write;
	if (strcmp(instr, "filter"      ) == 0) return &arg_format_res_rw;
	if (strcmp(instr, "fcasc"       ) == 0) return &arg_format_res_read;
	if (strcmp(instr, "tanh4"       ) == 0) return &arg_format_std_1;
	if (strcmp(instr, "sin2pi"      ) == 0) return &arg_format_std_1;
	
	return NULL;
}

int kest_instr_opcode(const char *instr)
{
	if (!instr) return BLOCK_INSTR_NOP;
	
	if (strcmp(instr, "nop"         ) == 0) return BLOCK_INSTR_NOP;
	if (strcmp(instr, "mov"         ) == 0) return BLOCK_INSTR_MADD;
	if (strcmp(instr, "add"         ) == 0) return BLOCK_INSTR_MADD;
	if (strcmp(instr, "sub"         ) == 0) return BLOCK_INSTR_MADD;
	if (strcmp(instr, "mul"         ) == 0) return BLOCK_INSTR_MADD;
	if (strcmp(instr, "madd"        ) == 0) return BLOCK_INSTR_MADD;
	if (strcmp(instr, "arsh"        ) == 0) return BLOCK_INSTR_ARSH;
	if (strcmp(instr, "lsh"         ) == 0) return BLOCK_INSTR_LSH;
	if (strcmp(instr, "rsh"         ) == 0) return BLOCK_INSTR_RSH;
	if (strcmp(instr, "abs"         ) == 0) return BLOCK_INSTR_ABS;
	if (strcmp(instr, "min"         ) == 0) return BLOCK_INSTR_MIN;
	if (strcmp(instr, "max"         ) == 0) return BLOCK_INSTR_MAX;
	if (strcmp(instr, "clamp"       ) == 0) return BLOCK_INSTR_CLAMP;
	if (strcmp(instr, "mov_acc"     ) == 0) return BLOCK_INSTR_MOV_ACC;
	if (strcmp(instr, "mov_lacc"    ) == 0) return BLOCK_INSTR_MOV_LACC;
	if (strcmp(instr, "mov_uacc"    ) == 0) return BLOCK_INSTR_MOV_UACC;
	if (strcmp(instr, "macz"        ) == 0) return BLOCK_INSTR_MACZ;
	if (strcmp(instr, "umacz"       ) == 0) return BLOCK_INSTR_UMACZ;
	if (strcmp(instr, "mac"         ) == 0) return BLOCK_INSTR_MAC;
	if (strcmp(instr, "umac"        ) == 0) return BLOCK_INSTR_UMAC;
	if (strcmp(instr, "delay_read"  ) == 0) return BLOCK_INSTR_DELAY_READ;
	if (strcmp(instr, "delay_write" ) == 0) return BLOCK_INSTR_DELAY_WRITE;
	if (strcmp(instr, "mem_read"    ) == 0) return BLOCK_INSTR_MEM_READ;
	if (strcmp(instr, "mem_write"   ) == 0) return BLOCK_INSTR_MEM_WRITE;
	if (strcmp(instr, "filter"      ) == 0) return BLOCK_INSTR_FILTER;
	if (strcmp(instr, "fcasc"       ) == 0) return BLOCK_INSTR_FCASC;
	if (strcmp(instr, "tanh4"       ) == 0) return BLOCK_INSTR_LUT_READ;
	if (strcmp(instr, "sin2pi"      ) == 0) return BLOCK_INSTR_LUT_READ;
	
	return BLOCK_INSTR_NOP;
}

int kest_parse_asm_arg(kest_eff_parsing_state *ps, kest_asm_operand *arg)
{
	if (!ps)
		return ERR_NULL_PTR;
	
	kest_token_ll *current = ps->current_token;
	kest_token_ll *tok;
	
	kest_dsp_resource_pll *current_res;
	int resource_found;
	
	int ret_val = NO_ERROR;
	
	int n;
	int valid;
	
	if (!current || !current->data)
	{
		ret_val = ERR_BAD_ARGS;
		goto asm_parse_arg_fin;
	}
	
	if (current->data[0] == CH_DELIMITER)
	{
		if (arg) arg->type = KEST_ASM_ARG_CHANNEL;
		
		valid = 1;
		n = 0;
		for (int i = 1; current->data[i] != 0; i++)
		{
			if (i > 2)
				valid = 0;
			
			if ('0' <= current->data[i] && current->data[i] <= '9')
			{
				n = 10 * n + current->data[i] - '0';
			}
			else if ('a' <= current->data[i] && current->data[i] <= 'f')
			{
				n = 16 * n + current->data[i] - 'a' + 10;
			}
			else if ('A' <= current->data[i] && current->data[i] <= 'F')
			{
				n = 16 * n + current->data[i] - 'A' + 10;
			}
			else
			{
				valid = 0;
			}
			
			if (!valid)
			{
				kest_parser_error_at(ps, current, "Invalid argument \"%s\"");
				ret_val = ERR_BAD_ARGS;
				goto asm_parse_arg_fin;
			}
		}
		
		if (arg) arg->addr = n;
	}
	else if (strcmp(current->data, KEST_EXPR_DELIMITER) == 0)
	{
		if (arg) arg->type = KEST_ASM_ARG_EXPR;
		
		tok = current;
		
		do 
		{
			tok = tok->next;
			
			if (!tok || strcmp(tok->data, "\n") == 0)
			{
				kest_parser_error_at(ps, current, "Missing \"%s\"", DQ_CODELIMITER);
				ret_val = ERR_BAD_ARGS;
				goto asm_parse_arg_fin;
			}
			
		} while (strcmp(tok->data, DQ_CODELIMITER) != 0);
		
		if (arg)
		{
			arg->expr = kest_parse_expression(ps, current->next, tok);
			if (!arg->expr)
			{
				ret_val = ERR_BAD_ARGS;
				goto asm_parse_arg_fin;
			}
		}
		
		if (tok)
			current = tok;
		else
			current = NULL;
	}
	else if (strcmp(current->data, RS_DELIMITER) == 0)
	{
		if (arg) arg->type = KEST_ASM_ARG_RES;
		
		current = current->next;
		
		if (!current || !current->data || strcmp(current->data, "\n") == 0)
		{
			kest_parser_error_at(ps, current, "Missing resource identifier");
			ret_val = ERR_BAD_ARGS;
			goto asm_parse_arg_fin;
		}
		
		current_res = ps->resources;
		
		resource_found = 0;
		while (current_res && !resource_found)
		{
			if (current_res->data && current_res->data->name)
			{
				if (strcmp(current->data, current_res->data->name) == 0)
				{
					resource_found = 1;
					if (arg) arg->res = current_res->data;
				}
			}
			
			current_res = current_res->next;
		}
		
		if (resource_found && arg)
		{
			
		}
		else
		{
			kest_parser_error_at(ps, current, "Resource \"%s\" not found", current->data);
			ret_val = ERR_BAD_ARGS;
			goto asm_parse_arg_fin;
		}
	}
	else if (token_is_int(current->data))
	{
		if (arg)
		{
			arg->type = KEST_ASM_ARG_INT;
			arg->val = strtol(current->data, NULL, 10);
		}
	}
	else 
	{
		KEST_PRINTF("Syntax error: \"%s\"\n", current->data);
		ret_val = ERR_BAD_ARGS;
		goto asm_parse_arg_fin;
	}
	
asm_parse_arg_fin:

	if (current)
		current = current->next;

	ps->current_token = current;
	
	return ret_val;
}

int kest_parse_asm_arg_2(kest_eff_parsing_state *ps, kest_asm_arg *arg)
{
	if (!ps)
		return ERR_NULL_PTR;
	
	kest_token_ll *current = ps->current_token;
	kest_token_ll *tok;
	
	kest_expression *expr;
	
	kest_dsp_resource_pll *current_res;
	int resource_found;
	int line;
	
	int ret_val = NO_ERROR;
	
	int n;
	int valid;
	
	if (!current || !current->data)
	{
		KEST_PRINTF("Error: current = %p, current->data = %p\n", current, current ? current->data : NULL);
		ret_val = ERR_BAD_ARGS;
		goto asm_parse_arg_fin;
	}
	
	if (current->data[0] == CH_DELIMITER)
	{
		if (arg) arg->type = KEST_ASM_ARG_CHANNEL;
		
		valid = 1;
		n = 0;
		for (int i = 1; current->data[i] != 0; i++)
		{
			if (i > 2)
				valid = 0;
			
			if ('0' <= current->data[i] && current->data[i] <= '9')
			{
				n = 10 * n + current->data[i] - '0';
			}
			else if ('a' <= current->data[i] && current->data[i] <= 'f')
			{
				n = 16 * n + current->data[i] - 'a' + 10;
			}
			else if ('A' <= current->data[i] && current->data[i] <= 'F')
			{
				n = 16 * n + current->data[i] - 'A' + 10;
			}
			else
			{
				valid = 0;
			}
			
			if (!valid)
			{
				kest_parser_error_at(ps, current, "Invalid argument \"%s\"");
				ret_val = ERR_BAD_ARGS;
				goto asm_parse_arg_fin;
			}
		}
		
		if (arg)
		{
			arg->expr = kest_expr_new_const(n);
		}
	}
	else if (strcmp(current->data, KEST_EXPR_DELIMITER) == 0)
	{
		if (arg) arg->type = KEST_ASM_ARG_EXPR;
		
		tok = current;
		
		do 
		{
			tok = tok->next;
			
			if (!tok || strcmp(tok->data, "\n") == 0)
			{
				kest_parser_error_at(ps, current, "Missing \"%s\"", DQ_CODELIMITER);
				ret_val = ERR_BAD_ARGS;
				goto asm_parse_arg_fin;
			}
			
		} while (strcmp(tok->data, DQ_CODELIMITER) != 0);
		
		if (arg)
		{
			arg->expr = kest_parse_expression(ps, current->next, tok);
			if (!arg->expr)
			{
				kest_parser_error_at(ps, current, "Error parsing expression \"%s\"", kest_expression_to_string(arg->expr));
				ret_val = ERR_BAD_ARGS;
				goto asm_parse_arg_fin;
			}
		}
		
		if (tok)
			current = tok;
		else
			current = NULL;
	}
	else if (strcmp(current->data, RS_DELIMITER) == 0)
	{
		if (arg) arg->type = KEST_ASM_ARG_RES;
		
		current = current->next;
		
		if (!current || !current->data || strcmp(current->data, "\n") == 0)
		{
			kest_parser_error_at(ps, current, "Missing resource identifier");
			ret_val = ERR_BAD_ARGS;
			goto asm_parse_arg_fin;
		}
		
		expr = kest_expr_new_reference(current->data);
		if (arg)
		{
			arg->expr = expr;
		}
	}
	else if (token_is_int(current->data))
	{
		if (arg)
		{
			arg->type = KEST_ASM_ARG_INT;
			arg->expr = kest_expr_new_const(token_to_float(current->data));
		}
	}
	else 
	{
		kest_parser_error_at(ps, current, "Syntax error");
		ret_val = ERR_BAD_ARGS;
		goto asm_parse_arg_fin;
	}
	
asm_parse_arg_fin:

	if (current)
		current = current->next;

	ps->current_token = current;
	
	return ret_val;
}

int is_valid_instr(const char *instr)
{
	if (!instr)
		return 0;
	
	for (int i = 0; i < n_instrs; i++)
		if (strcmp(instrs[i], instr) == 0) return 1;
	
	return 0;
}

int kest_parse_asm_line(kest_eff_parsing_state *ps)
{
	if (!ps)
		return ERR_NULL_PTR;
	
	kest_token_ll *current = ps->current_token;
	
	if (!current)
		return ERR_BAD_ARGS;
	
	if (!current->data)
		return ERR_BAD_ARGS;
	
	kest_block *block = kest_alloc(sizeof(kest_block));
	
	if (!block)
		return ERR_ALLOC_FAIL;
	
	memset(block, 0, sizeof(kest_block));
	
	int line_number = current->line;
	
	KEST_PRINTF("Parsing asm line, line %d: \"%s\"\n", line_number, ps->lines[line_number - 1]);
	
	int ret_val = NO_ERROR;
	
	const char *instr_char = current->data;
	
	kest_asm_line *line = NULL;
	kest_asm_operand args[INSTR_MAX_ARGS];
	
	int args_cont = 1;
	int line_fin = 0;
	int n_args_read = 0;
	int arg_ret_val;
	
	if (!is_valid_instr(current->data))
	{
		kest_parser_error_at(ps, current, "Unknown instruction \"%s\"", current->data);
		ret_val = ERR_BAD_ARGS;
		goto asm_line_parse_fin;
	}
	
	line = kest_parser_alloc(sizeof(kest_asm_line));
	
	if (!line)
	{
		kest_parser_error_at(ps, current, "Allocation failed", current->data);
		ret_val = ERR_ALLOC_FAIL;
		goto asm_line_parse_fin;
	}
	
	line->line_number = line_number;
	line->n_args = 0;
	line->instr = kest_parser_strndup(current->data, 12);
	
	if (!line->instr)
	{
		kest_parser_error_at(ps, current, "Allocation failed", current->data);
		ret_val = ERR_ALLOC_FAIL;
		goto asm_line_parse_fin;
	}
	
	current = current->next;
	ps->current_token = current;
	
	for (int i = 0; ; i++)
	{
		if (!current || !current->data || current->data[0] == '\n' || current->line != line_number)
		{
			KEST_PRINTF("Breaking argument loop; reason:\n");
			if (!current)
				KEST_PRINTF("current = NULL");
			if (!current->data)
				KEST_PRINTF("current->data = NULL");
			if (current->data[0] == '\n')
				KEST_PRINTF("current token is newline token\n");
			if (current->line != line_number)
				KEST_PRINTF("current token is on line %d, instead of %d\n", current->line, line_number);
			break;
		}
		
		if ((arg_ret_val = kest_parse_asm_arg_2(ps, (i < INSTR_MAX_ARGS) ? &line->args[i] : NULL)) != NO_ERROR)
		{
			current = ps->current_token;
			ret_val = arg_ret_val;
			kest_parser_error_at_line(ps, line_number, "Argument %d invalid", i);
			goto asm_line_parse_fin;
		}
		
		if (i < INSTR_MAX_ARGS)
		{
			KEST_PRINTF("Parsed an argument: ");
			switch (line->args[i].type)
			{
				case KEST_ASM_ARG_CHANNEL:
					KEST_PRINTF("channel c%s\n", kest_expression_to_string(line->args[i].expr));
					break;
					
				case KEST_ASM_ARG_EXPR:
					KEST_PRINTF("expression [%s]\n", kest_expression_to_string(line->args[i].expr));
					break;
				
				case KEST_ASM_ARG_RES:
					KEST_PRINTF("resource $%s\n", kest_expression_to_string(line->args[i].expr));
					break;
					
				case KEST_ASM_ARG_INT:
					KEST_PRINTF("integer: %s\n", kest_expression_to_string(line->args[i].expr));
					break;
			}
		}
		
		line->n_args++;
		n_args_read++;
		
		current = ps->current_token;
	}
	
	KEST_PRINTF("Line has %d args\n", line->n_args);
	
	kest_asm_line_pll_safe_append(&ps->asm_lines, line);

asm_line_parse_fin:
	
	ps->current_token = current;
	
	return ret_val;
}

int kest_parse_asm(kest_eff_parsing_state *ps)
{
	if (!ps)
		return ERR_NULL_PTR;
	
	kest_token_ll *current = ps->current_token;
	
	int line_start;
	int comment;
	int ret_val = NO_ERROR;
	
	kest_token_ll_skip_ws(&current);
	while (current)
	{
		// Advance to the next non-whitespace token
		ps->current_token = current;
		ret_val = kest_parse_asm_line(ps);
		
		if (ret_val != NO_ERROR)
		{
			kest_parser_error_at(ps, ps->current_token, "ASM parsing failed: %s", kest_error_code_to_string(ret_val));
			return ret_val;
		}
		
		current = ps->current_token;
		kest_token_ll_skip_ws(&current);
	}
	
	return NO_ERROR;
}

kest_dsp_resource *kest_resource_get_by_name(kest_dsp_resource_pll *resources, const char *name)
{
	if (!resources || !name)
		return NULL;
	
	kest_dsp_resource_pll *current = resources;
	
	while (current)
	{
		if (current->data && current->data->name && strcmp(current->data->name, name) == 0)
			return current->data;
		
		current = current->next;
	}
	
	return NULL;
}

int kest_process_asm_line(kest_eff_parsing_state *ps, kest_asm_line *line)
{
	if (!ps || !line)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	int line_number = line->line_number;
	
	kest_asm_instr_desc *desc = kest_instr_name_to_desc(line->instr);
	
	KEST_PRINTF_("Processing instruction %s online %d\n", line->instr ? line->instr : "(NULL)", line_number);
	
	if (!desc)
	{
		kest_parser_error_at_line(ps, line_number, "Instruction \"%s\" does not exist", line->instr ? line->instr : "(NULL)");
		return ERR_BAD_ARGS;
	}
	
	KEST_PRINTF_("Obtained instruction descriptor. Name: \"%s\", opcode: %d\n", desc->name, desc->opcode);
	
	kest_block *block = NULL;
	const kest_instr_arg_fmt *arg_format = &desc->arg_fmt;
	
	kest_dsp_resource *resource;
	
	if (line->n_args != arg_format->n_args)
	{
		kest_parser_error_at_line(ps, line_number,
			"Instruction \"%s\" expects %d arguments, but %d were given.", line->instr, arg_format->n_args, line->n_args);
		return ERR_BAD_ARGS;
	}
	
	block = kest_alloc(sizeof(kest_block));
	
	if (!block)
		return ERR_ALLOC_FAIL;
	
	kest_init_block_from_instr_desc(block, desc);
	
	kest_asm_arg arg;
	kest_block_operand *op = NULL;
	
	int reg_0_taken = 0;
	
	for (int i = 0; i < line->n_args; i++)
	{
		arg = line->args[i];
		
		if (i == arg_format->dest_pos)
		{
			if (arg.type != KEST_ASM_ARG_CHANNEL)
			{
				kest_parser_error_at_line(ps, line_number, "Destination must be a channel");
				return ERR_BAD_ARGS;
			}
			
			KEST_PRINTF("Evaluating destination; \"%s\"\n", kest_expression_to_string(line->args[i].expr));
			block->dest = (int)roundf(kest_expression_evaluate(line->args[i].expr, NULL));
			KEST_PRINTF("Result: c%d\n", block->dest);
		}
		else if (i == arg_format->res_pos)
		{
			if (arg.type != KEST_ASM_ARG_RES)
			{
				kest_parser_error_at_line(ps, line_number, "Argument %d of instruction \"%s\" must be a resource", i + 1, line->instr);
				return ERR_BAD_ARGS;
			}
			
			if (!arg.expr)
			{
				return ERR_UNKNOWN_ERR;
			}
			
			if (arg.expr->type != KEST_EXPR_REF)
			{
				// tbh.... 
				return ERR_UNKNOWN_ERR;
			}
			
			if (!arg.expr->val.ref_name)
			{
				kest_parser_error_at_line(ps, line_number, "Resource has no name!");
				return ERR_UNKNOWN_ERR;
			}
			
			resource = kest_resource_get_by_name(ps->resources, arg.expr->val.ref_name);
			
			if (!resource)
			{
				kest_parser_error_at_line(ps, line_number, "Could not find resource \"%s\"", arg.expr->val.ref_name);
				return ERR_BAD_ARGS;
			}
			
			block->res = resource;
		}
		else if (i == arg_format->shift_pos)
		{
			if (arg.type != KEST_ASM_ARG_INT)
			{
				kest_parser_error_at_line(ps, line_number, "Argument %d of instruction \"%s\" must be an integer", i + 1, line->instr);
				return ERR_BAD_ARGS;
			}
			
			if (!arg.expr)
			{
				return ERR_UNKNOWN_ERR;
			}
			
			if (arg.expr->type != KEST_EXPR_CONST)
			{
				// tbh.... 
				return ERR_UNKNOWN_ERR;
			}
			
			block->shift = (int)roundf(kest_expression_evaluate(arg.expr, NULL));
		}
		else
		{
			if (i == arg_format->arg_a_pos)
				op = &block->arg_a;
			else if (i == arg_format->arg_b_pos)
				op = &block->arg_b;
			else if (i == arg_format->arg_c_pos)
				op = &block->arg_c;
			
			if (!op)
			{
				kest_parser_error_at_line(ps, line_number, "Argument %d of instruction \"%s\" does not correspond to anything (this is a bug!!)", i + 1, line->instr);
				return ERR_UNKNOWN_ERR;
			}
			
			switch (arg.type)
			{
				case KEST_ASM_ARG_CHANNEL:
					op->type = BLOCK_OPERAND_TYPE_C;
					op->addr = (int)roundf(kest_expression_evaluate(arg.expr, NULL));
					break;
				
				case KEST_ASM_ARG_EXPR:
					op->type = BLOCK_OPERAND_TYPE_R;
					
					if (!reg_0_taken)
					{
						block->reg_0.active = 1;
						block->reg_0.expr = arg.expr;
						reg_0_taken = 1;
						
						op->addr = 0;
					}
					else 
					{
						block->reg_1.active = 1;
						block->reg_1.expr = arg.expr;
						
						op->addr = 1;
					}
					
					break;
				
				default:
					kest_parser_error_at_line(ps, line_number, "Argument %d of instruction \"%s\" must be either a channel or an expression", i + 1, line->instr);
					return ERR_BAD_ARGS;
			}
		}
	}
	
	/* ... and some special considerations for unusual instructions */
	
	if (strcmp(line->instr, "mov") == 0)
	{
		block->arg_b = operand_const_one();
		block->arg_c = operand_const_zero();
		block->shift = 1;
		block->shift_set = 1;
	}
	else if (strcmp(line->instr, "add") == 0)
	{
		block->arg_b = operand_const_one();
		block->shift = 1;
		block->shift_set = 1;
	}
	else if (strcmp(line->instr, "sub") == 0)
	{
		block->arg_b = operand_const_minus_one();
	}
	else if (strcmp(line->instr, "mul") == 0)
	{
		block->arg_c = operand_const_zero();
	}
	else if (strcmp(line->instr, "delay_read") == 0)
	{
		block->arg_a = operand_const_zero();
	}
	else if (strcmp(line->instr, "delay_write") == 0)
	{
		block->arg_b = operand_const_zero();
	}
	else if (strcmp(line->instr, "sin2pi") == 0)
	{
		block->res = &sin_lut;
	}
	else if (strcmp(line->instr, "tanh4") == 0)
	{
		block->res = &tanh_lut;
	}
	
	kest_block_pll_safe_append(&ps->blocks, block);
	
	return ret_val;
}

int kest_process_asm_lines(kest_eff_parsing_state *ps)
{
	KEST_PRINTF("kest_process_asm_lines\n");
	if (!ps)
		return ERR_NULL_PTR;
	
	int ret_val = NO_ERROR;
	
	kest_asm_line_pll *current = ps->asm_lines;
	
	int i = 0;
	while (current)
	{
		KEST_PRINTF("kest_process_asm_lines, line %d\n", i);
		if ((ret_val = kest_process_asm_line(ps, current->data)) != NO_ERROR)
		{
			if (current->data)
				kest_parser_error_at_line(ps, current->data->line_number, "%s", kest_error_code_to_string(ret_val));
			else
				kest_parser_error(ps, "%s", kest_error_code_to_string(ret_val));
			return ret_val;
		}
		current = current->next; i++;
	}
	
	return ret_val;
}
