#ifndef KEST_EFF_DESC_FILE_SECTIONS_H_
#define KEST_EFF_DESC_FILE_SECTIONS_H_

typedef struct {
	char *name;
	kest_token_ll *tokens;
	struct kest_dictionary *dict;
} kest_eff_desc_file_section;

int get_section_start_score(char *str, int current_score);

int kest_settings_section_extract  (kest_eff_parsing_state *ps, kest_setting_pll      **list, struct kest_ast_node *sect);
int kest_parameters_section_extract(kest_eff_parsing_state *ps, kest_parameter_pll    **list, struct kest_ast_node *sect);
int kest_resources_section_extract (kest_eff_parsing_state *ps, kest_dsp_resource_pll **list, struct kest_ast_node *sect);
int kest_defs_section_extract 		(kest_eff_parsing_state *ps, kest_scope *scope, 		 struct kest_ast_node *sect);

int kest_dictionary_section_lookup_str  (kest_ast_node *section, const char *name, const char **result);
int kest_dictionary_section_lookup_float(kest_ast_node *section, const char *name, float *result);
int kest_dictionary_section_lookup_int  (kest_ast_node *section, const char *name, int *result);
int kest_dictionary_section_lookup_expr (kest_ast_node *section, const char *name, kest_expression **result);
int kest_dictionary_section_lookup_dict (kest_ast_node *section, const char *name, kest_dictionary **result);

int token_is_valid_section_name(char *str);

int kest_parse_dictionary_section(kest_eff_parsing_state *ps, kest_ast_node *section);
int kest_parse_code_section(kest_eff_parsing_state *ps, kest_ast_node *section);

#endif
