<p align="center">
  <img src="docs/resources/image.png" alt="Demo Screenshots" width="100%">
</p>

# Kestrel Interface

This is the control system, graphical user interface and compiler for Kestrel. It uses FreeRTOS and LVGL and allows users to create, edit, manage, apply and sequence presets using effects from a local library of effects stored in text files on the local SD card. 

## Features

- Live, real-time smoothed parameter control
- Apply effects in any combination and any order
- Persistent state: resumes where left off on restart
- Preset management; create, save, edit and sequence presets
- Effects stored in plain text on SD card; new effects can easily be created and shared
- Symbolic math engine with range estimation for fixed-point format control
- High performance, lightweight UI: framerates over 100fps at 720p on ESP32-P4
- Input transmitted to FPGA in milliseconds

## Components

- Effect compiler
- FPGA core control engine
- Effect/preset/sequence engine
- Parameter control subsystem
- LVGL-based GUI framework
- Symbolic math engine
- File system

## Effect Descriptors

The Kestrel interface includes a parser and assembler for .eff files. These "effect descriptor" files are simple, small files containing metadata and descriptions of parameters and resource requirements as well as assembly code for the Kestrel core, in an intuitive syntax. For example, here is a biquad low-pass filter.

```
v1.0

.INFO

name: "Low Pass Filter"
cname: "example_low_pass_filter"

.PARAMETERS

cutoff: (name: "Cutoff",
         default: 1000,
         min: 60,
         max: sample_rate / 2 - 1,
         units: "Hz",
         scale = "logarithmic")
Q: (name: "Resonance", default: 1 / sqrt(2), min: 0.1, max: 3)

.DEFS

omega: 2 * pi * cutoff / sample_rate
alpha: sin(omega) / (2 * Q)

.RESOURCES

x1: (type: "mem")
x2: (type: "mem")
y1: (type: "mem")
y2: (type: "mem")

.CODE

mem_read c1 $x1
mem_read c2 $x2
mem_read c3 $y1
mem_read c4 $y2

macz [(1/2) * (1 - cos(omega)) / (1 + alpha)] c0
mac  [        (1 - cos(omega)) / (1 + alpha)] c1
mac  [(1/2) * (1 - cos(omega)) / (1 + alpha)] c2
mac  [        (2 * cos(omega)) / (1 + alpha)] c3
mac  [             (alpha - 1) / (1 + alpha)] c4

mem_write $x2 c1
mem_write $x1 c0
mem_write $y2 c3

mov_acc c0

mem_write $y1 c0
```

(Note: the filter engine allows for filters to be implemented more succinctly, and with better performance, this is just for demonstration).

All files in /sdcard/eff/ are read, parsed and assembled at startup, and used to populate the UI effect list. At runtime, effects are encoded on-the-fly using real-time parameter values and pipeline configurations and transmitted to the FPGA as programming commands over SPI.
Additionally, there are hooks for the UI framework in the .eff parser. For instance, the field "widget_type" can control whether a parameter is presented as a dial or a slider.

#### Features
- Simple assembly code with friendly syntax
- FPGA register values computed just-in-time from real-time
- Inline math expressions
- Delay buffers and scratchpad memory with simple assembly interface
- Variable, dependent parameter bounds
- Support for continuous parameters and discrete "settings"
- UI generated from file
- Definable parameter widget type and appearance

#### Planned features
- Include the instructions for accessing sin, tanh, etc look-up tables
- Add biquad and general IIR filter instructions
- Named expressions and custom function definition
- More UI control; widget size, placement


## Getting Started

### ESP32

The current carrier firmware uses the Waveshare ESP32-P4-nano board support package, touch-LCD-5A and SGTL5000 audio codec. Its verified build baseline is **ESP-IDF v5.3.3**. The manifest pins the SDK and direct components; retain `dependencies.lock` to preserve transitive versions. The integrated Rev-B board requires a separate SDK, silicon-revision and driver migration.

Install the exact SDK release and its ESP32-P4 tools:

```bash
git clone --branch v5.3.3 --depth 1 --shallow-submodules --recursive https://github.com/espressif/esp-idf.git /path/to/esp-idf-v5.3.3
cd /path/to/esp-idf-v5.3.3
./install.sh esp32p4
```

Use Bash, source that checkout's environment, then build from this repository's root. CMake and Ninja must be available; if they are absent, install `cmake==3.30.9` and `ninja==1.11.1.4` into the SDK Python environment after exporting it.

```bash
source /path/to/esp-idf-v5.3.3/export.sh
idf.py --version  # ESP-IDF v5.3.3
cd /path/to/kestrel_interface
idf.py build
```

The application image is `build/kestrel-interface.bin`; a successful build does not verify device operation. To flash a connected carrier:

```bash
idf.py -p PORT flash
```

