---
status: "unverified"
created_at: "2026-09-20T00:02:28+10:00"
scope: "local"
source: "components/core/kest_init.c"
---
Status: Green

`kest_init` initializes memory and the update task, then conditionally initializes UI/context, FPGA communication and parameter update tasks, SD, and SGTL5000. It creates directories, loads effects/presets/sequences under the UI lock, schedules UI creation, restores saved state/page if available, starts the file task, and initializes footswitches outside desktop builds. SGTL_TEST and no-display builds return early. The return value is the state-load or restore result, so startup can return an error after setting up services. Source: components/core/kest_init.c
