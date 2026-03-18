#ifndef M_DICTIONARY_H_
#define M_DICTIONARY_H_

#define DICT_ENTRY_TYPE_NOT_FOUND 	22222
#define DICT_ENTRY_TYPE_NOTHING 	22221
#define DICT_ENTRY_TYPE_INT 		0
#define DICT_ENTRY_TYPE_FLOAT 		1
#define DICT_ENTRY_TYPE_STR 		2
#define DICT_ENTRY_TYPE_EXPR 		3
#define DICT_ENTRY_TYPE_SUBDICT		4
#define DICT_ENTRY_TYPE_LIST		5

struct m_dictionary;
struct m_dictionary_entry_ptr_list;

typedef struct {
	const char *name;
	int type;
	int line;
	union {
		int val_int;
		float val_float;
		const char *val_string;
		m_expression *val_expr;
		struct m_dictionary *val_dict;
		struct m_dictionary_entry_list *val_list;
	} value;
} m_dictionary_entry;

DECLARE_LIST(m_dictionary_entry);

const char *m_dict_entry_type_to_string(int type);
const char *m_dict_entry_type_to_string_nice(int type);
m_string *m_dict_entry_to_string(m_dictionary_entry *entry);

typedef struct m_dictionary_bucket {
	int n_entries;
	int entry_array_length;
	m_dictionary_entry *entries;
} m_dictionary_bucket;

struct m_dictionary;

int m_dictionary_bucket_add_entry(m_dictionary_bucket *dict, m_dictionary_entry entry);

int m_dictionary_bucket_add_entry_str  (m_dictionary_bucket *dict, const char *name, const char *value);
int m_dictionary_bucket_add_entry_int  (m_dictionary_bucket *dict, const char *name, int value);
int m_dictionary_bucket_add_entry_float(m_dictionary_bucket *dict, const char *name, float value);
int m_dictionary_bucket_add_entry_expr (m_dictionary_bucket *dict, const char *name, m_expression *value);
int m_dictionary_bucket_add_entry_dict (m_dictionary_bucket *dict, const char *name, struct m_dictionary *value);

int m_dictionary_bucket_lookup_str  (m_dictionary_bucket *dict, const char *name, const char **result);
int m_dictionary_bucket_lookup_float(m_dictionary_bucket *dict, const char *name, float *result);
int m_dictionary_bucket_lookup_int  (m_dictionary_bucket *dict, const char *name, int *result);
int m_dictionary_bucket_lookup_expr (m_dictionary_bucket *dict, const char *name, m_expression **result);
int m_dictionary_bucket_lookup_dict (m_dictionary_bucket *dict, const char *name, struct m_dictionary **result);

#define M_DICTIONARY_N_BUCKETS 8

typedef struct m_dictionary {
	const char *name;
	
	int n_entries;
	int entry_array_length;
	m_dictionary_entry *entries;
	
	m_dictionary_bucket buckets[M_DICTIONARY_N_BUCKETS];
} m_dictionary;

m_dictionary *m_new_dictionary();

int m_dictionary_add_entry(m_dictionary *dict, m_dictionary_entry entry);

int m_dictionary_add_entry_str  (m_dictionary *dict, const char *name, const char *value);
int m_dictionary_add_entry_int  (m_dictionary *dict, const char *name, int value);
int m_dictionary_add_entry_float(m_dictionary *dict, const char *name, float value);
int m_dictionary_add_entry_expr (m_dictionary *dict, const char *name, m_expression *value);
int m_dictionary_add_entry_dict (m_dictionary *dict, const char *name, m_dictionary *value);

int m_dictionary_lookup_str  (m_dictionary *dict, const char *name, const char **result);
int m_dictionary_lookup_float(m_dictionary *dict, const char *name, float *result);
int m_dictionary_lookup_int  (m_dictionary *dict, const char *name, int *result);
int m_dictionary_lookup_expr (m_dictionary *dict, const char *name, m_expression **result);
int m_dictionary_lookup_list (m_dictionary *dict, const char *name, m_dictionary_entry_list **result);
int m_dictionary_lookup_dict (m_dictionary *dict, const char *name, m_dictionary **result);

#define ERR_NOT_FOUND  10
#define ERR_WRONG_TYPE 11

void print_dict(m_dictionary *dict);

struct m_eff_parsing_state;

int m_parse_dict_val(struct m_eff_parsing_state *ps, m_dictionary *dict, m_dictionary_entry *result);
int m_parse_dictionary(struct m_eff_parsing_state *ps, m_dictionary **result, const char *name);

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
	m_allocator alloc;\
} X##_ptr_dict;\
\
int X##_ptr_dict_init(X##_ptr_dict *dict, size_t n_buckets);\
int X##_ptr_dict_init_with_allocator(X##_ptr_dict *dict, size_t n_buckets, m_allocator *alloc);\
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
int X##_ptr_dict_init_with_allocator(X##_ptr_dict *dict, size_t n_buckets, m_allocator *alloc)\
{\
	if (!dict) return ERR_NULL_PTR;\
	if (n_buckets == 0)\
	{\
		dict->buckets = NULL;\
		dict->n_buckets = 0;\
		dict->alloc = alloc ? *alloc : (m_allocator){0};\
		return NO_ERROR;\
	}\
	\
	dict->n_buckets = 0;\
	dict->buckets = m_allocator_alloc(alloc, sizeof(X##_ptr_dict_bucket) * n_buckets);\
	\
	if (!dict->buckets)\
		return ERR_ALLOC_FAIL;\
	\
	for (size_t i = 0; i < n_buckets; i++)\
		X##_ptr_dict_entry_list_init_with_allocator(&dict->buckets[i], alloc);\
	\
	dict->n_buckets = n_buckets;\
	dict->alloc = alloc ? *alloc : (m_allocator){0};\
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
	entry.key = m_allocator_strndup(&dict->alloc, key, 1024);\
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
				m_allocator_free(&dict->alloc, dict->buckets[i].entries[j].key);\
			}\
			\
			m_allocator_free(&dict->buckets[i].alloc, dict->buckets[i].entries); \
			\
			dict->buckets[i].entries = NULL; \
			dict->buckets[i].count = 0; \
			dict->buckets[i].capacity = 0; \
			\
		}\
		\
		m_allocator_free(&dict->alloc, dict->buckets);\
		dict->buckets = NULL;\
	}\
	\
	dict->n_buckets = 0;\
}

#endif
