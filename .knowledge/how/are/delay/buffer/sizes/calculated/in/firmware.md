---
status: green
revised_at: "2026-10-05T23:35:09+11:00"
---

Initial encoding and preset-rebuild updates evaluate delay/size expressions against the effect instance scope. They ceil each expression before multiplying by its unit conversion: milliseconds use sample_rate/1000, seconds use sample_rate, samples use 1. Size uses the same units as delay; use delay_samples for word-based sizing.

Absent size defaults to base_delay+4. Smaller explicit size is raised to that minimum. Padding adds 4-(size%4), advancing to the next multiple of four even when already aligned. Allocation sends padded size and base delay separately.

The tempo fixture qualifies repeated rebuilds using current instance settings: 120-BPM quarter note gives 22050/22056 delay/allocated words; 90 quarter gives 29400/29408; 60 dotted eighth gives 33075/33080; 30 quarter gives 88200/88208; restoring defaults gives the initial allocation. Descriptor defaults remain unchanged. The updater ignores incoming updates while in REPROGRAM; the regression models return to READY between completed batches. Physical resize timing, transitions and SDRAM admission remain unqualified.

Sources: components/fpga/kest_fpga_encoding.c, components/core/kest_update.c, tests/fixtures/tempo-delay.eff, tests/core/kest_update_test.c and /tmp/kestrel-setting-rebuild-tests.log (201 tests).
