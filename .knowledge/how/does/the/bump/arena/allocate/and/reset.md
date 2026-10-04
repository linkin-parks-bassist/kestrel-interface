---
status: green
revised_at: "2026-09-20T00:07:11+10:00"
---

`kest_bump_arena_init` allocates an aligned backing block and installs alloc/realloc/free wrappers. Allocation rounds up to KEST_BUMP_ARENA_ALLOC_ALIGN and advances a position; free is a no-op; reset rewinds position to zero, invalidating prior allocations. Source: components/core/kest_bump_arena.c:9-139
