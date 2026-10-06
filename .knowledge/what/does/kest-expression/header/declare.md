---
status: green
revised_at: "2026-10-06T18:09:06+11:00"
---

components/core/kest_expression.h declares expression/interval types, constructors, reference inspection, evaluation, constant detection, range computation, dependency updates and LPF/HPF/BPF coefficient generation.

Current source also declares kest_expression_capture_begin/end/destroy and kest_expression_alloc/free_node. Capture records newly allocated nodes for descriptor ownership; freeing a fresh node removes it from the active capture. Capture destruction frees owned reference names and each retained node once. Its global serialization and unqualified lifetime boundaries belong to how/is/the/descriptor/parser/arena/managed.md.

It declares the borrowed kest_expression_t time reference. ACC and DIFF type identifiers extend TYPE_MAX_VAL but have no evaluator cases; they are declarations, not implemented operators.

This header is a declaration map, not proof of complete ownership or evaluation correctness. Source: components/core/kest_expression.h.
