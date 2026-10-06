---
status: green
revised_at: "2026-10-04T23:00:41+11:00"
---

`components/core/kest_parameter.h` declares API names: `kest_parameter_evaluate`, `kest_parameter_evaluate_rec`, `kest_parameter_if_driven_refresh`, `kest_parameter_set`, `kest_parameter_driver_set`, `kest_parameter_refresh_pw_async`, `kest_parameter_if_updated_refresh_pw_async`, `kest_parameter_clear_update`, `kest_parameter_add_dependent_block`, `kest_parameters_assign_ids`, `kest_settings_assign_ids`, `kest_parameter_free`, `kest_setting_free`, `kest_parameter_get_range`, `kest_parameter_get_range_rec`, `kest_parameter_detect_bounds_updates`, `kest_parameter_if_updated_refresh_pw`; named types: `kest_parameter_id`, `kest_parameter`, `kest_setting_option`, `kest_setting_id`, `kest_setting`; configuration symbols: `KEST_INT_PARAMETER_H_`, `KEST_STANDARD_GAIN_MIN`, `KEST_STANDARD_GAIN_MAX`, `KEST_PARAMETER_UNDRIVEN`, `KEST_ATOMIC`.

`kest_parameter_get_range_rec` starts a valid parameter's range from its literal min/max and independently overrides either bound with its expression evaluation or cached value. Parameters such as preset Gain without expressions therefore retain their finite constructor bounds; mixed literal/expression bounds retain the untouched literal side. The focused host tests cover preset Gain's -12..12 range and each mixed-bound direction. Invalid/deep recursion returns the real-line sentinel; the null diagnostic does not dereference the absent parameter. Other expression scope/evaluation behavior remains as implemented.

Source: components/core/kest_parameter.[ch] and tests/ui/kest_parameter_widget_test.c. The preset-settings owner holds widget initialization and hardware observations.
