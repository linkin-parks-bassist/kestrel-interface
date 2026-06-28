#include "kest_int.h"

static const char *FNAME = "kest_dependent.c";
#define PRINTLINES_ALLOWED 0

IMPLEMENT_LIST(kest_dependent);

kest_dependent kest_dependent_scope_entry(const char *key)
{
	kest_dependent dep = {0};
	
	if (!key)
		return dep;
	
	dep.type = KEST_DEPENDENT_SCOPE_ENTRY;
	dep.data.entry_key = key;
	
	return dep;
}

kest_dependent kest_dependent_bound_parameter(kest_parameter *param)
{
	kest_dependent dep = {0};
	dep.type = KEST_DEPENDENT_BOUND_PARAMETER;
	dep.data.param = param;
	return dep;
}

kest_dependent kest_dependent_driven_parameter(kest_parameter *param)
{
	kest_dependent dep = {0};
	dep.type = KEST_DEPENDENT_DRIVEN_PARAMETER;
	dep.data.param = param;
	return dep;
}

kest_dependent kest_dependent_block_reg(int block, int reg, int format)
{
	kest_dependent dep = {0};
	dep.type = KEST_DEPENDENT_BLOCK_REG;
	dep.data.block_reg.block = block;
	dep.data.block_reg.reg = reg;
	dep.format = format;
	return dep;
}

kest_dependent kest_dependent_filter_coef(int filter, int coef, int format)
{
	kest_dependent dep = {0};
	dep.type = KEST_DEPENDENT_FILTER_COEF;
	dep.data.filter_coef.filter = filter;
	dep.data.filter_coef.coef = coef;
	dep.format = format;
	return dep;
}

int kest_string_append_dependent(kest_string *str, kest_dependent dep)
{
	if (!str)
		return ERR_NULL_PTR;
	
	switch (dep.type)
	{
		case KEST_DEPENDENT_SCOPE_ENTRY:
			return kest_string_appendf(str, "Scope entry \"%s\"", dep.data.entry_key);
		case KEST_DEPENDENT_BLOCK_REG:
			return kest_string_appendf(str, "block %d reg %d", dep.data.block_reg.block, dep.data.block_reg.reg);
		case KEST_DEPENDENT_FILTER_COEF:
			return kest_string_appendf(str, "filter %d coef %d", dep.data.filter_coef.filter, dep.data.filter_coef.coef);
		case KEST_DEPENDENT_BOUND_PARAMETER:
			return kest_string_appendf(str, "bounded parameter \"%s\"", dep.data.param ? dep.data.param->name_internal : "(NULL)");
		case KEST_DEPENDENT_DRIVEN_PARAMETER:
			return kest_string_appendf(str, "driven parameter \"%s\"", dep.data.param ? dep.data.param->name_internal : "(NULL)");
		default:
			return kest_string_appendf(str, "unknown");
	}
	
	return NO_ERROR;
}

int kest_dependent_is_updatable(int type)
{
	switch (type)
	{
		case KEST_DEPENDENT_BLOCK_REG:
		case KEST_DEPENDENT_FILTER_COEF:
		case KEST_DEPENDENT_DRIVEN_PARAMETER:
		case KEST_DEPENDENT_BOUND_PARAMETER:
			return 1;
	}
	return 0;
}
