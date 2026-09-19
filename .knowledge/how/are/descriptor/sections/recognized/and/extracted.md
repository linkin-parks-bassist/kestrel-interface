---
status: "unverified"
created_at: "2026-09-20T00:07:53+10:00"
scope: "local"
source: "components/parser/kest_eff_section.c; components/parser/kest_eff_parser.c"
---
Status: Green

`kest_eff_section.c` recognizes exactly INFO, RESOURCES, PARAMETERS, SETTINGS, DEFS and CODE when a dot and name follow a newline. It parses dictionary-like sections, extracts typed parameters/settings/resources/definitions and parses code separately. Source: components/parser/kest_eff_section.c; components/parser/kest_eff_parser.c
