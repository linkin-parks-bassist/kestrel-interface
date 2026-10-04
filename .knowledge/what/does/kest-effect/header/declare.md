---
status: green
revised_at: "2026-10-04T05:03:02+11:00"
---

components/core/kest_effect.h declares instance initialization/cloning, parameter/settings access, FPGA compilation/updates, scope creation, LFO activation, enable/disable, view initialization and update handlers. The instance stores resources, blocks/drivers, scope, parameters/settings, an atomic alive flag and intrusive retirement/SPI-completion fields.

free_effect marks an instance and its resource payloads for control-loop retirement. kest_effect_free_retired(void *) is the final cleanup callback, scheduled on the UI task when enabled after control/SPI borrowers have finished. It is not a general immediate-free API. how/does/a/preset/pipeline/manage/effects.md owns lifetime and release details.

Source: components/core/kest_effect.[ch].
