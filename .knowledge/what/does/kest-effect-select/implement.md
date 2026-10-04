---
status: green
revised_at: "2026-10-04T04:05:47+11:00"
---

components/ui/kest_effect_select.c builds selector buttons from global_cxt.effects when the selector is initialized. Enter it from a preset's add-effect control; selecting a description invokes add_effect_from_menu_eff, appends an instance to global_cxt.working_preset, returns to that preset's view, appends its effect button and creates the effect view.

This selects a loaded descriptor, not a file directly from SD. Publishing a new .eff through UART does not add a selector entry in the existing session: refresh_effect_selector returns ERR_UNIMPLEMENTED, and selector buttons snapshot the descriptor list at initialization. Startup load_effects populates that list before selector initialization. Live discovery and selector refresh remain planned.

Sources: components/ui/kest_effect_select.c, components/core/kest_files.c and the UART file-command contract.
