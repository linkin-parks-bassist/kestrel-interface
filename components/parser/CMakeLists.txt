idf_component_register(SRCS "kest_asm_parser.c"
							"kest_dictionary.c"
							"kest_expr_parser.c"
							"kest_eff_parser.c"
							"kest_eff_section.c"
							"kest_tokenizer.c"
							"kest_dict_extract.c"
					REQUIRES
							driver
							drivers
							ui
					INCLUDE_DIRS "." "../../main" "../" "../ui" "../drivers")
