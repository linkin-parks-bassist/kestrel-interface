---
status: green
revised_at: "2026-10-04T09:34:50+11:00"
---

kest_expression.c builds constant, reference, unary and binary expression nodes; recursively evaluates expressions against a scope; tests constantness and parameter references; computes min/max/interval bounds; and prints expressions. Depth is bounded by KEST_EXPR_REC_MAX_DEPTH.

Instruction send policies compose into this same expression tree during assembly parsing. Evaluation, bounds and dependency discovery therefore include the transformation without separate evaluator or register-writer instruction cases. ERF and LOG10 upper-bound cases now apply erf and log10 respectively, matching their evaluation and lower-bound functions. The numeric policy tests independently check upper bounds at x=100: erf gives 1 and log10 gives 2.

Subtraction bounds use min(a)-max(b) and max(a)-min(b). Both right-hand bounds are actually requested before evaluation; the lower-bound dependency regression checks 1-p for p in [0.1,0.9] and its use in instruction format resolution. This prevents inverted intervals for envelope coefficients such as 1-attack. Observed carrier Auto-Wah, Compressor and Gate descriptors compile after this correction.

Sources: components/core/kest_expression.c, components/parser/kest_asm_parser.c and tests/core/kest_numeric_policy_test.c.
