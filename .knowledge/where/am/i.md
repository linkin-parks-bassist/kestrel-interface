---
status: "unverified"
updated_at: "2026-09-20T00:13:24+10:00"
---
Status: Green

Kestrel Interface controls the pedal on ESP32-P4 and has a POSIX desktop demo. `components/core` manages effects, presets, context, updates and state; `parser` reads .eff descriptors; `fpga` encodes and transports commands; `ui` supplies LVGL views; `drivers` integrates SD, touch, codec and footswitches. `main` starts the ESP-IDF app, `desktop` hosts the SDL demo, and `tests` holds C tests. Generated Doxygen docs/html was removed; authored sources and Doxyfile remain.

The `what/` branch contains architecture, declaration and first-party C file maps plus focused behavior and gap leaves; `how/` traces build, parsing, loading and FPGA updates; `why/` records diagnosed failures and design rationale; `where/` locates tests and sources. Read a focused leaf, then source, for consequential edits. The current code includes deliberate bypasses, unfinished functions and build failures; the README is not a complete implementation spec.
