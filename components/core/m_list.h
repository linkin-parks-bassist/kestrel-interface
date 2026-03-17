#ifndef M_DYNAMIC_ARRAY_H_
#define M_DYNAMIC_ARRAY_H_

#define DECLARE_LIST(X) \
typedef struct { \
    X* entries; \
    int count; \
    int capacity; \
    m_allocator alloc; \
} X##_list; \
\
int X##_list_init(X##_list *list); \
int X##_list_init_with_allocator(X##_list *list, const m_allocator *alloc); \
int X##_list_init_reserved(X##_list *list, size_t n); \
int X##_list_init_reserved_with_allocator(X##_list *list, size_t n, const m_allocator *alloc); \
int X##_list_reserve(X##_list *list, size_t n); \
int X##_list_append(X##_list *list, X x); \
int X##_list_append_ref(X##_list *list, const X *x); \
int X##_list_destroy(X##_list *list); \
int X##_list_destroy_all(X##_list *list, void (*destructor)(X *x)); \
int X##_list_contains(X##_list *list, X x, int (*cmp)(const X*, const X*)); \
int X##_list_contains_ref(X##_list *list, const X *x, int (*cmp)(const X*, const X*)); \
int X##_list_index_of(X##_list *list, X x, int (*cmp)(const X*, const X*)); \
int X##_list_index_of_ref(X##_list *list, const X *x, int (*cmp)(const X*, const X*));

