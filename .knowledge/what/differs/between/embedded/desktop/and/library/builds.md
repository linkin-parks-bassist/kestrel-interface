---
status: green
revised_at: "2026-10-04T11:15:32+11:00"
---

main/kest_int.h enables ESP32 display, SD, FPGA, codec, UI, sequences, global context and FreeRTOS. desktop/kest_desktop.h enables UI/FreeRTOS and simulated FPGA. The deprecated representation framework, feature switch and object fields are absent from all configurations.

Library compilation uses main/kest_lib.h/kest_lib_cmph.h and excludes UI, FreeRTOS, sequence/context services and firmware persistence queue calls. make lib builds a shared object linked to libm, with unresolved-symbol rejection; make compile-eff builds the standalone host compiler against that object. It uses the production parser, effect constructor and pipeline encoder. The compiler also accepts PARAMETER=VALUE arguments after its input/output paths, using existing effect parameter setters before encoding; nonfinite, malformed and unknown overrides are rejected. The shared arithmetic/SVF effect library consumes these bytes through a model/RTL rendering loop. The compiled readback and SVF fixtures execute in focused Core C++ harness modes. Full controller/mixer/SPI and all-resource execution remain unqualified.

Desktop/application builds and the C suite pass; pinned ESP-IDF builds also pass. Desktop dialog interaction qualifies the accepted visual adjustment, with its evidence owned by the button/UI procedure. These checks do not establish every feature combination.

Sources: main/kest_int.h, desktop/kest_desktop.h, main/kest_lib.h, main/kest_lib_cmph.h, Makefile, tools/compile_eff.c and executed builds/tests.
