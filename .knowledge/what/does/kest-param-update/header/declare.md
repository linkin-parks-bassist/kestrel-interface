---
status: green
revised_at: "2026-10-04T05:55:35+11:00"
---

components/core/kest_param_update.h declares kest_init_parameter_updater, kest_param_update_task and kest_parameter_trigger_update, with MAX_CONCURRENT_PARAM_UPDATES=16. kest_parameter_update contains a parameter ID, parameter and effect pointers, float target and integer send flag. Initialization starts the separate smoothing task.

Source: components/core/kest_param_update.h. how/are/smooth/parameter/targets/applied.md owns behavior, scheduling direction and lifetime limits.
