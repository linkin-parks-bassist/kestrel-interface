---
status: "unverified"
created_at: "2026-09-20T00:04:02+10:00"
scope: "local"
source: "components/core/kest_file_task.c:1-140"
---
Status: Green

A FreeRTOS queue of 16 jobs accepts state values by copy and preset/sequence pointers for later saving. `kest_queue_state_save` skips the first 3000 ms to avoid persisting incompletely initialized state, clones context state, then queues it; other queue functions enqueue pointers. They return ERR_UNINITIALISED before task queue creation or ERR_CURRENTLY_EXHAUSTED when full. Source: components/core/kest_file_task.c:1-140
