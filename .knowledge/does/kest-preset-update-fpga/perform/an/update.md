---
status: "unverified"
created_at: "2026-09-20T00:03:23+10:00"
scope: "local"
source: "components/core/kest_preset.c:475-540; components/core/kest_update.c"
---
Status: Green

No. `kest_preset_update_fpga` immediately returns NO_ERROR before its locking and `kest_pipeline_update_fpga` code, so that code is unreachable. The active-update wrapper consequently reports success without performing that update path. The separate updater task handles preset reprogram notifications. Source: components/core/kest_preset.c:475-540; components/core/kest_update.c
