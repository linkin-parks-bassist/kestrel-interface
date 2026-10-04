---
status: green
revised_at: "2026-10-04T06:50:53+11:00"
---

Typed pools preallocate storage for Kestrel's predetermined important structs, reducing repeated malloc work during operation. David identifies this as a central embedded-performance rationale alongside leaving decommissioned objects accessible. Adoption is incomplete: he reports that many allocations still use the system heap. Treat that as unfinished integration, not evidence that the allocation design should be replaced.

`components/core/kest_pool.h` defines declaration and implementation macros for initialization/reservation, obtaining and returning objects, batch obtains, locking and allocator adapters. Pool entries and a pointer FIFO hold reusable objects; optional initialization, deinitialization and copy callbacks customize lifecycle.

David explains that pools were intended in part to prevent access-after-free crashes by leaving decommissioned objects in allocated storage, so stale work can still encounter recently valid data. Returning a slot retains the pool's backing allocation, but invokes any configured deinitializer and makes the slot available for reuse. Separately allocated members can be released and reused slots can hold new objects. Preserve the intended design rather than treating every stale pointer as justification for new lifetime infrastructure; the intention does not establish safety for every cleanup path.

The allocator adapter uses the common byte-size contract: allocation/reallocation accept exactly sizeof(X) and reject other sizes. String copying is prohibited. Reallocation obtains a second slot, copies through the configured function or memcpy, then returns the old slot; exhaustion leaves the old object intact. tests/core/kest_pool_test.c covers this behavior.

Pool reservation and return validation require review when substituting strategies: reservation does not reclaim an existing allocation, and return assumes a valid outstanding pool member. These APIs are not ownership transfer mechanisms.

Sources: components/core/kest_pool.h and David's pool rationale. Allocator dispatch/initialization is owned by what/does/kest-alloc/header/declare.md; persistence direction belongs to how/does/the/file/task/queue/persistence.md.