The firmware starts a UART0 diagnostic console at 115200 baud. Keep one connection
open during a test session; connection-state changes can reboot the carrier:

```bash
python3 tools/uart_console.py --port PORT --log /path/to/new-capture.log
```

Use `help`, `info`, `heap`, `uptime` and `ui-tree` for inspection. `tap X Y` and
`touch down X Y`, `touch move X Y`, `touch up` feed a separate LVGL pointer through
normal UI input handling. `fpga-read ADDRESS` queues an asynchronous memory read.
`fpga-read32 ADDRESS` reads a word-aligned 24-bit byte address and prints four bytes
as a hexadecimal word. With the matching FPGA image, address 0 is `0x4b455354`
("KEST") and address 4 is the build mask: filter/polynomial/SVF in bits 0/1/2.
`fpga-status` reads and decodes the status byte through the SPI task.
`dsp` lists the active preset's effects and DSP resource addresses.
`eff-file list`, `eff-file read NAME.eff`, `eff-file move OLD.eff NEW.eff` and
`eff-file delete NAME.eff` operate in the SD effect directory. Upload with
`eff-file write NAME.eff OFFSET HEX`: start at offset 0, then append consecutive
chunks (up to 128 decoded bytes each, within the 256-character command limit).
Wait for each command's result before sending the next chunk; rapid batches can
overrun the UART input buffer during SD writes. Verify the published bytes with
`eff-file read`.
Use 8.3 filenames (up to eight characters before `.eff`); extension matching is
case-insensitive. Writes go to `tmp/NAME.tmp` inside the effect directory; startup
skips that subdirectory. `eff-file publish NAME.eff` renames the completed
file into place. FAT refuses an existing destination; delete an old file explicitly
before replacing it. Reads return hex chunks. Commands require local SD ownership
and reject paths outside that directory. Existing loaded effects are unchanged;
live discovery/refresh remains planned. The host utility requires pyserial.

### Desktop 

The repo includes a POSIX desktop interface demo. Install SDL2 development tooling
(`libsdl2-dev` and `pkg-config` on Ubuntu), then build with GNU Make:

```bash
# make
```

in the repo root directory. To run it, run

```bash
# ./kest
```

For repeatable UI inspection using a temporary copy of `sdcard`:

```bash
python3 tools/desktop_ui.py --headless --script tools/ui_scripts/danger_button.txt --output /tmp/kestrel-ui-inspection
```

Choose a fresh output directory. The runner writes `control.log` and BMP screenshots,
plus PNG copies when Pillow is available. Omit `--headless` for a visible window.
Scripts accept `wait MS`, `click X Y`, `tree`, `screenshot NAME.bmp` and `quit`;
the current desktop coordinates are 600×1024. `tree` reports visible LVGL objects,
labels and bounds so subsequent clicks can be chosen from the actual UI.
The sample opens the erase confirmation without confirming it. This exercises UI
code with simulated FPGA communications; it does not simulate DSP audio.

Run the C suite with `make tests && ./kest_tests`.

There is an additional makefile target to compile the non-GUI/hardware components (preset library, .eff assembler) as a shared object library. To build the library,

```bash
# make lib
```

To compile a single effect into a binary SPI programming body without installing
the library:

```bash
make compile-eff
bin/lib/compile_eff tests/fixtures/readback.eff /tmp/readback.bin
# Optional parameter overrides use the descriptor's internal names:
bin/lib/compile_eff ../effects/SVFHP.EFF /tmp/highpass.bin cutoff=2000 Q=0.7
```

The executable finds `libkest.so` beside itself. It uses the production parser,
effect constructor and pipeline encoder. The output includes the tail-enable
command but excludes the begin/end-program framing supplied by the transport.
Compilation alone does not verify DSP execution or audio results.

and to install the libkest.so to /usr/lib/ and the headers to /usr/include/libkest, run 

```bash
# make lib_install
```
as root. Then you can #include <libkest/kest_lib.h> and link with "-lkest" (if using ld)



## Repository Structure

The repo is structured as an ESP-IDF project currently. Future plans include supporting STM32 targets.

```
/docs           Documentation
/components
    /core       Core engine logic; handling presets, sequences, etc
    /parser     Parser for .eff files
    /fpga       Kestrel core driver/encoding
    /drivers    Other hardware drivers
    /ui         LVGL GUI code
/main           Headers and 'main.c' for ESP32
/desktop        Headers and 'main.c' for desktop demo
```

## Future Plans
- Support for STM32h743
- Better coverage for pre-allocation and memory pools
- Dependency trees for math expressions (for optimisation and cycle detection)
- Support for (not-yet-implemented) biquad and IIR filter modules on FPGA
- Various optimisations


## Contact

I'd love to hear from you! email: davidjfarrell96@gmail.com

## License

GNU GPL 3.0
