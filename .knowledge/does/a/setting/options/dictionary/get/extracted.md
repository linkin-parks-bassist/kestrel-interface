---
status: "unverified"
created_at: "2026-09-20T00:14:05+10:00"
scope: "local"
source: "caller-supplied answer; evidence not recorded"
---
Status: Green

No. In components/parser/kest_dict_extract.c around line 1481, a setting attribute named options passes ASSERT_ATTR_DICT(), then enters a TODO branch with no extraction. The parser does not report an unrecognized-attribute error for this key, so a descriptor can appear accepted while its options are ignored. The intended options semantics need user specification and a parser test before implementation.
