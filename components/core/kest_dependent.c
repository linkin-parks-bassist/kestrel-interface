#include "kest_int.h"

static const char *FNAME = "kest_dependent.c";
#define PRINTLINES_ALLOWED 1

IMPLEMENT_LIST(kest_dependent);

kest_dependent kest_dependent_scope_entry(struct kest_scope_entry *entry)
{
	kest_dependent dep = {0};
	
	if (!entry)
		return dep;
	
	dep.type = KEST_DEPENDENT_SCOPE_ENTRY;
	dep.data.scope_entry = entry;
	return dep;
}

kest_dependent kest_dependent_block_reg(int block, int reg)
{
	kest_dependent dep = {0};
	dep.type = KEST_DEPENDENT_BLOCK_REG;
	dep.data.block_reg.block = block;
	dep.data.block_reg.reg = reg;
	return dep;
}

kest_dependent kest_dependent_filter_coef(int filter, int coef)
{
	kest_dependent dep = {0};
	dep.type = KEST_DEPENDENT_FILTER_COEF;
	dep.data.filter_coef.filter = filter;
	dep.data.filter_coef.coef = coef;
	return dep;
}

int kest_string_append_dependent(kest_string *str, kest_dependent dep)
{
	if (!str)
		return ERR_NULL_PTR;
	
	switch (dep.type)
	{
		case KEST_DEPENDENT_SCOPE_ENTRY:
			return kest_string_appendf(str, "scope entry %p\n", dep.data.scope_entry);
			break;
		case KEST_DEPENDENT_BLOCK_REG:
			return kest_string_appendf(str, "block %d reg %d\n", dep.data.block_reg.block, dep.data.block_reg.reg);
			break;
		case KEST_DEPENDENT_FILTER_COEF:
			return kest_string_appendf(str, "filter %d coef %d\n", dep.data.filter_coef.filter, dep.data.filter_coef.coef);
			break;
		default:
			return kest_string_appendf(str, "unknown");
	}
	
	return NO_ERROR;
}
