#ifndef KEST_DEPENDER_H_
#define KEST_DEPENDER_H_

#define KEST_DEPENDENT_NONE			0
#define KEST_DEPENDENT_SCOPE_ENTRY 	1
#define KEST_DEPENDENT_BLOCK_REG	2
#define KEST_DEPENDENT_FILTER_COEF	3

typedef struct {
	int type;
	
	union {
		const char *entry_key;
		
		struct {
			int reg;
			int block;
		} block_reg;
		
		struct {
			int filter;
			int coef;
		} filter_coef;
	} data;
} kest_dependent;

DECLARE_LIST(kest_dependent);

kest_dependent kest_dependent_scope_entry(const char *key);
kest_dependent kest_dependent_block_reg(int block, int reg);
kest_dependent kest_dependent_filter_coef(int block, int reg);

int kest_string_append_dependent(kest_string *str, kest_dependent dep);

int kest_dependent_is_updatable(int type);

#endif
