---
status: "unverified"
created_at: "2026-09-20T00:13:59+10:00"
scope: "local"
source: "caller-supplied answer; evidence not recorded"
---
Status: Green

Source audit 2026-09-20: these functions directly return ERR_UNIMPLEMENTED: refresh_effect_selector (components/ui/kest_effect_select.c:80), clear_effect_view (kest_effect_view.c:341), free_preset_view (kest_preset_view.c:512), free_menu_page_ui (kest_menu.c:698), free_effect_settings_page_ui and effect_settings_page_free_all (kest_effect_settings.c:265,276). In the latter two and free_preset_view, code after that return is unreachable. The parameter widget also returns ERR_UNIMPLEMENTED for its default switch case (kest_parameter_widget.c:834), which is conditional on an unsupported type. These are source observations; whether a given call is reached in normal use needs call-path testing.
