#ifndef KEST_DICT_EXTRACT_H_
#define KEST_DICT_EXTRACT_H_

kest_setting      *kest_extract_setting_from_dict	 (kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict);
kest_parameter    *kest_extract_parameter_from_dict(kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict);
kest_dsp_resource *kest_extract_resource_from_dict (kest_eff_parsing_state *ps, kest_ast_node *dict_node, kest_dictionary *dict);

#endif
