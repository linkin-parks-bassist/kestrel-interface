---
status: "unverified"
created_at: "2026-09-20T00:07:53+10:00"
scope: "local"
source: "components/parser/kest_expr_parser.c:9-180,352"
---
Status: Green

`kest_expr_parser.c` maps unary functions and binary operators to expression node kinds, assigns precedence for add/subtract, multiply/divide and power, and uses a Pratt parser for nested expressions and function calls. Function calls are arity checked and malformed separators produce parser errors. Source: components/parser/kest_expr_parser.c:9-180,352
