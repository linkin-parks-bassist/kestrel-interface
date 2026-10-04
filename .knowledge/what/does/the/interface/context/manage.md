---
status: green
revised_at: "2026-10-04T01:31:14+10:00"
---

kest_context holds preset and sequence collections, working/active/default presets, effect descriptions, input/output gain parameters, optional UI pages and an optional FreeRTOS mutex. Context APIs resolve IDs, switch presets and handle switches. Persistence uses the existing kest_queue_state_save file-task path; the unused representation-based kest_cxt_queue_save_state wrapper and its state fields are removed.

Sources: components/core/kest_context.[ch], kest_file_task.[ch], kest_param_update.c.
