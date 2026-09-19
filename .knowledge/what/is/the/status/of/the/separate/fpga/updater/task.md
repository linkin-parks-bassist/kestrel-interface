---
status: "unverified"
created_at: "2026-09-20T00:07:54+10:00"
scope: "local"
source: "components/fpga/kest_fpga_update.c; components/core/kest_preset.c:475-540; components/core/kest_update.c:147-215"
---
Status: Green

`kest_init_fpga_updater` immediately returns ERR_FEATURE_DISABLED before task creation. The task and wake implementation remain below that return; it calls the no-op `kest_preset_update_fpga` path, so this is not the active update route. The active route is `kest_update_task`. Source: components/fpga/kest_fpga_update.c; components/core/kest_preset.c:475-540; components/core/kest_update.c:147-215
