---
status: green
revised_at: "2026-09-20T00:02:29+10:00"
---

`kest_updater_handle_preset_update` clears pending state, enters REPROGRAM state, stores the target as active, updates effect positions and adds each effect to the staged program. While in REPROGRAM state, `kest_updater_handle_update` drops other notifications. Sending queues a program batch and returns the updater to READY. Source: components/core/kest_update.c:710,984,1185
