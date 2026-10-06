---
status: green
revised_at: "2026-10-05T22:54:50+11:00"
---

set_active_preset calls kest_preset_set_active on a non-null target and returns any capacity rejection before altering the old preset, containing sequence or global pointer. Accepted activation marks target active/pending and notifies the updater, returns early if already selected, otherwise positions a containing sequence, marks the previous preset inactive and stores the target in global_cxt. A null target deactivates the previous selection.

set_active_preset_from_sequence propagates the same rejection before changing context; it omits containing-sequence activation and queues a state save after selection. Sequence navigation commits its cursor/active/global sequence changes only after successful preset admission. Capacity tests preserve the previous active flags/pointers/cursors through all these rejection paths. Updater queue-failure rollback is separate and not established by capacity preflight.

Sources: components/core/kest_context.c, kest_preset.c, kest_sequence.c and tests/core/kest_pipeline_test.c. Activation guards are installed; carrier preset Play rejection retains the previous running preset. Physical sequence overflow and queue-failure rollback remain unqualified; installed identity has its own owner.
