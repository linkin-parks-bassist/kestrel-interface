---
status: "unverified"
created_at: "2026-09-20T00:02:28+10:00"
scope: "local"
source: "components/core/kest_update.c:9,101,147"
---
Status: Green

`kest_update_task` drains a 16-item FreeRTOS queue into an update list; a preset update first drains pending lists. On each 100 Hz iteration it handles updates, resource changes, generates command list and encoded transfer batch, sends it, drains temporary lists, and delays until the next tick. It is started only under KEST_USE_FREERTOS. Source: components/core/kest_update.c:9,101,147
