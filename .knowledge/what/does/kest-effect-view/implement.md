---
status: green
revised_at: "2026-10-04T05:18:36+11:00"
---

components/ui/kest_effect_view.c creates an effect page, configures parameter/setting controls and settings navigation, and implements entry/refresh and destruction. Initialization propagates failure from its nested settings-page initializer.

free_effect_view runs on the UI task for retirement: it cancels queued page entry, destroys parameter/setting widget lists, fully releases the nested settings page, frees view data, deletes the screen and frees the panel/page. Widget cleanup cancels refreshes/timers, clears parameter backlinks and deletes containers, including backstage containers. Final owner reclamation is coordinated by the control loop; how/does/a/preset/pipeline/manage/effects.md owns that boundary. Constructor rollback remains separate unfinished work.

Sources: components/ui/kest_effect_view.c, kest_effect_settings.c and kest_parameter_widget.c.
