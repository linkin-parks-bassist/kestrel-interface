---
status: green
revised_at: "2026-10-04T05:18:37+11:00"
---

These functions directly return ERR_UNIMPLEMENTED: refresh_effect_selector (components/ui/kest_effect_select.c), clear_effect_view (kest_effect_view.c), free_preset_view (kest_preset_view.c) and free_menu_page_ui (kest_menu.c). Code after the return in free_preset_view is unreachable. The parameter widget also returns ERR_UNIMPLEMENTED for an unsupported setting-widget type. These are source observations; whether a call is reached in normal use needs call-path testing.

Effect-settings UI and complete-page destruction are implemented; their ownership belongs to what/does/kest-effect-settings/implement.md.
