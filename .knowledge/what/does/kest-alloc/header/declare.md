---
status: green
revised_at: "2026-10-06T00:42:06+11:00"
---

components/core/kest_alloc.h declares tracked heap allocation, allocator initialization/allocation/reallocation/string-copy/free dispatch, LVGL heap wrappers, memory initialization and reporting. kest_allocator contains callbacks, opaque data and flags prohibiting free, reallocation or string copying. All allocation/reallocation sizes are bytes, including typed pool adapters.

print_memory_report counts requested bytes through kest_alloc/kest_realloc and their peak, excluding tracking headers, heap metadata and other allocators. UART heap reports ESP-IDF capability-heap availability; pools reports slot counts, not bytes. These are different measures.

The installed ESP32-P4 ELF gives sizeof(kest_expression)=28, pointer=4 and expression-pool struct=40. Reserving 16,384 nodes allocates 458,752 payload bytes plus 65,536 bytes for the free-pointer ring: 524,288 bytes, excluding allocation headers/mutex and static pool struct. A 789-slot occupancy represents 22,092 live node bytes, excluding reference-name strings and other graph containers. Nodes already occupy a contiguous array; arena allocation alone does not establish faster traversal. An arena proposal must separately budget strings/containers and staging headroom; pool reservation is not its required capacity. Evidence: /tmp/kestrel-expression-storage-types.log, kest_pool.h and kest_alloc.h; installed identity/occupancy has its own owner.

David intends preallocation of known important structs to avoid repeated embedded heap costs. Complete useful integration within the existing architecture.

kest_mem_init reserves typed pools for DSP-resource wrappers, effect descriptors, expressions, parameters, settings, effects and presets, plus sequences outside library builds, installing corresponding global allocators. Initialization/reservation failures return immediately; initialization is not transactional.

Typed constructors request sizeof(type); adapters accept exactly sizeof(X) and reject other sizes. The obsolete object-count flag is removed. Null allocators/missing allocation callbacks fall back to tracked heap with the same byte contract. Tests exercise the production parameter constructor with typed-pool and zero-initialized heap allocators, release and heap cloning; this does not establish arbitrary live strategy substitution.

Both context preset constructors and the sequence constructor use configured allocators, matching file loading/release. Tests exercise two preset slots and one sequence slot for three cycles, exact membership, exhaustion without count changes and reclamation. Carrier HIL exercises ordinary temporary preset creation/deletion; the installed build owner governs current identity/evidence. Sequence creation and every allocator path are not physically qualified.

Runtime constructors and parsed parameter/setting/resource templates use matching typed allocators/releases in source. Variable payloads, names and list nodes remain heap-backed. No pools are reserved for filter/memory/delay/LFO payloads; generic payload release uses kest_free. New pools or a descriptor arena need separate matched-lifetime design and are unselected. One-slot tests qualify exhaustion, retained retirement, rejection and reuse; partial polynomial construction is also checked. 205 host tests and heap-fault probes pass. Parameter/setting integration is installed with stable live reload pools; resource-template integration is installed; three active Swamp reloads retain 46 resource slots and all controls (/tmp/kestrel-resource-template-live.log). Installed-image owner governs exact occupancy and evidence.

Allocation/release strategies must match. Replacing a global allocator while objects remain alive does not transfer ownership; establish lifecycle boundaries before runtime substitution. Missing free/reallocation callbacks use heap fallbacks, so arbitrary partial allocators may be incompatible.

Sources: components/core/kest_alloc.[ch], kest_pool.h, typed constructors and tests/core/kest_pool_test.c, kest_context_test.c, kest_update_test.c. Custom reallocator hazards and callback destination lifetime belong to their focused owners.
