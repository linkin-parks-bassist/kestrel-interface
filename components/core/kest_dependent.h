#ifndef KEST_DEPENDER_H_
#define KEST_DEPENDER_H_

#define KEST_DEPENDENT_NONE				0
#define KEST_DEPENDENT_SCOPE_ENTRY 		1
#define KEST_DEPENDENT_BLOCK_REG		2
#define KEST_DEPENDENT_FILTER_COEF		3
#define KEST_DEPENDENT_DRIVEN_PARAMETER	4
#define KEST_DEPENDENT_BOUND_PARAMETER 	5

struct kest_parameter;

typedef struct {
	int type;
	
	union {
		const char *entry_key;
		struct kest_parameter *param;
		
		struct {
			int reg;
			int block;
		} block_reg;
		
		struct {
			int filter;
			int coef;
		} filter_coef;
	} data;
	
	int format;
} kest_dependent;

DECLARE_LIST(kest_dependent);

kest_dependent kest_dependent_scope_entry(const char *key);
kest_dependent kest_dependent_bound_parameter(struct kest_parameter *param);
kest_dependent kest_dependent_driven_parameter(struct kest_parameter *param);
kest_dependent kest_dependent_block_reg(int block, int reg, int format);
kest_dependent kest_dependent_filter_coef(int block, int reg, int format);

int kest_string_append_dependent(kest_string *str, kest_dependent dep);

int kest_dependent_is_updatable(int type);

#endif
