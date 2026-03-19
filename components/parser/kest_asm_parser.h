#ifndef ASM_H_
#define ASM_H_

#define KEST_ASM_ARG_CHANNEL 	0
#define KEST_ASM_ARG_EXPR 		1
#define KEST_ASM_ARG_RES  		2
#define KEST_ASM_ARG_INT  		3

#define INSTR_MAX_ARGS 4

typedef struct {
	int type;
	struct kest_expression *expr;
} kest_asm_arg;

#define KEST_ARG_POS_NONE -1

typedef struct {
	int n_args;
	int arg_a_pos;
	int arg_b_pos;
	int arg_c_pos;
	int dest_pos;
	int res_pos;
	int shift_pos;
} kest_arg_format;

typedef struct {
	int type;
	int addr;
	int val;
	struct kest_expression *expr;
	kest_dsp_resource *res;
	const char *res_name;
} kest_asm_operand;

typedef struct {
	int instr;
	
	int n_args;
	kest_asm_operand args[INSTR_MAX_ARGS];
} kest_asm_instr;

typedef struct {
	const char *instr;
	kest_asm_arg args[INSTR_MAX_ARGS];
	int line_number;
	int n_args;
} kest_asm_line;

DECLARE_LINKED_PTR_LIST(kest_asm_line);

int kest_parse_asm(kest_eff_parsing_state *ps);
int kest_process_asm_lines(kest_eff_parsing_state *ps);

#endif
