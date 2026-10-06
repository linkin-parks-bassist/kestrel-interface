---
status: green
revised_at: "2026-10-06T18:15:36+11:00"
---

Run commands from kestrel_interface.

Carrier uses pinned ESP-IDF 5.3.3 for esp32p4. On this workstation:

```bash
source /home/david/tools/esp-idf-v5.3.3/export.sh
idf.py build
```

Image: build/kestrel-interface.bin. Global:where/is/esp-idf/installed.md owns SDK setup; README gives portable installation. Preserve sdkconfig, main/idf_component.yml and dependencies.lock. CMake optimizes LVGL with -O2; firmware retains -Og. LVGL's PPA backend handles opaque rectangular fills; images remain software. Display-init owns buffering/performance. Carrier uses 1000-Hz ticks, 360-MHz CPU, 200-MHz PSRAM and 1-ms LVGL refresh. P4 Kconfig gates 200-MHz PSRAM on CONFIG_IDF_EXPERIMENTAL_FEATURES=y; fresh probe defaults otherwise fall back to 20 MHz. Verify resolved configuration. Rev-B migration is separate.

Rounded-fill experiments use isolated SDKCONFIG/build paths. Copy sdkconfig to /tmp/kestrel-rounded-ppa-sdkconfig; run idf.py -B /tmp/kestrel-rounded-ppa-build -D SDKCONFIG=/tmp/kestrel-rounded-ppa-sdkconfig -D KEST_ROUNDED_FILL_PREVIEW=ON build. Corner candidates use separate /tmp/kestrel-rounded-corners-{sdkconfig,build} paths plus KEST_ROUNDED_CORNERS_PREVIEW=ON. KEST_DRAW_PROFILE_PREVIEW=ON enables diagnosis with both previews; profiling owns counters/interpretation. KEST_ROUNDED_CACHE_PREVIEW=ON tests pinned-driver cache handling; cache/display/installed-image owners govern qualification. Options default OFF but persist in CMake cache: explicitly disable variants for baseline builds. Normal build/ remains separate.

Close UART before flashing:

```bash
idf.py -p /dev/serial/by-id/usb-1a86_USB_Single_Serial_5B5F091047-if00 flash
python3 tools/uart_console.py --port /dev/serial/by-id/usb-1a86_USB_Single_Serial_5B5F091047-if00 --log /tmp/new-kestrel-console.log
```

Wait for hash-verified flash before the 115200-baud console. Firmware boots automatically; help/info inspect it. Keep one HIL connection because opening can reset the carrier. Re-enumerate absent ports. UART/installed-image owners govern commands, identity and boot evidence.

Desktop needs SDL2 development tooling (Ubuntu libsdl2-dev/pkg-config):

```bash
make
./kest
```

FPGA communications are simulated; desktop does not run DSP audio. Desktop UI owns interaction/isolated SD copies. Desktop object rules depend on app_hdrs, including kest_context.h: changing context layout must rebuild desktop/kest_desktop.c, which defines global_cxt. A stale object reserved 0x540 bytes against the merged 0x5c8-byte context and crashed in SDL startup. Rebuilding with header dependencies passes the filter smoke script and four unchanged captures; /tmp/kestrel-mainline-desktop-result.json binds diagnostics. This is desktop build qualification, not physical firmware acceptance.

C tests:

```bash
make tests && ./kest_tests
make test-parser-allocation
```

Coverage/location owners govern details.

Host library/compiler:

```bash
make lib
make compile-eff
bin/lib/compile_eff tests/fixtures/readback.eff /tmp/readback.bin
bin/lib/compile_eff ../effects/experimental/RHYTHM.EFF /tmp/rhythm.bin setting.tempo=90 setting.division=24
```

Outputs: bin/lib/libkest.so and adjacent compile_eff. Syntax: INPUT.eff OUTPUT.bin [PARAMETER=VALUE ...] [setting.NAME=INTEGER ...], using internal names. Settings require integral int-representable values within bounds; enums require an existing choice. Output is a programming body with tail-enable, excluding transport framing. --update UPDATE.bin before overrides writes default programming plus a production live-update body; settings are rejected because they require reprogramming. Compilation does not verify DSP/audio. make lib_install writes system library/headers and requires permission; local use needs no installation.

Sources: Makefile, README, compiler, SDK/build configuration and carrier procedure. Setting boundary/rejection probes pass; RHYTHM owns model/RTL qualification.
