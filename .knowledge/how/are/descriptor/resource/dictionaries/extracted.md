---
status: "unverified"
created_at: "2026-09-20T00:07:53+10:00"
scope: "local"
source: "components/parser/kest_dict_extract.c"
---
Status: Green

`kest_dict_extract.c` converts descriptor dictionaries into parameters, settings, memory, delay, filter, LFO and polynomial resources. Low/high/band-pass filter extractors require numeric cutoff, accept Q or resonance (default 1/sqrt(2)), build coefficient expressions and attach filter resource data. Several allocation sites carry TODO comments to move parser-created objects into the parser arena. Source: components/parser/kest_dict_extract.c
