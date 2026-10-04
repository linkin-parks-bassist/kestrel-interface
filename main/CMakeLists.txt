idf_component_register(SRCS "kest_int.c" "kest_console.c"
					REQUIRES
                            console
                            esp_app_format
							esp_timer
							fatfs
							drivers
							ui
							core
							fpga
					INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE -DM_INTERFACE)
