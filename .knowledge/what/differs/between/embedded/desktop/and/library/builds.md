---
status: "unverified"
created_at: "2026-09-20T00:10:34+10:00"
scope: "local"
source: "main/kest_int.h; desktop/kest_desktop.h; main/kest_lib.h; main/kest_lib_cmph.h; observed make lib output"
---
Status: Green

`main/kest_int.h` enables ESP32 display, SD, FPGA, codec, UI, sequences, global context and FreeRTOS. `desktop/kest_desktop.h` enables UI/FreeRTOS and simulated FPGA, plus representations. Library compilation uses `main/kest_lib.h`/`kest_lib_cmph.h` and omits some sequence/context members; the current library build fails in a preset name-change path that still references sequence. Source: main/kest_int.h; desktop/kest_desktop.h; main/kest_lib.h; main/kest_lib_cmph.h; observed make lib output
