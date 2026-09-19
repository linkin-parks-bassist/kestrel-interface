---
status: "unverified"
created_at: "2026-09-20T00:03:23+10:00"
scope: "local"
source: "components/core/kest_parameter.c:413-487"
---
Status: Green

`kest_parameter_get_range_rec` begins with an unbounded interval, evaluates constant bound expressions without a scope, evaluates dependent bounds in the effect scope when available, and can use cached expression values when there is no effect scope. A recursion-depth guard returns the unbounded interval. Source: components/core/kest_parameter.c:413-487
