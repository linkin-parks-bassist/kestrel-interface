#ifndef KEST_TEST_H
#define KEST_TEST_H

#include <assert.h>
#include "kest_int.h"

typedef void (*kest_test_fn)(void);

typedef struct kest_test_entry {
    const char *name;
    kest_test_fn fn;
    struct kest_test_entry *next;
} kest_test_entry;

extern kest_test_entry *kest_test_list;

static inline void kest_register_test(kest_test_entry *t)
{
    t->next = kest_test_list;
    kest_test_list = t;
}

#define KEST_TEST(name) \
    static void name(void); \
    static kest_test_entry entry_##name = { #name, name, NULL }; \
    static void register_##name(void) __attribute__((constructor, used)); \
    static void register_##name(void) { \
        kest_register_test(&entry_##name); \
    } \
    static void name(void)

#endif
