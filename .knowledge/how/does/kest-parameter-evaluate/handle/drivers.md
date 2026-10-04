---
status: green
revised_at: "2026-09-20T00:03:23+10:00"
---

`kest_parameter_evaluate_rec` returns 0 for a null parameter or recursion-depth overflow. For a valid non-overridden driver index, it evaluates that driver in the effect scope and stores the computed result into `param->value` as a side effect; otherwise it returns the stored value. Source: components/core/kest_parameter.c:539-600
