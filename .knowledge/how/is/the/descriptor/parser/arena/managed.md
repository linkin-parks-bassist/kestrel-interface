---
status: "unverified"
created_at: "2026-09-20T00:02:29+10:00"
scope: "local"
source: "components/parser/kest_eff_parser.c:14-75; components/parser/kest_eff_parser.h"
---
Status: Green

The descriptor parser uses a 512 KiB bump arena (`KEST_EFF_PARSER_MEM_POOL_SIZE_KB`). Initialization is idempotent, reset requires prior initialization, deinitialization destroys it, and parser allocation/string duplication draw from that arena. Source: components/parser/kest_eff_parser.c:14-75; components/parser/kest_eff_parser.h
