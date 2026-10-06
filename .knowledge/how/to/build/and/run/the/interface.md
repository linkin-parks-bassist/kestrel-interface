---
status: green
revised_at: "2026-10-06T15:57:33+11:00"
---

Run commands from kestrel_interface.

Carrier firmware uses pinned ESP-IDF 5.3.3 for esp32p4. On this workstation:

```bash
source /home/david/tools/esp-idf-v5.3.3/export.sh
idf.py build
```

Image: build/kestrel-interface.bin. Global:where/is/esp-idf/installed.md owns SDK setup; README.md gives portable installation. Preserve sdkconfig, main/idf_component.yml and dependencies.lock. CMake optimizes LVGL alone with -O2; firmware retains -Og. sdkconfig enables LVGL's existing PPA backend for opaque rectangular fills; images remain software. The display-init owner governs buffering/performance. Carrier uses 1000-Hz ticks, 360-MHz CPU, 200-MHz PSRAM and LVGL 1-ms refresh. Pinned P4 Kconfig gates 200-MHz PSRAM on CONFIG_IDF_EXPERIMENTAL_FEATURES=y; fresh probe defaults silently fall back to 20 MHz without it. Verify resolved sdkconfig. Rev-B migration remains separate.

For the isolated rounded-fill experiment, copy sdkconfig to /tmp/kestrel-rounded-ppa-sdkconfig and build with idf.py -B /tmp/kestrel-rounded-ppa-build -D SDKCONFIG=/tmp/kestrel-rounded-ppa-sdkconfig -D KEST_ROUNDED_FILL_PREVIEW=ON build. For the corner-split candidate, use separate /tmp/kestrel-rounded-corners-{sdkconfig,build} paths and additionally pass -D KEST_ROUNDED_CORNERS_PREVIEW=ON. Both options default OFF; normal build/ remains separate. Add -D KEST_DRAW_PROFILE_PREVIEW=ON only for draw-path diagnosis with both previews; how/to/profile/carrier/drawing/paths.md owns the counters and interpretation. KEST_ROUNDED_CACHE_PREVIEW=ON additionally tests pinned-driver cache handling; its cache owner governs qualification. All preview options default OFF and are cached: explicitly disable diagnostic/cache variants when reusing a build for baseline comparison. Display/installed-image owners govern results and qualification.

Close UART before flashing:

```bash
idf.py -p /dev/serial/by-id/usb-1a86_USB_Single_Serial_5B5F091047-if00 flash
python3 tools/uart_console.py --port /dev/serial/by-id/usb-1a86_USB_Single_Serial_5B5F091047-if00 --log /tmp/new-kestrel-console.log
```

Wait for hash-verified flash before opening the 115200-baud console. Firmware boots automatically; help/info inspect it. Keep one connection during HIL because opening can reset the carrier. Re-enumerate absent ports. UART/installed-image owners govern commands, identity and boot evidence.

POSIX desktop needs SDL2 development tooling (Ubuntu libsdl2-dev and pkg-config):

```bash
make
./kest
```

FPGA communications are simulated; desktop does not run DSP audio. Desktop UI owner governs interaction and isolated SD copies.

C tests:

```bash
make tests && ./kest_tests
make test-parser-allocation
```

Test-location and coverage owners govern details.

Host library/compiler:

```bash
make lib
make compile-eff
bin/lib/compile_eff tests/fixtures/readback.eff /tmp/readback.bin
bin/lib/compile_eff ../effects/experimental/RHYTHM.EFF /tmp/rhythm.bin setting.tempo=90 setting.division=24
```

Outputs: bin/lib/libkest.so and adjacent bin/lib/compile_eff. Compiler accepts INPUT.eff OUTPUT.bin [PARAMETER=VALUE ...] [setting.NAME=INTEGER ...] using internal names. Setting overrides require integral int-representable values inside declared bounds; enums require an existing choice. Normal output is a programming body with tail-enable, excluding transport framing. --update UPDATE.bin before parameter overrides writes default programming plus a separate production live-update body; settings are rejected in this mode because they require complete reprogramming. Compilation alone does not verify DSP/audio. make lib_install installs library/headers into system directories and requires write access; local use does not need it.

Sources: Makefile, README.md, tools/compile_eff.c, SDK/build configuration and carrier procedure. Compiler setting boundary/rejection probes passed; RHYTHM's owner governs model/RTL qualification.
