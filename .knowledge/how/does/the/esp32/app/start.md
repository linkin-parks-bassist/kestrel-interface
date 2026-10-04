---
status: green
revised_at: "2026-10-04T01:06:57+10:00"
---

app_main initializes printing, seeds random, disables the ESP task watchdog, starts the Waveshare display when enabled, starts the event task and posts the startup event. It then starts the UART diagnostic/UI console; a console-start error is logged rather than aborting the application. kest_event_handle consumes startup and calls kest_init, so main initialization still runs on the event task. Console commands may become available while that startup work is in progress.

Sources: main/kest_int.c, main/kest_console.c, components/core/kest_event.c. Console commands are owned by how/to/control/the/interface/over/uart.md.
