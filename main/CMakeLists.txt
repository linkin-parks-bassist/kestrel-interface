idf_component_register(SRCS "kest_int.c"
					REQUIRES
							esp_timer
							fatfs
							drivers
							ui
							core
							fpga
					INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE -DM_INTERFACE)
