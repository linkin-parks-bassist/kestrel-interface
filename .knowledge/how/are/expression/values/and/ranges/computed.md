---
status: green
revised_at: "2026-10-06T18:09:05+11:00"
---

kest_expression.c builds constant, reference, unary and binary expression nodes; recursively evaluates expressions against a scope; tests constantness and parameter references; computes min/max/interval bounds; and prints expressions. Depth is bounded by KEST_EXPR_REC_MAX_DEPTH.

The borrowed t expression samples host/MCU time once at kest_expression_evaluate entry. UI/firmware evaluation returns seconds since global_cxt.epoch_start_ms; library builds have no context and use their clock value (currently zero). Scope dependency propagation treats the exact key t as updatable. The updater visits each active effect's t entry each control iteration; successful program queue submission advances the epoch. Rejected submission preserves epoch/state. The merged source retains these main-branch hooks; installed firmware predates them. A two-second reference/dependency host check and queue-rejection epoch checks pass; continuous modulation, epoch timing against actual FPGA commit and physical performance remain unqualified.

Evaluation writes cached/cached_val on its return path. Only constant-and-cached nodes bypass recursive evaluation; ordinary references resolve through the supplied scope. Recognized pi/e/sample_rate references become constant during evaluation. Shared descriptor graphs therefore contain mutable cache state even when dynamic expressions are reevaluated per scope. Parameter bounds can use a dependent node's last cached value without an associated effect; the dynamic-bounds owner specifies that fallback.

UI widgets evaluate parameter values and bounds; the control task evaluates register/coefficient/delay expressions while holding the UI lock in UI builds. Separate 100-Hz smoothing calls kest_parameter_evaluate, which can follow a driver to a scope expression or LFO and traverse descriptor graphs. The installed smoothing pass now holds the UI/model lock, coordinating evaluation with control/UI/reload; its effect mutex remains unused. These are source-reachable overlapping evaluation paths; no physical cache-race or numerical failure is established. Arena allocation changes storage lifetime, not mutable-cache ownership or concurrency safety. Preserve separate smoothing and review evaluation ownership before architecture changes.

Instruction send policies compose into this same expression tree during assembly parsing. Evaluation, bounds and dependency discovery therefore include the transformation without separate evaluator or register-writer instruction cases. ERF and LOG10 upper-bound cases now apply erf and log10 respectively, matching their evaluation and lower-bound functions. The numeric policy tests independently check upper bounds at x=100: erf gives 1 and log10 gives 2.

Subtraction bounds use min(a)-max(b) and max(a)-min(b). Both right-hand bounds are actually requested before evaluation; the lower-bound dependency regression checks 1-p for p in [0.1,0.9] and its use in instruction format resolution. This prevents inverted intervals for envelope coefficients such as 1-attack. Observed carrier Auto-Wah, Compressor and Gate descriptors compile after this correction.

Sources: components/core/{kest_expression,kest_parameter,kest_driver,kest_expr_scope,kest_param_update,kest_update}.c, components/ui/kest_parameter_widget.c, components/parser/kest_asm_parser.c and tests/core/kest_numeric_policy_test.c.
