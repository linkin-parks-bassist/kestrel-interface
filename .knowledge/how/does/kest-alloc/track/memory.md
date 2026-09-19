---
status: "unverified"
created_at: "2026-09-20T00:07:10+10:00"
scope: "local"
source: "components/core/kest_alloc.c:1-145"
---
Status: Green

`kest_alloc` prepends allocation size, increments current and peak byte counters, and returns the payload pointer. `kest_free` reads the header and decrements current. `kest_realloc` resizes the header-bearing block and updates counters; `kest_mem_init` reserves pools for resources, effect descriptions, expressions, parameters, settings, effects and presets (and sequences outside library builds). Source: components/core/kest_alloc.c:1-145
