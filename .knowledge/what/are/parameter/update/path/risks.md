---
status: green
revised_at: "2026-10-04T05:55:34+11:00"
---

kest_parameter_set has no bounds clamp, and kest_preset_update_fpga is a no-op because of an unconditional early return. These are source observations, not runtime failure reports.

Smooth targets remain in a separate 100 Hz task by David's explicit direction. It resolves IDs every pass and uses the existing atomic parameter-value path, but its effect-mutex attempt is commented out and pointers are retained during a pass and queued for control. Pointer lifetime, queued-ID reuse, queue-initialization spinning, persistence-job ownership and non-UI mutation remain review questions. The smooth-target and pipeline lifecycle owners govern their boundaries. Do not change the task architecture without discussing it with David.

Sources: components/core/kest_parameter.c, kest_preset.c, kest_param_update.c and kest_update.c; David's rollback and thread-safety guidance.
