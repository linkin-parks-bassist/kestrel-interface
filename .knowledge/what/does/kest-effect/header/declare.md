---
status: green
revised_at: "2026-10-05T02:51:06+11:00"
---

components/core/kest_effect.h declares initialization/cloning, parameter/settings access, FPGA compilation/updates, scope creation, LFO activation, enable/disable, view initialization and update handlers. Instances store resources, blocks/drivers, scope, parameters/settings, an atomic alive flag and intrusive retirement/SPI-completion fields.

Effect member helpers use configured typed allocators and release unattached members on list failure. Host tests cover exact one-slot membership, initialization, exhaustion and three reuse cycles. Descriptor construction checks parameter/setting/resource clone and driver errors before publication; failed driver-list insertion releases its unattached payload. The unit-test owner governs current coverage; the installed-firmware owner governs carrier identity and qualification limits.

free_effect marks instances/resources for control retirement. kest_effect_free_retired is final cleanup after control/SPI borrowers finish, scheduled on the UI task when enabled. Never-published pipeline construction and staged decoder rollback may use it directly; it is not a general immediate-free API. how/does/a/preset/pipeline/manage/effects.md owns lifetime/release details and remaining scope/direct-caller rollback gaps.

Sources: components/core/kest_effect.[ch], kest_driver.c and tests/core/kest_context_test.c.