#define IMPLEMENT_LIST(X) \
\
int X##_list_init(X##_list *list) \
{ \
    return X##_list_init_with_allocator(list, NULL); \
} \
\
int X##_list_init_with_allocator(X##_list *list, const m_allocator *alloc) \
{ \
    if (!list) return ERR_NULL_PTR; \
\
    list->entries = NULL; \
    list->count = 0; \
    list->capacity = 0; \
    list->alloc = alloc ? *alloc : (m_allocator){0}; \
\
    return NO_ERROR; \
} \
\
int X##_list_init_reserved(X##_list *list, size_t n) \
{ \
    return X##_list_init_reserved_with_allocator(list, n, NULL); \
} \
\
int X##_list_init_reserved_with_allocator(X##_list *list, size_t n, const m_allocator *alloc) \
{ \
    int r = X##_list_init_with_allocator(list, alloc); \
    if (r != NO_ERROR) return r; \
\
    if (n == 0) return NO_ERROR; \
\
    list->entries = m_allocator_alloc(&list->alloc, sizeof(X) * n); \
    if (!list->entries) return ERR_ALLOC_FAIL; \
\
    memset(list->entries, 0, sizeof(X) * n); \
    list->capacity = n; \
\
    return NO_ERROR; \
} \
\
int X##_list_reserve(X##_list *list, size_t n) \
{ \
    if (!list) return ERR_NULL_PTR; \
    if (!n) return NO_ERROR; \
\
    if (list->count + n <= list->capacity) \
        return NO_ERROR; \
\
    size_t cap_needed = list->count + n - 1; \
    cap_needed |= cap_needed >> 1; \
    cap_needed |= cap_needed >> 2; \
    cap_needed |= cap_needed >> 4; \
    cap_needed |= cap_needed >> 8; \
    cap_needed |= cap_needed >> 16; \
    if (sizeof(size_t) > 4) cap_needed |= cap_needed >> 32; \
    cap_needed += 1; \
\
    if (!list->entries) \
    { \
        list->entries = m_allocator_alloc(&list->alloc, sizeof(X) * cap_needed); \
        if (!list->entries) \
        { \
            list->count = 0; \
            list->capacity = 0; \
            return ERR_ALLOC_FAIL; \
        } \
        memset(list->entries, 0, sizeof(X) * cap_needed); \
    } \
    else \
    { \
        X *new_array = m_allocator_realloc(&list->alloc, list->entries, sizeof(X) * cap_needed); \
        if (!new_array) return ERR_ALLOC_FAIL; \
\
        list->entries = new_array; \
        memset(&list->entries[list->count], 0, sizeof(X) * (cap_needed - list->count)); \
    } \
\
    list->capacity = cap_needed; \
    return NO_ERROR; \
} \
\
int X##_list_append(X##_list *list, X x) \
{ \
    return X##_list_append_ref(list, &x); \
} \
\
int X##_list_append_ref(X##_list *list, const X *x) \
{ \
    if (!list) return ERR_NULL_PTR; \
    if (!x) return ERR_BAD_ARGS; \
\
    int r = X##_list_reserve(list, 1); \
    if (r != NO_ERROR) return r; \
\
    memcpy(&list->entries[list->count], x, sizeof(X)); \
    list->count++; \
\
    return NO_ERROR; \
} \
\
int X##_list_destroy(X##_list *list) \
{ \
    if (!list) return ERR_NULL_PTR; \
\
    if (list->entries) \
        m_allocator_free(&list->alloc, list->entries); \
\
    list->entries = NULL; \
    list->count = 0; \
    list->capacity = 0; \
\
    return NO_ERROR; \
} \
\
int X##_list_destroy_all(X##_list *list, void (*destructor)(X *x)) \
{ \
    if (!list) return ERR_NULL_PTR; \
\
    if (!list->entries) \
    { \
        list->count = 0; \
        list->capacity = 0; \
        return NO_ERROR; \
    } \
\
    if (destructor) \
    { \
        for (int i = 0; i < list->count; i++) \
            destructor(&list->entries[i]); \
    } \
\
    m_allocator_free(&list->alloc, list->entries); \
\
    list->entries = NULL; \
    list->count = 0; \
    list->capacity = 0; \
\
    return NO_ERROR; \
} \
\
int X##_list_contains(X##_list *list, X x, int (*cmp)(const X*, const X*)) \
{ \
    return X##_list_contains_ref(list, &x, cmp); \
} \
\
int X##_list_contains_ref(X##_list *list, const X *x, int (*cmp)(const X*, const X*)) \
{ \
    if (!list || !list->entries) return 0; \
\
    for (int i = 0; i < list->count && i < list->capacity; i++) \
    { \
        if (cmp) \
        { \
            if (cmp(&list->entries[i], x) == 0) return 1; \
        } \
        else \
        { \
            if (memcmp(&list->entries[i], x, sizeof(X)) == 0) return 1; \
        } \
    } \
    return 0; \
} \
\
int X##_list_index_of(X##_list *list, X x, int (*cmp)(const X*, const X*)) \
{ \
    return X##_list_index_of_ref(list, &x, cmp); \
} \
\
int X##_list_index_of_ref(X##_list *list, const X *x, int (*cmp)(const X*, const X*)) \
{ \
    if (!list || !list->entries) return -1; \
\
    for (int i = 0; i < list->count && i < list->capacity; i++) \
    { \
        if (cmp) \
        { \
            if (cmp(&list->entries[i], x) == 0) return i; \
        } \
        else \
        { \
            if (memcmp(&list->entries[i], x, sizeof(X)) == 0) return i; \
        } \
    } \
    return -1; \
}

#define DECLARE_PTR_LIST(X) \
typedef struct { \
    X** entries; \
    int count; \
    int capacity; \
    m_allocator alloc; \
} X##_ptr_list; \
\
int X##_ptr_list_init(X##_ptr_list *list); \
int X##_ptr_list_init_with_allocator(X##_ptr_list *list, const m_allocator *alloc); \
int X##_ptr_list_reserve(X##_ptr_list *list, size_t n); \
int X##_ptr_list_append(X##_ptr_list *list, X *x); \
int X##_ptr_list_destroy(X##_ptr_list *list); \
int X##_ptr_list_destroy_all(X##_ptr_list *list, void (*destructor)(X *x)); \
int X##_ptr_list_contains(X##_ptr_list *list, X *x); \
int X##_ptr_list_index_of(X##_ptr_list *list, X *x);


