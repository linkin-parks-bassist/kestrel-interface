---
status: "unverified"
created_at: "2026-09-20T00:07:12+10:00"
scope: "local"
source: "components/core/kest_expression.c"
---
Status: Green

`kest_expression.c` builds constant, reference, unary and binary expression nodes; recursively evaluates expressions against a scope; tests constantness and parameter references; computes min/max/range bounds; and prints expressions. Depth is bounded by KEST_EXPR_REC_MAX_DEPTH. Source: components/core/kest_expression.c
