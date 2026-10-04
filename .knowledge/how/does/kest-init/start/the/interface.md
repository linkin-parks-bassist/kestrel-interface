---
status: green
revised_at: "2026-10-04T05:55:34+11:00"
---

kest_init initializes memory, then UI/context/global pages and FPGA communication. Under USE_FPGA it starts the separate parameter-smoothing task; it then starts the control task. Smoothing creates its own target queue inside its task and waits for FPGA boot before its periodic work. Startup next initializes SD/SGTL5000, creates directories, loads effects/presets/sequences under the UI lock, schedules UI creation and restores saved state/page under that lock. It starts the file task and initializes footswitches outside desktop builds. SGTL_TEST and no-display builds return early.

The return value is the state-load or restore result, so startup can return an error after setting up services. Sources: components/core/kest_init.c, kest_param_update.c and kest_update.c.
