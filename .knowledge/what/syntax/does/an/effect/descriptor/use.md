---
status: green
revised_at: "2026-10-06T09:50:04+11:00"
---

A descriptor begins with v1.0. Recognized sections are .INFO, .RESOURCES, .PARAMETERS, .SETTINGS, .DEFS and .CODE. Metadata is dictionary-like; .CODE is DSP assembly. Expressions and named resources are resolved while compiling.

.INFO requires a string name; optional cname supplies stable effect identity. Optional description is a string; keywords, instruments, types and genres are string lists: keywords: {"feedback", "rhythmic"}. Empty lists and final items without trailing commas are accepted. Known discovery fields with wrong value types reject the descriptor. These fields do not alter DSP encoding. The descriptor retains full text, case, list order and duplicates; missing fields need no defaults in old files. Discovery search/filter UI remains planned.

Sources: docs/eff_guide.html, components/parser/{kest_dictionary,kest_eff_parser}.c and discovery metadata regressions.
