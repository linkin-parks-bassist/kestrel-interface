---
status: "unverified"
created_at: "2026-09-20T00:02:28+10:00"
scope: "local"
source: "components/core/kest_context.c:7"
---
Status: Green

`kest_init_context` clears the active and working preset and collections; creates a named main sequence; clears UI pointers and load flags; initializes input/output gain controls with -24 to +24 dB bounds, IDs under CONTEXT_PRESET_ID, and maximum velocity 0.4; and creates a mutex under KEST_USE_FREERTOS. It returns ERR_NULL_PTR for a null context. Source: components/core/kest_context.c:7
