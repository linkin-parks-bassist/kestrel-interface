---
status: green
revised_at: "2026-10-06T00:43:40+11:00"
---

components/parser/kest_dict_extract.h declares kest_extract_parameter, kest_extract_setting and kest_extract_resource over kest_eff_entry_dict, plus guard KEST_DICT_EXTRACT_H_. The obsolete kest_dictionary-based *_from_dict declarations and roughly 950-line implementation have no remaining source callers and are removed. Current extraction behavior/ownership belongs to how/are/descriptor/resource/dictionaries/extracted.md. 205 host tests and allocation-failure probes pass; cleanup is installed; boot/active Swamp reload preserve pools/presets/status 0x01 (/tmp/kestrel-extractor-cleanup-live.log) (/tmp/kestrel-extractor-cleanup-{tests,allocation}.log).
