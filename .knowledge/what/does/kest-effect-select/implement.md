---
status: green
revised_at: "2026-10-05T22:43:21+11:00"
---

components/ui/kest_effect_select.c builds selector buttons from global_cxt.effects at initialization. Enter it from a preset's Add Effect control. Selection checks the working preset's remaining instruction capacity before calling kest_preset_append_effect_eff. Capacity rejection stays on the selector, preserves the preset and displays a closable 'Cannot add effect' explanation asking the user to remove an effect. Construction failure likewise returns with an explanation rather than dereferencing a null effect. Only successful creation returns to the preset, adds its button and creates the effect view. A host LVGL event test checks full-preset rejection, unchanged pipeline/page and creation of the modal.

This selects a loaded descriptor, not an SD file. Startup load_effects populates descriptors before selector initialization. Buttons snapshot that list; refresh_effect_selector remains ERR_UNIMPLEMENTED. Existing-identity eff-reload rebinds selector descriptors; publishing a new identity through UART alone does not add a button. Live discovery/refresh remain planned.

Sources: components/ui/kest_effect_select.c, components/core/kest_files.c, kest_preset.c, tests/core/kest_pipeline_test.c and UART/refresh owners.
