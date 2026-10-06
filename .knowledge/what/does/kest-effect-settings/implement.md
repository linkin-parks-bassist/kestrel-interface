---
status: green
revised_at: "2026-10-05T23:53:19+11:00"
---

components/ui/kest_effect_settings.c initializes/configures an effect settings page and creates/refreshes its band controls. It owns a panel, copied title and data containing embedded LP/HP parameter widgets and a band-mode setting widget. Configuration replaces the title and reports allocation failure.

The built-in Apply to dropdown has four options from kest_effect.c: all frequencies, above cutoff, below cutoff and a band. Effect initialization creates Wet Mix and cutoff parameters. This is scaffolding: filtered-effect/complementary-clean recombination needs symbolic program transformations under what/is/the/symbolic/dsp/composition/direction.md. Deprecated Teensy support is user-reported precedent, not evidence of current FPGA band processing.

Generic dropdowns consume setting->options/n_options; clones copy option records while retaining descriptor-owned labels. Authored .eff enum options are implemented and installed; the options-extraction owner specifies schema, validation and ownership. Integer/enum types and widget_type survive instance cloning. Numeric/dropdown commits share changed-value handling, mark presets dirty and request active-preset rebuilding. Actual LVGL/queue tests qualify that route; RHYTHM carrier checks also qualify selection and active reload preservation; exact physical delay timing remains unqualified under its owner. Tempo/subdivision requirements belong to the superproject RHYTHM owner. David recalls previous dropdown support; its history remains unestablished.

Both destruction hooks are installed. free_effect_settings_page_ui cancels queued entry, cleans embedded widgets without freeing storage, deletes containers even when reparented backstage, clears backlinks/timers, deletes screen and resets UI state. Repeated cleanup is safe; creation restores backlinks. effect_settings_page_free_all also releases title, panel, data and page. Effect-view destruction invokes complete cleanup on the UI task.

C regressions exercise backstage cutoff containers, pending refresh/timer, repeated cleanup and complete release. Sources: components/ui/kest_effect_settings.c, kest_effect_view.c, kest_parameter_widget.c, core/kest_effect.c, kest_parameter.c, parser/kest_dict_extract.c and tests/ui/kest_parameter_widget_test.c. Pipeline lifecycle owner governs retirement.
