---
status: green
revised_at: "2026-10-04T05:18:35+11:00"
---

components/ui/kest_effect_settings.c initializes/configures an effect settings page and creates/refreshes its band controls. The page owns a panel, dynamically copied title and data containing embedded LP/HP parameter widgets and a band-mode setting widget. Configuration replaces the owned title and reports allocation failure.

Both destruction hooks are installed. free_effect_settings_page_ui cancels queued page entry, cleans embedded widgets without freeing their storage, deletes their containers even when reparented onto the global backstage object, clears parameter backlinks and timers, deletes the screen and resets UI state. Repeated UI cleanup is safe; widget creation restores parameter backlinks. effect_settings_page_free_all additionally releases the title, panel, data and page. Effect-view destruction invokes this complete cleanup on the UI task.

The C regression exercises backstage cutoff containers, a pending refresh and timer, repeated UI cleanup and complete page release. Sources: components/ui/kest_effect_settings.c, kest_effect_view.c, kest_parameter_widget.c and tests/ui/kest_parameter_widget_test.c. The pipeline lifecycle owner governs effect retirement.
