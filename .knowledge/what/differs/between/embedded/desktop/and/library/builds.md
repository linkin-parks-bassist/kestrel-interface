---
status: green
revised_at: "2026-10-05T23:39:53+11:00"
---

main/kest_int.h enables ESP32 display, SD, FPGA, codec, UI, sequences, global context and FreeRTOS. desktop/kest_desktop.h enables UI/FreeRTOS and simulated FPGA. Deprecated representation framework, switch and object fields are absent from all configurations.

Library compilation uses main/kest_lib.h/kest_lib_cmph.h; it excludes UI, FreeRTOS, sequence/context services and firmware persistence queue calls. make lib builds a libm-linked shared object with unresolved-symbol rejection. make compile-eff builds the host compiler using production parser, effect constructor and pipeline encoder. Internal-name PARAMETER=VALUE overrides use existing setters. setting.NAME=INTEGER overrides validate bounds and enum membership before complete encoding; parameter-only --update rejects settings. The build/run owner specifies commands and output boundaries.

The shared arithmetic/SVF library consumes these bytes through model/RTL rendering; focused readback/SVF harnesses execute compiled fixtures. Full controller/mixer/SPI and all-resource execution remain unqualified. Desktop/application, C-suite and pinned ESP-IDF builds pass. Desktop dialog interaction qualifies the accepted visual adjustment under its UI owner. These checks do not establish every feature combination.

Sources: main/kest_int.h, desktop/kest_desktop.h, main/kest_lib.h, main/kest_lib_cmph.h, Makefile, tools/compile_eff.c and executed builds/tests.
