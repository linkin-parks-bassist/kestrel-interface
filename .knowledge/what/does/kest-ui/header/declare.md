---
status: green
revised_at: "2026-10-04T05:19:49+11:00"
---

components/ui/kest_ui.h declares page creation/configuration/navigation, panels, global pages, UI locking and asynchronous calls. kest_ui_async_call(callback, data) always takes the existing recursive UI lock around LVGL enqueue, then releases it. Its void-function variant delegates to the same helper. It returns NO_ERROR only when LVGL accepts the callback; a UI-lock refusal or allocation failure returns an error. Existing callers can ignore that result; retirement uses it to retain an owner until cleanup is actually scheduled. kest_ui_async_call_void retains its existing void interface.

kest_ui_page_cancel_async(page) cancels queued direct/forward/backward page-entry callbacks on the UI task before destroying an effect page. It does not establish general page ownership or cancel unrelated callbacks.

Sources: components/ui/kest_ui.[ch]; how/does/the/ui/page/lifecycle/work.md owns page behavior.
