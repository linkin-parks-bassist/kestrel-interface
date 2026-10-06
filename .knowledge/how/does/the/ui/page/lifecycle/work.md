---
status: green
revised_at: "2026-10-06T16:15:02+11:00"
---

kest_ui.c initializes/configures global pages and lazily creates page UI on first entry. Entry refreshes and invokes page callbacks, loads the LVGL screen, sets current_page and logs an event; forward/backward variants animate screen transitions. It also builds common panels, keyboard and layout containers.

Asynchronous entry borrows the target page. kest_ui_page_cancel_async cancels its queued direct/forward/backward entry callbacks on the UI task before effect-page destruction. Background enqueue takes the existing recursive UI lock, serializing LVGL timer allocation with the UI handler; UI callbacks can re-enter that lock. kest_ui_async_call reports acceptance/lock/allocation errors, allowing the control loop to retain a retired effect if final UI cleanup could not be queued. There is no blanket ownership guarantee for arbitrary pages or callbacks.

Boot restoration resolves only page types handled by kest_page_id_find_page; the effect selector is unsupported and returns NULL. kest_cxt_enter_previous_current_page queues that result, so automation must inspect the actual screen rather than assume the previous selector survived reboot. A tap on the Presets footer plus creates an empty preset; it is not Add Effect navigation.

Source: components/ui/kest_ui.[ch], kest_page_id.c and components/core/kest_state.c; effect-resource retirement belongs to how/does/a/preset/pipeline/manage/effects.md.
