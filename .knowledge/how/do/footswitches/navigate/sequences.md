---
status: "unverified"
created_at: "2026-09-20T00:06:18+10:00"
scope: "local"
source: "components/drivers/kest_footswitch.c; components/core/kest_context.c:610-627"
---
Status: Green

GPIO 20 and 21 are configured as pulled-up inputs with interrupt-on-any-edge. The ISR debounces using system milliseconds and logs a footswitch event; context handling maps switch 0 to sequence regress and switch 1 to advance. A separate footswitch task/queue path is also defined. Source: components/drivers/kest_footswitch.c; components/core/kest_context.c:610-627
