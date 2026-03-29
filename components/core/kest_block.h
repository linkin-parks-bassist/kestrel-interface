#ifndef BLOCK_H_
#define BLOCK_H_

#define BLOCK_INSTR_NOP 			0
#define BLOCK_INSTR_MADD			1
#define BLOCK_INSTR_ARSH 			2
#define BLOCK_INSTR_LSH 			3
#define BLOCK_INSTR_RSH 			4
#define BLOCK_INSTR_ABS				5
#define BLOCK_INSTR_MIN			    6
#define BLOCK_INSTR_MAX			    7
#define BLOCK_INSTR_CLAMP		    8
#define BLOCK_INSTR_MOV_ACC			9
#define BLOCK_INSTR_MOV_LACC		10
#define BLOCK_INSTR_MOV_UACC		11
#define BLOCK_INSTR_MACZ			12
#define BLOCK_INSTR_UMACZ			13
#define BLOCK_INSTR_MAC				14
#define BLOCK_INSTR_UMAC			15
#define BLOCK_INSTR_LUT_READ		16
#define BLOCK_INSTR_DELAY_READ 		17
#define BLOCK_INSTR_DELAY_WRITE 	18
#define BLOCK_INSTR_MEM_READ 		19
#define BLOCK_INSTR_MEM_WRITE		20
#define BLOCK_INSTR_FILTER			21
#define BLOCK_INSTR_FCASC			22

struct kest_dsp_resource;

#define BLOCK_OPERAND_TYPE_C 0
#define BLOCK_OPERAND_TYPE_R 1

typedef struct {
	int type;
	int addr;
} kest_block_operand;

#define ZERO_REGISTER_ADDR 		2
#define POS_ONE_REGISTER_ADDR  	3
#define NEG_ONE_REGISTER_ADDR  	4

kest_block_operand operand_const_zero();
kest_block_operand operand_const_one();
kest_block_operand operand_const_minus_one();

typedef struct
{
	int format;
	int active;
	kest_expression *expr;
} kest_block_reg_val;

typedef struct {
	int instr;
	
	kest_block_operand arg_a;
	kest_block_operand arg_b;
	kest_block_operand arg_c;
	
	int dest;
	
	kest_block_reg_val reg_0;
	kest_block_reg_val reg_1;
	
	int shift;
	int shift_set;
	int saturate_disable;
	
	kest_dsp_resource *res;
} kest_block;

int kest_init_block(kest_block *block);

DECLARE_LINKED_PTR_LIST(kest_block);

#endif
