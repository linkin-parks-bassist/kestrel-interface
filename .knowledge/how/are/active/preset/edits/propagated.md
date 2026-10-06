---
status: green
revised_at: "2026-10-05T22:54:50+11:00"
---

Append/remove/move modifies the preset pipeline and notifies the updater when active; outside KEST_LIBRARY it queues a save. Preset append checks capacity before allocating/publishing, returning NULL on rejection without save/reprogram. The selector explains rejection and preserves its page/preset. Pipeline/selector owners govern checks/tests.

Installed kest_preset_set_active checks capacity before active/pending flags or updater notification. Context activation propagates rejection before deactivating the old preset/changing pointers, and sequence navigation preserves cursors/sequence state on rejection. Direct/context/sequence start/next/previous host tests preserve running state. Preset Play explains capacity rejection; carrier Play rejection retains the running preset, and Add Effect rejection leaves its page/model intact. Physical sequence overflow remains pending under its owner. Programming returns compilation failures before queueing a transfer. Queue-failure rollback and worst-case timing are separate unresolved contracts.

Sources: components/core/kest_preset.c, kest_context.c, kest_sequence.c, components/ui/kest_preset_view.c and kest_effect_select.c.
