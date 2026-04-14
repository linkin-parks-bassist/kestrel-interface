#ifndef KEST_FPGA_INSTR_H_
#define KEST_FPGA_INSTR_H_

#define KEST_ARG_POS_NONE -1

#define SHIFT_POLICY_0 		0
#define SHIFT_POLICY_1		1
#define SHIFT_POLICY_SFAB 	2
#define SHIFT_POLICY_MFAB	3
#define SHIFT_POLICY_FA		4
#define SHIFT_POLICY_FB		5
#define SHIFT_POLICY_FC		6
#define SHIFT_POLICY_SET	7

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
	
	int shift_policy;
	
	kest_instr_arg_fmt arg_fmt;
} kest_asm_instr_desc;

const kest_asm_instr_desc *kest_instr_name_to_desc(char *name);
const kest_asm_instr_desc *kest_instr_opcode_to_desc(int opcode);

void kest_instr_print(kest_string *str, uint32_t instr);

#endif
