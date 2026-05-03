#ifndef KEST_EXPR_SCOPE_H_
#define KEST_EXPR_SCOPE_H_

#define KEST_SCOPE_ENTRY_TYPE_EXPR 		0
#define KEST_SCOPE_ENTRY_TYPE_PARAM 	1
#define KEST_SCOPE_ENTRY_TYPE_SETTING 	2
#define KEST_SCOPE_ENTRY_TYPE_MEM	 	3

struct kest_mem_slot;
struct kest_expression;
struct kest_parameter;
struct kest_setting;

typedef struct kest_scope_entry {
	int type;
	union {
		struct kest_mem_slot 	*mem;
		struct kest_expression *expr;
		struct kest_parameter *param;
		struct kest_setting *setting;
	} val;
	
	int updated;
} kest_scope_entry;

DECLARE_DICT(kest_scope_entry);

typedef struct kest_scope {
	kest_scope_entry_dict dict;
	size_t count;
} kest_scope;

kest_scope *kest_scope_new();
int kest_scope_init(kest_scope *scope);

int kest_scope_entry_init_mem(kest_scope_entry *entry, struct kest_mem_slot *mem);
int kest_scope_entry_init_expr(kest_scope_entry *entry, struct kest_expression *expr);
int kest_scope_entry_init_param(kest_scope_entry *entry, struct kest_parameter *param);
int kest_scope_entry_init_setting(kest_scope_entry *entry, struct kest_setting *setting);

int kest_scope_add_mem(kest_scope *scope, const char *name, struct kest_mem_slot *mem);
int kest_scope_add_expr(kest_scope *scope, const char *name, struct kest_expression *expr);
int kest_scope_add_param(kest_scope *scope, struct kest_parameter *param);
int kest_scope_add_setting(kest_scope *scope, struct kest_setting *setting);

kest_scope_entry *kest_scope_add_mem_return_entry(kest_scope *scope, const char *name, struct kest_mem_slot *mem);
kest_scope_entry *kest_scope_add_expr_return_entry(kest_scope *scope, const char *name, struct kest_expression *expr);
kest_scope_entry *kest_scope_add_param_return_entry(kest_scope *scope, struct kest_parameter *param);
kest_scope_entry *kest_scope_add_setting_return_entry(kest_scope *scope, struct kest_setting *setting);

int kest_scope_propagate_updates(kest_scope *scope);
int kest_scope_clear_updates(kest_scope *scope);

int kest_scope_add_params(kest_scope *scope, struct kest_parameter_pll *params);
int kest_scope_add_settings(kest_scope *scope, struct kest_setting_pll *settings);

int kest_scope_entry_eval(kest_scope_entry *entry, kest_scope *scope, float *dest);
int kest_scope_entry_eval_rec(kest_scope_entry *entry, kest_scope *scope, float *dest, int depth);

kest_scope_entry *kest_scope_lookup(kest_scope *scope, const char *name);
kest_scope_entry *kest_scope_fetch(kest_scope *scope, const char *name);

size_t kest_scope_count(kest_scope *scope);
kest_scope_entry *kest_scope_index(kest_scope *scope, size_t n);
const char *kest_scope_index_key(kest_scope *scope, size_t n);

#endif
