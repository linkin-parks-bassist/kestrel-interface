#ifndef KEST_DICT_H_
#define KEST_DICT_H_

#define DECLARE_DICT(X) \
typedef struct {\
	const char *key; \
	X data; \
} X##_dict_entry; \
\
DECLARE_LIST(X##_dict_entry);\
typedef X##_dict_entry_list X##_dict_bucket;\
\
typedef struct {\
	X##_dict_bucket *buckets;\
	size_t n_buckets;\
	kest_allocator alloc;\
} X##_dict;\
\
int X##_dict_init(X##_dict *dict, size_t n_buckets);\
int X##_dict_init_with_allocator(X##_dict *dict, size_t n_buckets, kest_allocator *alloc);\
int X##_dict_insert(X##_dict *dict, const char *key, X x);\
X *X##_dict_insert_return_ptr(X##_dict *dict, const char *key, X x);\
X##_dict_entry *X##_dict_insert_return_entry_ptr(X##_dict *dict, const char *key, X x);\
X *X##_dict_lookup(X##_dict *dict, const char *key);\
X##_dict_entry *X##_dict_lookup_entry(X##_dict *dict, const char *key);\
size_t X##_dict_count(X##_dict *dict);\
X *X##_dict_index(X##_dict *dict, size_t n);\
const char *X##_dict_index_key(X##_dict *dict, size_t n);\
X##_dict_entry *X##_dict_index_entry(X##_dict *dict, size_t n);\
void X##_dict_destroy(X##_dict *dict, void (*destructor)(X *x));

#define IMPLEMENT_DICT(X) \
IMPLEMENT_LIST(X##_dict_entry);\
int X##_dict_init(X##_dict *dict, size_t n_buckets)\
{\
	return X##_dict_init_with_allocator(dict, n_buckets, NULL);\
}\
\
int X##_dict_init_with_allocator(X##_dict *dict, size_t n_buckets, kest_allocator *alloc)\
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
	dict->buckets = kest_allocator_alloc(alloc, sizeof(X##_dict_bucket) * n_buckets);\
	\
	if (!dict->buckets)\
		return ERR_ALLOC_FAIL;\
	\
	for (size_t i = 0; i < n_buckets; i++)\
		X##_dict_entry_list_init_with_allocator(&dict->buckets[i], alloc);\
	\
	dict->n_buckets = n_buckets;\
	dict->alloc = alloc ? *alloc : (kest_allocator){0};\
	\
	return NO_ERROR;\
}\
\
int X##_dict_entry_cmp(const X##_dict_entry *a, const X##_dict_entry *b)\
{\
	if (!a || !b) return -1;\
	if (!a->key || !b->key) return -1;\
	return strcmp(a->key, b->key);\
}\
\
int X##_dict_insert(X##_dict *dict, const char *key, X x)\
{\
	if (!dict) return ERR_NULL_PTR;\
	if (!key) return ERR_BAD_ARGS;\
	if (!dict->n_buckets) return ERR_BAD_ARGS;\
	\
	size_t bucket = hash(key) % dict->n_buckets;\
	\
	X##_dict_entry entry = {.key = key, .data = x};\
	\
	int index = X##_dict_entry_list_index_of(&dict->buckets[bucket], entry, X##_dict_entry_cmp);\
	\
	if (index >= 0)\
		return ERR_DUPLICATE_KEY;\
	\
	entry.key = kest_allocator_strndup(&dict->alloc, key, 1024);\
	\
	if (!entry.key)\
		return ERR_ALLOC_FAIL;\
	\
	return X##_dict_entry_list_append(&dict->buckets[bucket], entry);\
}\
\
X *X##_dict_insert_return_ptr(X##_dict *dict, const char *key, X x)\
{\
	if (!dict) 				return NULL;\
	if (!key) 				return NULL;\
	if (!dict->n_buckets) 	return NULL;\
	\
	size_t bucket = hash(key) % dict->n_buckets;\
	\
	X##_dict_entry entry = {.key = key, .data = x};\
	\
	int index = X##_dict_entry_list_index_of(&dict->buckets[bucket], entry, X##_dict_entry_cmp);\
	\
	if (index >= 0)\
		return NULL;\
	\
	entry.key = kest_allocator_strndup(&dict->alloc, key, 1024);\
	\
	if (!entry.key)\
		return NULL;\
	\
	X##_dict_entry *entry_ptr = X##_dict_entry_list_append_return_ptr(&dict->buckets[bucket], entry);\
	\
	if (!entry_ptr) return NULL;\
	\
	return &entry_ptr->data;\
}\
\
X##_dict_entry *X##_dict_insert_return_entry_ptr(X##_dict *dict, const char *key, X x)\
{\
	if (!dict) 				return NULL;\
	if (!key) 				return NULL;\
	if (!dict->n_buckets) 	return NULL;\
	\
	size_t bucket = hash(key) % dict->n_buckets;\
	\
	X##_dict_entry entry = {.key = key, .data = x};\
	\
	int index = X##_dict_entry_list_index_of(&dict->buckets[bucket], entry, X##_dict_entry_cmp);\
	\
	if (index >= 0)\
		return NULL;\
	\
	entry.key = kest_allocator_strndup(&dict->alloc, key, 1024);\
	\
	if (!entry.key)\
		return NULL;\
	\
	return X##_dict_entry_list_append_return_ptr(&dict->buckets[bucket], entry);\
}\
X *X##_dict_lookup(X##_dict *dict, const char *key)\
{\
	if (!dict || !key) return NULL;\
	if (!dict->n_buckets) return NULL;\
	\
	X##_dict_entry entry = {.key = key};\
	\
	size_t bucket = hash(key) % dict->n_buckets;\
	\
	int index = X##_dict_entry_list_index_of(&dict->buckets[bucket], entry, X##_dict_entry_cmp);\
	\
	if (index >= 0 && index < dict->buckets[bucket].count)\
		return &dict->buckets[bucket].entries[index].data;\
	\
	return NULL;\
}\
X##_dict_entry *X##_dict_lookup_entry(X##_dict *dict, const char *key)\
{\
	if (!dict || !key) return NULL;\
	if (!dict->n_buckets) return NULL;\
	\
	X##_dict_entry entry = {.key = key};\
	\
	size_t bucket = hash(key) % dict->n_buckets;\
	\
	int index = X##_dict_entry_list_index_of(&dict->buckets[bucket], entry, X##_dict_entry_cmp);\
	\
	if (index >= 0 && index < dict->buckets[bucket].count)\
		return &dict->buckets[bucket].entries[index];\
	\
	return NULL;\
}\
size_t X##_dict_count(X##_dict *dict)\
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
X *X##_dict_index(X##_dict *dict, size_t n)\
{\
	if (!dict) return NULL;\
	\
	if (!dict->buckets || !dict->n_buckets)\
		return NULL;\
	\
	for (size_t i = 0; i < dict->n_buckets; i++)\
	{\
		if (dict->buckets[i].count > n)\
			return &dict->buckets[i].entries[n].data;\
		else\
			n -= dict->buckets[i].count;\
	}\
	\
	return NULL;\
}\
const char *X##_dict_index_key(X##_dict *dict, size_t n)\
{\
	if (!dict) return NULL;\
	\
	if (!dict->buckets || !dict->n_buckets)\
		return NULL;\
	\
	for (size_t i = 0; i < dict->n_buckets; i++)\
	{\
		if (dict->buckets[i].count > n)\
			return dict->buckets[i].entries[n].key;\
		else\
			n -= dict->buckets[i].count;\
	}\
	\
	return NULL;\
}\
X##_dict_entry *X##_dict_index_entry(X##_dict *dict, size_t n)\
{\
	if (!dict) return NULL;\
	\
	if (!dict->buckets || !dict->n_buckets)\
		return NULL;\
	\
	for (size_t i = 0; i < dict->n_buckets; i++)\
	{\
		if (dict->buckets[i].count > n)\
			return &dict->buckets[i].entries[n];\
		else\
			n -= dict->buckets[i].count;\
	}\
	\
	return NULL;\
}\
void X##_dict_destroy(X##_dict *dict, void (*destructor)(X *x))\
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
					destructor(&dict->buckets[i].entries[j].data); \
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
