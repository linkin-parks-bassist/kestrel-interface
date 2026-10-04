#ifndef KEST_FPGA_INSTR_H_
#define KEST_FPGA_INSTR_H_

#include "kest_numeric_format.h"

struct kest_block;
struct kest_expression;

/* Bit n permits a format with n fractional bits. */
typedef struct {
	uint32_t signed_formats;
	uint32_t unsigned_formats;
	kest_numeric_overflow overflow;
	struct kest_expression *(*send_expression)(struct kest_expression *value);
} kest_arg_numeric_policy;

typedef struct {
	kest_arg_numeric_policy args[3];
	int (*resolve_field)(const struct kest_block *block,
		const kest_numeric_format formats[3], int *field);
} kest_instr_numeric_policy;

#define KEST_ARG_POS_NONE -1


typedef struct {
	int n_args;
	int arg_a_pos;
	int arg_b_pos;
	int arg_c_pos;
	int dest_pos;
	int res_pos;
	int shift_pos;
} kest_instr_arg_fmt;

typedef struct kest_asm_instr_desc {
	int opcode;
	char *name;
	
	const kest_instr_numeric_policy *numeric;
	
	kest_instr_arg_fmt arg_fmt;
} kest_asm_instr_desc;

const kest_asm_instr_desc *kest_instr_name_to_desc(const char *name);
const kest_asm_instr_desc *kest_instr_opcode_to_desc(int opcode);

void kest_instr_print(kest_string *str, uint32_t instr);

#endif
