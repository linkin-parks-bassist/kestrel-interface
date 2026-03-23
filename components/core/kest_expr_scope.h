#ifndef KEST_EXPR_SCOPE_H_
#define KEST_EXPR_SCOPE_H_

#define KEST_SCOPE_ENTRY_TYPE_EXPR 		0
#define KEST_SCOPE_ENTRY_TYPE_PARAM 	1
#define KEST_SCOPE_ENTRY_TYPE_SETTING 	2

struct kest_expression;
struct kest_parameter;
struct kest_setting;

typedef struct {
	const char *name;
	int type;
	union {
		struct kest_expression *expr;
		struct kest_parameter *param;
		struct kest_setting *setting;
	} val;
} kest_expr_scope_entry;

kest_expr_scope_entry *kest_new_expr_scope_entry_expr(const char *name, struct kest_expression *expr);
kest_expr_scope_entry *kest_new_expr_scope_entry_param(struct kest_parameter *param);
kest_expr_scope_entry *kest_new_expr_scope_entry_setting(struct kest_setting *setting);

DECLARE_LINKED_PTR_LIST(kest_expr_scope_entry);

// like, really this should be a hash table but, i honestly 
// don't expect them to get particularly large so im
// just going to use my default, beloved linked list
typedef struct {
	kest_expr_scope_entry_pll *entries;
} kest_expr_scope;

kest_expr_scope *kest_new_expr_scope();
int kest_expr_scope_init(kest_expr_scope *scope);
int kest_expr_scope_add_expr(kest_expr_scope *scope, const char *name, struct kest_expression *expr);
int kest_expr_scope_add_param(kest_expr_scope *scope, struct kest_parameter *param);
int kest_expr_scope_add_setting(kest_expr_scope *scope,struct kest_setting *setting);

struct kest_parameter_pll;
struct kest_setting_pll;

int kest_expr_scope_add_params(kest_expr_scope *scope, struct kest_parameter_pll *params);
int kest_expr_scope_add_settings(kest_expr_scope *scope, struct kest_setting_pll *settings);

kest_expr_scope_entry *kest_expr_scope_fetch(kest_expr_scope *scope, const char *name);

kest_expr_scope *kest_expr_scope_copy(kest_expr_scope *scope);

#endif
