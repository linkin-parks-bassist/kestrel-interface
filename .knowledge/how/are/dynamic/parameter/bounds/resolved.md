---
status: green
revised_at: "2026-10-04T23:51:24+11:00"
---

kest_parameter_get_range_rec starts with the parameter's literal min/max. Null parameters or excessive recursion return an unbounded interval. An associated effect without a scope returns the literal interval immediately.

Otherwise each available expression independently overrides its bound: constants evaluate without a scope; dependent expressions use the effect scope when available, or a cached value when there is no effect. A missing expression retains its literal bound. Gain and mixed literal/expression regressions establish the repaired fallback.

Sources: components/core/kest_parameter.c and tests/ui/kest_parameter_widget_test.c.
