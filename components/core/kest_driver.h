#ifndef KEST_DRIVER_H_
#define KEST_DRIVER_H_

#define KEST_DRIVER_NOTHING		0
#define KEST_DRIVER_SCOPE_ENTRY 1

struct kest_scope_entry;
struct kest_scope;

struct kest_effect;

typedef struct {
	const char *key;
	struct kest_scope_entry *entry;
	struct kest_scope *scope;
} kest_driver_scope_entry;

typedef struct {
	int type;
	void *data;
} kest_driver;

DECLARE_LIST(kest_driver);

int kest_driver_init(kest_driver *driver);

int kest_driver_init_scope_entry(kest_driver *driver, const char *key);

int kest_driver_evaluate(kest_driver *driver, float *dest);

int kest_driver_clone(kest_driver *dest, kest_driver *src);
kest_driver *kest_driver_make_clone(kest_driver *src);
int kest_driver_clone_for(kest_driver *dest, kest_driver *src, struct kest_effect *effect);

#endif
