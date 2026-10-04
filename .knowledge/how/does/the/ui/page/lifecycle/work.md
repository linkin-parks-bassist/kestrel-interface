---
status: green
revised_at: "2026-10-04T05:19:49+11:00"
---

kest_ui.c initializes/configures global pages and lazily creates page UI on first entry. Entry refreshes and invokes page callbacks, loads the LVGL screen, sets current_page and logs an event; forward/backward variants animate screen transitions. It also builds common panels, keyboard and layout containers.

Asynchronous entry borrows the target page. kest_ui_page_cancel_async cancels its queued direct/forward/backward entry callbacks on the UI task before effect-page destruction. Background enqueue takes the existing recursive UI lock, serializing LVGL timer allocation with the UI handler; UI callbacks can re-enter that lock. kest_ui_async_call reports acceptance/lock/allocation errors, allowing the control loop to retain a retired effect if final UI cleanup could not be queued. There is no blanket ownership guarantee for arbitrary pages or callbacks.

Source: components/ui/kest_ui.[ch]; effect-resource retirement belongs to how/does/a/preset/pipeline/manage/effects.md.
