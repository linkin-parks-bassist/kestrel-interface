---
status: green
revised_at: "2026-10-04T05:18:37+11:00"
---

components/ui/kest_parameter_widget.h declares parameter/setting widget types, configuration, creation, refresh and value editing APIs, including kest_parameter_widget_refresh_async and its wrapper. KEST_PARAM_WIDGET_MAX_REFRESH_HZ and KEST_PARAM_WIDGET_MIN_REFRESH_MS govern refresh timing.

gut_parameter_widget and gut_setting_widget release UI-owned contents while preserving embedded widget storage; free_parameter_widget and free_setting_widget additionally release heap widget storage. The change/value owner describes cancellation, container ownership and parameter backlinks.

Source: components/ui/kest_parameter_widget.h.
