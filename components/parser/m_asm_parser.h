#ifndef ASM_H_
#define ASM_H_

#define M_ASM_ARG_CHANNEL 	0
#define M_ASM_ARG_EXPR 		1
#define M_ASM_ARG_RES  		2
#define M_ASM_ARG_INT  		3

#define INSTR_MAX_ARGS 4

typedef struct {
	int type;
	struct m_expression *expr;
} m_asm_arg;

#define M_ARG_POS_NONE -1

typedef struct {
	int n_args;
	int arg_a_pos;
	int arg_b_pos;
	int arg_c_pos;
	int dest_pos;
	int res_pos;
	int shift_pos;
} m_arg_format;

typedef struct {
	int type;
	int addr;
	int val;
	struct m_expression *expr;
	m_dsp_resource *res;
	const char *res_name;
} m_asm_operand;

typedef struct {
	int instr;
	
	int n_args;
	m_asm_operand args[INSTR_MAX_ARGS];
} m_asm_instr;

typedef struct {
	const char *instr;
	m_asm_arg args[INSTR_MAX_ARGS];
	int line_number;
	int n_args;
} m_asm_line;

DECLARE_LINKED_PTR_LIST(m_asm_line);

int m_parse_asm(m_eff_parsing_state *ps);
int m_process_asm_lines(m_eff_parsing_state *ps);

#endif
