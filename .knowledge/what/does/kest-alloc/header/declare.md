---
status: green
revised_at: "2026-10-04T06:50:54+11:00"
---

`components/core/kest_alloc.h` declares tracked heap allocation, allocator initialization/allocation/reallocation/string-copy/free dispatch, LVGL heap wrappers, memory initialization and reporting. `kest_allocator` contains callback functions, opaque data and flags prohibiting free, reallocation or string copying. Allocation and reallocation sizes are always bytes, including typed pool adapters.

David intends preallocation of known important structs to avoid repeated embedded heap-allocation costs. He reports patchy adoption and many remaining system-malloc allocations; completing useful strategy integration is the direction, rather than replacing the architecture.

`kest_mem_init` initializes and reserves typed pools for DSP resources, effect descriptors, expressions, parameters, settings, effects and presets, plus sequences outside the library build. It installs each pool's allocator into the corresponding global typed allocator. A failed pool initialization/reservation returns immediately; initialization is not transactional.

Typed constructors request `sizeof(type)`. Pool adapters accept exactly `sizeof(X)` and reject other sizes; the obsolete singular/object-count flag is removed. Null allocators or missing allocation callbacks fall back to the tracked heap using the same byte-size contract. The C suite exercises the production parameter constructor with both a typed pool and a zero-initialized heap allocator, paired release, and heap cloning. This establishes size compatibility, not arbitrary live strategy replacement.

Allocation and release must use compatible strategies. Switching a global allocator while its objects remain alive does not transfer their ownership; resolve lifecycle boundaries before enabling runtime substitution. Absent free/reallocation callbacks use heap fallbacks, so arbitrary partially configured allocators are not necessarily compatible.

Source: components/core/kest_alloc.[ch], components/core/kest_pool.h, typed constructor call sites and tests/core/kest_pool_test.c. Reallocator-specific hazards belong to what/risks/are/visible/in/custom/reallocators.md; scratchpad callback destination lifetime belongs to what/is/the/status/of/periodic/fpga/memory/reads.md.
