---
status: green
revised_at: "2026-10-04T23:02:34+11:00"
---

`components/ui/kest_preset_settings.c` owns the preset settings page lifecycle and callbacks for saving and choosing the default preset. Its constructor uses tracked allocation, explicitly initializes the page data and embedded Gain widget, and creates one panel. Panel-allocation failure releases the data allocation. Configuration uses the shared parameter-widget configurator, binding the preset volume, parent page, driver state and current nominal value before UI creation.

The focused host regression checks initialized widget/button fields and configuration from a -2.5 Gain parameter, including the -2.50 label and literal -12..12 bounds. The parameter owner holds the repaired literal-bound fallback. Together these repair the missing widget initialization/configuration and unbounded drag calculation found during carrier HIL. /tmp/kestrel-gain-range-live-hil.log records ordinary synthetic touch on Preset 4 Settings showing 0.0, 7.9 and -8.0 dB, followed by deletion of the temporary preset and restoration of the prior preset. The installed image and build/flash evidence belong to how/to/build/and/run/the/interface.md; this is a focused UI check, not instrumented audio qualification.

The page's free/refresh/entry functions remain placeholders; this change does not establish complete page teardown or persistence behavior. Source: components/ui/kest_preset_settings.c and tests/ui/kest_parameter_widget_test.c.
