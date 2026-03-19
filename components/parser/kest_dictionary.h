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

#define DECLARE_PTR_DICT(X) \
typedef struct {\
	const char *key; \
	X *data; \
} X##_ptr_dict_entry; \
\
DECLARE_LIST(X##_ptr_dict_entry);\
typedef X##_ptr_dict_entry_list X##_ptr_dict_bucket;\
\
typedef struct {\
	X##_ptr_dict_bucket *buckets;\
	size_t n_buckets;\
	kest_allocator alloc;\
} X##_ptr_dict;\
\
int X##_ptr_dict_init(X##_ptr_dict *dict, size_t n_buckets);\
int X##_ptr_dict_init_with_allocator(X##_ptr_dict *dict, size_t n_buckets, kest_allocator *alloc);\
int X##_ptr_dict_insert(X##_ptr_dict *dict, char *key, X *x);\
X *X##_ptr_dict_lookup(X##_ptr_dict *dict, const char *key);\
size_t X##_ptr_dict_count(X##_ptr_dict *dict);\
X *X##_ptr_dict_index(X##_ptr_dict *dict, size_t n);\
void X##_ptr_dict_destroy(X##_ptr_dict *dict, void (*destructor)(X *x));

#define IMPLEMENT_PTR_DICT(X) \
IMPLEMENT_LIST(X##_ptr_dict_entry);\
int X##_ptr_dict_init(X##_ptr_dict *dict, size_t n_buckets)\
{\
	return X##_ptr_dict_init_with_allocator(dict, n_buckets, NULL);\
}\
\
int X##_ptr_dict_init_with_allocator(X##_ptr_dict *dict, size_t n_buckets, kest_allocator *alloc)\
{\
	if (!dict) return ERR_NULL_PTR;\
	if (n_buckets == 0)\
	{\
		dict->buckets = NULL;\
		dict->n_buckets = 0;\
		dict->alloc = alloc ? *alloc : (kest_allocator){0};\
		return NO_ERROR;\
	}\
	\
	dict->n_buckets = 0;\
	dict->buckets = kest_allocator_alloc(alloc, sizeof(X##_ptr_dict_bucket) * n_buckets);\
	\
	if (!dict->buckets)\
		return ERR_ALLOC_FAIL;\
	\
	for (size_t i = 0; i < n_buckets; i++)\
		X##_ptr_dict_entry_list_init_with_allocator(&dict->buckets[i], alloc);\
	\
	dict->n_buckets = n_buckets;\
	dict->alloc = alloc ? *alloc : (kest_allocator){0};\
	\
	return NO_ERROR;\
}\
\
int X##_ptr_dict_entry_cmp(const X##_ptr_dict_entry *a, const X##_ptr_dict_entry *b)\
{\
	if (!a || !b) return -1;\
	if (!a->key || !b->key) return -1;\
	return strcmp(a->key, b->key);\
}\
\
int X##_ptr_dict_insert(X##_ptr_dict *dict, char *key, X *x)\
{\
	if (!dict) return ERR_NULL_PTR;\
	if (!key) return ERR_BAD_ARGS;\
	if (!dict->n_buckets) return ERR_BAD_ARGS;\
	\
	size_t bucket = hash(key) % dict->n_buckets;\
	\
	X##_ptr_dict_entry entry = {.key = key, .data = x};\
	\
	int index = X##_ptr_dict_entry_list_index_of(&dict->buckets[bucket], entry, X##_ptr_dict_entry_cmp);\
	\
	if (index >= 0)\
		return ERR_DUPLICATE_KEY;\
	\
	entry.key = kest_allocator_strndup(&dict->alloc, key, 1024);\
	\
	if (!entry.key)\
		return ERR_ALLOC_FAIL;\
	\
	return X##_ptr_dict_entry_list_append(&dict->buckets[bucket], entry);\
}\
X *X##_ptr_dict_lookup(X##_ptr_dict *dict, const char *key)\
{\
	if (!dict || !key) return NULL;\
	if (!dict->n_buckets) return NULL;\
	\
	X##_ptr_dict_entry entry = {.key = key, .data = NULL};\
	\
	size_t bucket = hash(key) % dict->n_buckets;\
	\
	int index = X##_ptr_dict_entry_list_index_of(&dict->buckets[bucket], entry, X##_ptr_dict_entry_cmp);\
	\
	if (index >= 0 && index < dict->buckets[bucket].count)\
		return dict->buckets[bucket].entries[index].data;\
	\
	return NULL;\
}\
size_t X##_ptr_dict_count(X##_ptr_dict *dict)\
{\
	if (!dict) return 0;\
	\
	if (!dict->buckets || !dict->n_buckets)\
		return 0;\
	\
	size_t count = 0;\
	for (size_t i = 0; i < dict->n_buckets; i++)\
	{\
		count += dict->buckets[i].count;\
	}\
	\
	return count;\
}\
X *X##_ptr_dict_index(X##_ptr_dict *dict, size_t n)\
{\
	if (!dict) return NULL;\
	\
	if (!dict->buckets || !dict->n_buckets)\
		return NULL;\
	\
	for (size_t i = 0; i < dict->n_buckets; i++)\
	{\
		if (dict->buckets[i].count > n)\
			return dict->buckets[i].entries[n].data;\
		else\
			n -= dict->buckets[i].count;\
	}\
	\
	return NULL;\
}\
void X##_ptr_dict_destroy(X##_ptr_dict *dict, void (*destructor)(X *x))\
{\
	if (!dict)\
		return;\
	\
	if (dict->buckets)\
	{\
		for (size_t i = 0; i < dict->n_buckets; i++)\
		{\
			for (size_t j = 0; j < dict->buckets[i].count; j++) \
			{\
				if (destructor)\
					destructor(dict->buckets[i].entries[j].data); \
				kest_allocator_free(&dict->alloc, dict->buckets[i].entries[j].key);\
			}\
			\
			kest_allocator_free(&dict->buckets[i].alloc, dict->buckets[i].entries); \
			\
			dict->buckets[i].entries = NULL; \
			dict->buckets[i].count = 0; \
			dict->buckets[i].capacity = 0; \
			\
		}\
		\
		kest_allocator_free(&dict->alloc, dict->buckets);\
		dict->buckets = NULL;\
	}\
	\
	dict->n_buckets = 0;\
}

#endif
