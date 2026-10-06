---
status: green
revised_at: "2026-10-06T00:12:25+11:00"
---

Parameter arc/slider callbacks map positions through linear/logarithmic bounds, update nominal labels, queue smoothing and mark the preset unsaved. Numeric/dropdown settings share acceptance: unchanged values do nothing; changed values retain old_value, mark updated/unsaved and queue an active-preset setting event for full reprogramming. Inactive edits only mark dirty. LVGL event/queue regressions and RHYTHM carrier checks qualify this path; physical delay timing remains unqualified.

Integer fields accept a whole signed decimal int, including an optional + or − sign. Empty/sign-only input, whitespace, decimals, junk and overflow reject without changing the setting or dirty state. Rejection restores the bare old value for retry with the keyboard open. Valid values retain existing declared-bound clamping; acceptance restores units and closes the keyboard. Parsing borrows textarea text without allocating/truncating it. tests/ui/kest_parameter_widget_test.c covers both int endpoints, malformed inputs, retry text and clamping; 202 host tests pass (/tmp/kestrel-integer-field-tests.log). Carrier checks reject 999999999999 at Tempo 120, restore bare 120 with keyboard open, accept retry 999 clamped to 300, clamp 1 to 30 and restore/save 120. Swamp/pools/status return to baseline (/tmp/kestrel-integer-boundaries-live.log). Decimal rejection also passes; arbitrary malformed text and signed endpoints remain host-qualified.

Labels/positions use pw->nominal_value, distinct from smoothed values. UART parameter-target does not update nominal state: Swamp DSP values converge while labels retain defaults, even after reopening (/tmp/kestrel-swamp-live-ui-2.log). Observe dsp for external targets. Normal arc/slider edits update label/target together. External-target label synchronization remains an unimplemented usability repair.

gut_parameter_widget cancels queued refreshes/timer, clears param->pw if matching, deletes its container and clears UI pointers while preserving embedded storage/configuration; creation restores the backlink. free_parameter_widget also frees heap storage. gut_setting_widget releases saved text/container and clears UI pointers; free_setting_widget also frees storage. Setting initialization clears the entire struct.

Regressions cover timer/callback borrowers and embedded backstage settings widgets. Effect views remain alive until control/SPI finish and scheduled UI cleanup; pipeline lifecycle owner governs ordering. Sources: components/ui/kest_parameter_widget.c, main/kest_console.c and widget tests.
