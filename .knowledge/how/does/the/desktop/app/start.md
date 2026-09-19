---
status: "unverified"
created_at: "2026-09-20T00:10:34+10:00"
scope: "local"
source: "desktop/kest_desktop.c:1-150"
---
Status: Green

`desktop/kest_desktop.c` supplies SD stubs and a global context, initializes SDL video, an LVGL display/flush callback and mouse input, starts the event task, posts startup, and pumps SDL/LVGL events in a loop. It sets SDL_VIDEODRIVER to x11 if unset because of a noted Wayland compatibility problem. Source: desktop/kest_desktop.c:1-150
