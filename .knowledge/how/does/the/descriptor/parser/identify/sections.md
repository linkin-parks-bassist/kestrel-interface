---
status: green
revised_at: "2026-09-20T00:02:29+10:00"
---

`kest_parse_tokens` first scans tokens for section starts and creates AST section nodes containing names and token spans; it then parses those spans as sections. Invalid section names report a parser error and return ERR_BAD_ARGS. Source: components/parser/kest_eff_parser.c:95-220
