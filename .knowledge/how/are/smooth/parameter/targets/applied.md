---
status: "unverified"
created_at: "2026-09-20T00:03:22+10:00"
scope: "local"
source: "components/core/kest_param_update.c:1-320"
---
Status: Green

Under KEST_USE_FREERTOS, a 100 Hz task accepts parameter ID/target updates, coalesces targets for the same ID, resolves the parameter/effect each tick, and moves the current value toward target by a velocity-limited step. Linear scaling limits absolute change; logarithmic scaling multiplies the limit by current parameter value. The task delays startup by FPGA_BOOT_MS. Without FreeRTOS, `kest_parameter_trigger_update` assigns the target directly. Source: components/core/kest_param_update.c:1-320
