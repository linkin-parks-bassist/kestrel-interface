---
status: "unverified"
created_at: "2026-09-20T00:02:28+10:00"
scope: "local"
source: "components/core/kest_context.c:475"
---
Status: Green

`set_active_preset` first calls `kest_preset_set_active` on a non-null target, returns early if it is already active, activates a containing sequence at that preset when present, marks the previous active preset inactive, and stores the new pointer in global_cxt. `set_active_preset_from_sequence` omits the sequence activation and queues a state save. Source: components/core/kest_context.c:475
