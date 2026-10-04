---
status: green
revised_at: "2026-10-04T04:26:23+11:00"
---

init_effect_from_effect_desc initializes the instance, clones resources and blocks, clones parameters/settings, creates its scope, then relinks resource references in cloned blocks. Resource linkage waits until the scope and instance-specific resources exist.

kest_dsp_resource_make_clone_for_effect returns NULL if resource cloning fails before writing the effect backlink. A regression covers a null source and a filter resource without a payload; both return NULL without crashing. This fixes the helper's failure path, not all construction rollback: the effect constructor's disaster-recovery label still has no cleanup implementation.

Resource wrappers/payloads are currently heap-cloned; their release and queued-borrow boundaries belong to how/does/a/preset/pipeline/manage/effects.md. Typed allocation strategies must be paired with compatible release at those boundaries.

Sources: components/core/kest_effect.c, kest_resource.c and tests/core/kest_update_test.c.
