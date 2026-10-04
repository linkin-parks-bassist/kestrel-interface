---
status: green
revised_at: "2026-10-04T05:18:36+11:00"
---

The widget callback reads an LVGL slider or arc position, maps it into current parameter bounds with linear or logarithmic scaling, updates its nominal label, triggers a smooth parameter target, and marks the owning preset unsaved. Setting widgets use a separate edit/accept path.

gut_parameter_widget runs on the UI task: it cancels queued refreshes and its animation timer, clears param->pw when it points to this widget, deletes its container and clears UI pointers. It preserves embedded storage and configuration; creation restores the parameter backlink. free_parameter_widget performs that cleanup and frees heap widget storage. gut_setting_widget releases saved field text and the container and clears UI pointers; free_setting_widget also frees heap storage. Setting-widget initialization explicitly clears the entire struct, including previously uninitialized container/label/pad fields.

Regressions cover timer/callback borrowers and embedded settings widgets whose containers live on backstage. The effect view stays alive until control/SPI finish and final UI cleanup is scheduled; the pipeline lifecycle owner governs that ordering.

Sources: components/ui/kest_parameter_widget.c and tests/ui/kest_parameter_widget_test.c.
