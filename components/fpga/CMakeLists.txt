idf_component_register(SRCS "kest_fpga_comms.c"
							"kest_fpga_encoding.c"
							"kest_fpga_io.c"
							"kest_reg_format.c"
							"kest_fixed_point.c"
					REQUIRES
							driver
							drivers
							ui
					INCLUDE_DIRS "." "../../main" "../" "../ui" "../drivers" "../parser")

							
