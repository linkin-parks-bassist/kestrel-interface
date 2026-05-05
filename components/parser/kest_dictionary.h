#ifndef KEST_DICTIONARY_H_
#define KEST_DICTIONARY_H_

#define DICT_ENTRY_TYPE_NOT_FOUND 	22222
#define DICT_ENTRY_TYPE_NOTHING 	22221
#define DICT_ENTRY_TYPE_INT 		0
#define DICT_ENTRY_TYPE_FLOAT 		1
#define DICT_ENTRY_TYPE_STR 		2
#define DICT_ENTRY_TYPE_EXPR 		3
#define DICT_ENTRY_TYPE_SUBDICT		4
#define DICT_ENTRY_TYPE_LIST		5

struct kest_dictionary;
struct kest_dictionary_entry_ptr_list;

typedef struct {
	const char *name;
	int type;
	int line;
	union {
		int val_int;
		float val_float;
		const char *val_string;
		kest_expression *val_expr;
		struct kest_dictionary *val_dict;
		struct kest_dictionary_entry_list *val_list;
	} value;
} kest_dictionary_entry;

DECLARE_LIST(kest_dictionary_entry);

#define KEST_EFF_ENTRY_TYPE_NOT_FOUND 	22222
#define KEST_EFF_ENTRY_TYPE_NOTHING 	22221
#define KEST_EFF_ENTRY_TYPE_STR 		2
#define KEST_EFF_ENTRY_TYPE_EXPR 		3
#define KEST_EFF_ENTRY_TYPE_SUBDICT		4
#define KEST_EFF_ENTRY_TYPE_LIST		5

struct kest_eff_entry_dict;
struct kest_eff_entry_list;

typedef struct {
	const char *name;
	int type;
	int line;
	union {
		const char *val_string;
		kest_expression *val_expr;
		struct kest_eff_entry_dict *val_dict;
		struct kest_eff_entry_list *val_list;
	} value;
} kest_eff_entry;

DECLARE_LIST(kest_eff_entry);

typedef struct kest_eff_entry_dict {
	kest_eff_entry *entries;
	size_t capacity;
	size_t count;
} kest_eff_entry_dict;

int kest_eff_entry_dict_init(kest_eff_entry_dict *dict);
void kest_eff_entry_dict_print(kest_eff_entry_dict *dict);

size_t kest_eff_entry_dict_count(kest_eff_entry_dict *dict);

int kest_eff_entry_dict_insert(kest_eff_entry_dict *dict, const char *key, kest_eff_entry entry);

const char *kest_eff_entry_dict_index_key(kest_eff_entry_dict *dict, size_t i);
kest_eff_entry *kest_eff_entry_dict_index(kest_eff_entry_dict *dict, size_t i);
kest_eff_entry *kest_eff_entry_dict_lookup(kest_eff_entry_dict *dict, const char *key);

const char *kest_eff_entry_type_to_string(int type);
const char *kest_eff_entry_type_to_string_nice(int type);
kest_string *kest_eff_entry_to_string(kest_eff_entry *entry);
kest_string *kest_eff_entry_to_string_nice(kest_eff_entry *entry);


struct kest_eff_parsing_state;

int kest_parse_eff_entry(struct kest_eff_parsing_state *ps, kest_eff_entry *result);
int kest_parse_eff_entries(struct kest_eff_parsing_state *ps, kest_eff_entry_dict *dict);







const char *kest_dict_entry_type_to_string(int type);
const char *kest_dict_entry_type_to_string_nice(int type);
kest_string *kest_dict_entry_to_string(kest_dictionary_entry *entry);

int kest_dictionary_entry_is_constant_number(kest_dictionary_entry *entry);
float kest_const_num_dictionary_entry_evaluate(kest_dictionary_entry *entry);

typedef struct kest_dictionary_bucket {
	int n_entries;
	int entry_array_length;
	kest_dictionary_entry *entries;
} kest_dictionary_bucket;

struct kest_dictionary;

int kest_dictionary_bucket_add_entry(kest_dictionary_bucket *dict, kest_dictionary_entry entry);

int kest_dictionary_bucket_add_entry_str  (kest_dictionary_bucket *dict, const char *name, const char *value);
int kest_dictionary_bucket_add_entry_int  (kest_dictionary_bucket *dict, const char *name, int value);
int kest_dictionary_bucket_add_entry_float(kest_dictionary_bucket *dict, const char *name, float value);
int kest_dictionary_bucket_add_entry_expr (kest_dictionary_bucket *dict, const char *name, kest_expression *value);
int kest_dictionary_bucket_add_entry_dict (kest_dictionary_bucket *dict, const char *name, struct kest_dictionary *value);

int kest_dictionary_bucket_lookup_entry(kest_dictionary_bucket *dict, const char *name, kest_dictionary_entry *result);
int kest_dictionary_bucket_lookup_str  (kest_dictionary_bucket *dict, const char *name, const char **result);
int kest_dictionary_bucket_lookup_float(kest_dictionary_bucket *dict, const char *name, float *result);
int kest_dictionary_bucket_lookup_int  (kest_dictionary_bucket *dict, const char *name, int *result);
int kest_dictionary_bucket_lookup_expr (kest_dictionary_bucket *dict, const char *name, kest_expression **result);
int kest_dictionary_bucket_lookup_dict (kest_dictionary_bucket *dict, const char *name, struct kest_dictionary **result);

#define KEST_DICTIONARY_N_BUCKETS 8

typedef struct kest_dictionary {
	const char *name;
	
	int n_entries;
	int entry_array_length;
	kest_dictionary_entry *entries;
	
	kest_dictionary_bucket buckets[KEST_DICTIONARY_N_BUCKETS];
} kest_dictionary;

kest_dictionary *kest_new_dictionary();

int kest_dictionary_add_entry(kest_dictionary *dict, kest_dictionary_entry entry);

int kest_dictionary_add_entry_str  (kest_dictionary *dict, const char *name, const char *value);
int kest_dictionary_add_entry_int  (kest_dictionary *dict, const char *name, int value);
int kest_dictionary_add_entry_float(kest_dictionary *dict, const char *name, float value);
int kest_dictionary_add_entry_expr (kest_dictionary *dict, const char *name, kest_expression *value);
int kest_dictionary_add_entry_dict (kest_dictionary *dict, const char *name, kest_dictionary *value);

kest_dictionary_entry *kest_dictionary_get_entry(kest_dictionary *dict, const char *name);

int kest_dictionary_lookup_entry(kest_dictionary *dict, const char *name, kest_dictionary_entry *result);
int kest_dictionary_lookup_str  (kest_dictionary *dict, const char *name, const char **result);
int kest_dictionary_lookup_float(kest_dictionary *dict, const char *name, float *result);
int kest_dictionary_lookup_int  (kest_dictionary *dict, const char *name, int *result);
int kest_dictionary_lookup_expr (kest_dictionary *dict, const char *name, kest_expression **result);
int kest_dictionary_lookup_list (kest_dictionary *dict, const char *name, kest_dictionary_entry_list **result);
int kest_dictionary_lookup_dict (kest_dictionary *dict, const char *name, kest_dictionary **result);

void print_dict(kest_dictionary *dict);


int kest_parse_dict_val(struct kest_eff_parsing_state *ps, kest_dictionary *dict, kest_dictionary_entry *result);
int kest_parse_dictionary(struct kest_eff_parsing_state *ps, kest_dictionary **result, const char *name);

#endif
