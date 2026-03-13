#ifndef M_TEST_H
#define M_TEST_H

#include <assert.h>
#include "m_int.h"

typedef void (*m_test_fn)(void);

typedef struct m_test_entry {
    const char *name;
    m_test_fn fn;
    struct m_test_entry *next;
} m_test_entry;

extern m_test_entry *m_test_list;

static inline void m_register_test(m_test_entry *t)
{
    t->next = m_test_list;
    m_test_list = t;
}

#define M_TEST(name) \
    static void name(void); \
    static m_test_entry entry_##name = { #name, name, NULL }; \
    static void register_##name(void) __attribute__((constructor, used)); \
    static void register_##name(void) { \
        m_register_test(&entry_##name); \
    } \
    static void name(void)

#endif
