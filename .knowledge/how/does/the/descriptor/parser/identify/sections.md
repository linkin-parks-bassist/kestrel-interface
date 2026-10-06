---
status: green
revised_at: "2026-10-05T04:54:13+11:00"
---

kest_parse_tokens scans section starts into AST nodes with names, token spans and source lines, then parses sections. Invalid names report an error and return ERR_BAD_ARGS. Scope allocation is checked before initialization; initialization failure propagates.

kest_read_eff_desc_from_file rejects nonzero parser status or reported errors before assembler processing/publication. Both checks matter: a failed scope dictionary previously allowed an incomplete descriptor to publish without a diagnostic.

make test-parser-allocation builds a standalone host-library test. Its executable interposes kest_alloc and rejects the 32-bucket scope dictionary allocation. Three attempts return null descriptors; disabling injection restores successful parsing. Removing either scope-initialization propagation or file-reader status rejection independently makes the probe fail. Evidence: /tmp/kestrel-parser-allocation-{test,isolated}.log. This qualifies that allocation site and recovery, not general heap rollback, other scope insertions or physical memory pressure.

The unnamed-resource fixture separately qualifies malformed top-level entry rejection, which already worked before this repair. Sources: components/parser/kest_eff_parser.c, tools/test_parser_allocation.c, tests/fixtures/{scope-allocation,unnamed-resource}.eff and Makefile. Section/error owners govern focused contracts.
