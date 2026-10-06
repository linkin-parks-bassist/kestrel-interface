---
status: green
revised_at: "2026-10-05T17:06:49+11:00"
---

components/core/kest_expression.h declares expression/interval types, constructors, reference inspection, evaluation, constant detection, range computation, dependency updates and LPF/HPF/BPF coefficient generation.

Current source also declares kest_expression_capture_begin/end/destroy and kest_expression_alloc/free_node. Capture records newly allocated nodes for descriptor ownership; freeing a fresh node removes it from the active capture. Capture destruction frees owned reference names and each retained node once. Its global serialization and unqualified lifetime boundaries belong to how/is/the/descriptor/parser/arena/managed.md.

This header is a declaration map, not proof of complete ownership or evaluation correctness. Source: components/core/kest_expression.h.
