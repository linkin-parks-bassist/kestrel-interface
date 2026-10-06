#ifndef KEST_DICT_EXTRACT_H_
#define KEST_DICT_EXTRACT_H_

kest_setting      *kest_extract_setting(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name);
kest_dsp_resource *kest_extract_resource(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name);
kest_parameter    *kest_extract_parameter(kest_eff_parsing_state *ps, kest_eff_entry_dict *dict, const char *name);

#endif
