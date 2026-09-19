---
status: "unverified"
created_at: "2026-09-20T00:10:34+10:00"
scope: "local"
source: "main/kest_int.c:16-55; components/core/kest_event.c:88-105"
---
Status: Green

`app_main` initializes printing, seeds random, disables the ESP task watchdog, starts the Waveshare display when enabled, starts the event task, and posts a startup event. `kest_event_handle` consumes that event and calls `kest_init`; startup work is therefore performed by the event task. Source: main/kest_int.c:16-55; components/core/kest_event.c:88-105
