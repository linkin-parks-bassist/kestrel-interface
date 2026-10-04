---
status: green
revised_at: "2026-10-04T01:31:48+10:00"
---

components/core/kest_state.h declares kest_cxt_clone_state, kest_cxt_restore_state, kest_cxt_enter_previous_current_page, kest_init_state and kest_state_save. kest_state holds input/output gain, the current page identifier and active preset/sequence filenames. FreeRTOS builds declare state_mutex. The include guard is KEST_SETTINGS_H_.

Source: components/core/kest_state.h.
