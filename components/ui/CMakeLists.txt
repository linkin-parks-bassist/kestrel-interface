idf_component_register(SRCS "kest_button.c"
							"kest_home_view.c"
							"kest_menu.c"
							"kest_parameter_widget.c"
							"kest_profile_view.c"
							"kest_profile_settings.c"
							"kest_sequence_list.c"
							"kest_sequence_view.c"
							"kest_effect_select.c"
							"kest_effect_settings.c"
							"kest_effect_view.c"
							"kest_page_id.c"
							"kest_ui.c"
					REQUIRES
							driver
							drivers
							fpga
					INCLUDE_DIRS "." "../../main" "../" "../core" "../parser")