#define IMPLEMENT_PTR_LIST(X) \
\
int X##_ptr_list_init(X##_ptr_list *list) \
{ \
    return X##_ptr_list_init_with_allocator(list, NULL); \
} \
\
int X##_ptr_list_init_with_allocator(X##_ptr_list *list, const m_allocator *alloc) \
{ \
    if (!list) return ERR_NULL_PTR; \
\
    list->entries = NULL; \
    list->count = 0; \
    list->capacity = 0; \
    list->alloc = alloc ? *alloc : (m_allocator){0}; \
\
    return NO_ERROR; \
} \
\
int X##_ptr_list_reserve(X##_ptr_list *list, size_t n) \
{ \
    if (!list) return ERR_NULL_PTR; \
    if (!n) return NO_ERROR; \
\
    if (list->count + n <= list->capacity) \
        return NO_ERROR; \
\
    size_t cap_needed = list->count + n - 1; \
    cap_needed |= cap_needed >> 1; \
    cap_needed |= cap_needed >> 2; \
    cap_needed |= cap_needed >> 4; \
    cap_needed |= cap_needed >> 8; \
    cap_needed |= cap_needed >> 16; \
    if (sizeof(size_t) > 4) cap_needed |= cap_needed >> 32; \
    cap_needed += 1; \
\
    if (!list->entries) \
    { \
        list->entries = m_allocator_alloc(&list->alloc, sizeof(X*) * cap_needed); \
        if (!list->entries) return ERR_ALLOC_FAIL; \
        memset(list->entries, 0, sizeof(X*) * cap_needed); \
    } \
    else \
    { \
        X **new_array = m_allocator_realloc(&list->alloc, list->entries, sizeof(X*) * cap_needed); \
        if (!new_array) return ERR_ALLOC_FAIL; \
\
        list->entries = new_array; \
        memset(&list->entries[list->count], 0, sizeof(X*) * (cap_needed - list->count)); \
    } \
\
    list->capacity = cap_needed; \
    return NO_ERROR; \
} \
\
int X##_ptr_list_append(X##_ptr_list *list, X *x) \
{ \
    if (!list) return ERR_NULL_PTR; \
\
    int r = X##_ptr_list_reserve(list, 1); \
    if (r != NO_ERROR) return r; \
\
    list->entries[list->count++] = x; \
    return NO_ERROR; \
} \
\
int X##_ptr_list_destroy(X##_ptr_list *list) \
{ \
    if (!list) return ERR_NULL_PTR; \
\
    if (list->entries) \
        m_allocator_free(&list->alloc, list->entries); \
\
    list->entries = NULL; \
    list->count = 0; \
    list->capacity = 0; \
\
    return NO_ERROR; \
} \
\
int X##_ptr_list_destroy_all(X##_ptr_list *list, void (*destructor)(X *x)) \
{ \
    if (!list) return ERR_NULL_PTR; \
\
    if (!list->entries) \
    { \
        list->count = 0; \
        list->capacity = 0; \
        return NO_ERROR; \
    } \
\
    if (destructor) \
    { \
        for (int i = 0; i < list->count; i++) \
            if (list->entries[i]) destructor(list->entries[i]); \
    } \
    else \
    { \
        for (int i = 0; i < list->count; i++) \
            if (list->entries[i]) m_allocator_free(&list->alloc, list->entries[i]); \
    } \
\
    m_allocator_free(&list->alloc, list->entries); \
\
    list->entries = NULL; \
    list->count = 0; \
    list->capacity = 0; \
\
    return NO_ERROR; \
} \
\
int X##_ptr_list_contains(X##_ptr_list *list, X *x) \
{ \
    if (!list || !list->entries) return 0; \
\
    for (int i = 0; i < list->count; i++) \
        if (list->entries[i] == x) return 1; \
\
    return 0; \
} \
\
int X##_ptr_list_index_of(X##_ptr_list *list, X *x) \
{ \
    if (!list || !list->entries) return -1; \
\
    for (int i = 0; i < list->count; i++) \
        if (list->entries[i] == x) return i; \
\
    return -1; \
}

#endif
