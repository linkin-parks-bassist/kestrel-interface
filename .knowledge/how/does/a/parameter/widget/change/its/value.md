---
status: "unverified"
created_at: "2026-09-20T00:06:17+10:00"
scope: "local"
source: "components/ui/kest_parameter_widget.c:327-415"
---
Status: Green

The widget callback reads an LVGL slider or arc position, maps it into current parameter bounds with linear or logarithmic scaling, updates its nominal label, triggers a smooth parameter target, and marks the owning preset unsaved. The same file implements setting widgets with a separate edit/accept path. Source: components/ui/kest_parameter_widget.c:327-415
