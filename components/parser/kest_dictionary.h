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

const char *kest_dict_entry_type_to_string(int type);
const char *kest_dict_entry_type_to_string_nice(int type);
kest_string *kest_dict_entry_to_string(kest_dictionary_entry *entry);

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

int kest_dictionary_lookup_entry(kest_dictionary *dict, const char *name, kest_dictionary_entry *result);
int kest_dictionary_lookup_str  (kest_dictionary *dict, const char *name, const char **result);
int kest_dictionary_lookup_float(kest_dictionary *dict, const char *name, float *result);
int kest_dictionary_lookup_int  (kest_dictionary *dict, const char *name, int *result);
int kest_dictionary_lookup_expr (kest_dictionary *dict, const char *name, kest_expression **result);
int kest_dictionary_lookup_list (kest_dictionary *dict, const char *name, kest_dictionary_entry_list **result);
int kest_dictionary_lookup_dict (kest_dictionary *dict, const char *name, kest_dictionary **result);

#define ERR_NOT_FOUND  10
#define ERR_WRONG_TYPE 11

void print_dict(kest_dictionary *dict);

struct kest_eff_parsing_state;

int kest_parse_dict_val(struct kest_eff_parsing_state *ps, kest_dictionary *dict, kest_dictionary_entry *result);
int kest_parse_dictionary(struct kest_eff_parsing_state *ps, kest_dictionary **result, const char *name);

#endif
