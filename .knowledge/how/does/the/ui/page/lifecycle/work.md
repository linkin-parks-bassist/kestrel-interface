---
status: "unverified"
created_at: "2026-09-20T00:06:18+10:00"
scope: "local"
source: "components/ui/kest_ui.c:77-450"
---
Status: Green

`kest_ui.c` initializes/configures global pages and lazily creates page UI on first entry. Entry refreshes and invokes page callbacks, loads the LVGL screen, sets current_page and logs an event; forward/backward variants animate screen transitions. The file also builds common panels, keyboard and layout containers. Source: components/ui/kest_ui.c:77-450
