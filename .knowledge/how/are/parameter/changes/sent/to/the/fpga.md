---
status: green
revised_at: "2026-10-06T11:33:22+11:00"
---

Separate 100-Hz smoothing advances targets and queues notifications. The 100-Hz control task consumes parameter/scope/preset notifications and latest memory arrivals, computes dependent writes/allocations, then emits commands. David explicitly retains smoothing's task boundary.

parameter_widget_change_cb_inner obtains the effective range, converts the slider/arc position through linear or logarithmic scaling, updates its nominal display and calls kest_parameter_trigger_update. The firmware helper queues an ID/target for smoothing; it does not immediately set the value. The widget marks its preset dirty. kest_parameter_set writes the current value and marks it updated, rejecting non-overridden driven parameters; using it directly bypasses the normal smoothing entry.

A coefficient dependency stages every coefficient of its affected filter/polynomial handle, coalesces duplicates and emits one bank commit. Commit swaps the whole bank; sparse updates lost unchanged coefficients in actual-core replay. Host/core regressions and numerical carrier KTPOLY HIL verify ordinary UI/smoothing/control/SPI behavior. Test and installed-firmware owners govern counts and image identity; the periodic-read owner records scratchpad results/cleanup. Core's bank-update owner governs scope. Maximum-load/overlap pacing and audible transitions remain unqualified.

UI-enabled control ticks hold the recursive UI lock while borrowing model objects, including retries, releasing it before sleep. Smoothing passes also lock model borrows. File jobs save asynchronously without the display/model lock after David's accepted performance rollback; do not reintroduce SD I/O under that lock. File-task and refresh owners govern borrowed save pointers, deferred maintained buffers and background-mutation limits.

Sources: kest_update.c, kest_param_update.c, kest_parameter.c, kest_parameter_widget.c, kest_file_task.c, polynomial fixtures/tests, Core bank-update owner and accepted carrier rollback.
