---
status: "unverified"
created_at: "2026-09-20T00:07:54+10:00"
scope: "local"
source: "components/parser/kest_eff_parser.c:335-420; components/core/kest_files.c:1112-1160"
---
Status: Green

`kest_parse_tokens` allocates `ps->scope` and calls `kest_scope_init(ps->scope)` before testing `ps->scope` for null, so allocation failure can dereference null. Descriptor parsing also resets a shared arena between files; persistent result objects therefore must live outside that reset arena. Source: components/parser/kest_eff_parser.c:335-420; components/core/kest_files.c:1112-1160
