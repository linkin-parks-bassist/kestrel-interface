---
status: green
revised_at: "2026-10-06T09:50:03+11:00"
---

components/core/kest_eff_desc.h defines descriptor name/cname, optional description and keywords/instruments/types/genres string lists, parameter/setting/resource/block/definition lists, drivers, resource report, scope, captured expressions, instance reference count and retirement flag, plus its typed pool/allocator.

kest_init_effect_desc rejects NULL and zeroes the complete value. Use fresh or reclaimed storage; initialization does not destroy previous allocations. Optional discovery strings and list entries are descriptor-owned heap copies. Missing description is NULL; missing/empty lists have zero entries. Authored case, order, duplicates and full text are preserved.

kest_effect_desc_retain/release track instance borrows. Retirement destroys an unreferenced descriptor immediately or waits for its final release. Destruction releases discovery strings/list arrays, other owned metadata/payloads and captured expressions. Template lists use matching typed destructors after copied names are freed. Metadata remains valid through parser reset and retained retirement; the allocation probe checks all thirteen discovery-copy allocation boundaries over three rejection/recovery passes.

Resource reporting counts blocks, memory, delay and filters; kest_eff_desc_create_scope constructs parameter scope entries. The header uses KEST_EFFECT_H_ as its guard. Broader lifetime/fault variants remain governed by parser-arena and refresh-design owners.

Sources: components/core/kest_eff_desc.[ch], tests/parser/kest_readback_effect_test.c and tools/test_parser_allocation.c.
