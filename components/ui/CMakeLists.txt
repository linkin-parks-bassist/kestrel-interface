idf_component_register(SRCS "m_button.c"
							"m_home_view.c"
							"m_menu.c"
							"m_parameter_widget.c"
							"m_profile_view.c"
							"m_profile_settings.c"
							"m_sequence_list.c"
							"m_sequence_view.c"
							"m_effect_select.c"
							"m_effect_settings.c"
							"m_effect_view.c"
							"m_page_id.c"
							"m_ui.c"
					REQUIRES
							driver
							drivers
							fpga
					INCLUDE_DIRS "." "../../main" "../" "../core" "../parser")

