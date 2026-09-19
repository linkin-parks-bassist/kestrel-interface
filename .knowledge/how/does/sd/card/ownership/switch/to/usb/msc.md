---
status: "unverified"
created_at: "2026-09-20T00:06:18+10:00"
scope: "local"
source: "components/drivers/kest_sd.c:61-205"
---
Status: Green

`kest_sd.c` initializes SDMMC through the board pins and power LDO, then configures TinyUSB MSC storage. `kest_sd_mode_msc` takes sd_mutex and moves the mount to USB; `kest_sd_mode_local` moves it back to the app and releases the mutex. The failed USB mount path returns without releasing the mutex in current source. Source: components/drivers/kest_sd.c:61-205
