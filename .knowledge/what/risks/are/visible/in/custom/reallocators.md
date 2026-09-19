---
status: "unverified"
updated_at: "2026-09-20T00:07:24+10:00"
---
Status: Green

`kest_bump_arena_realloc` allocates a new span and copies the requested new size without tracking the old allocation length, so growing can read beyond the old allocation. `kest_realloc` uses unsigned size arithmetic for its accounting delta; while modular addition can yield the correct total for an ordinary shrink, this should be tested if changing the accounting implementation rather than assumed broken. Source: components/core/kest_bump_arena.c:100-119; components/core/kest_alloc.c:42-70.