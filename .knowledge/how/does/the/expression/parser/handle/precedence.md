---
status: green
revised_at: "2026-10-05T13:33:33+11:00"
---

kest_expr_parser.c maps unary functions and binary operators to node kinds. Pratt parsing gives add/subtract lower precedence than multiply/divide, with power above both; parentheses and function arity are checked. Malformed separators produce errors.

The wrapper initializes its output cursor to the starting token before recursion. Success publishes the next unconsumed token and detects constants. Early failure preserves the start instead of publishing uninitialized data; failure returns NULL.

Fresh Pratt nodes form owned trees: constants/references are allocated, references own duplicated names, and unary/binary constructors consume parsed children. They do not embed global constants or compiler-generated shared graphs. kest_free_parsed_expression releases reference names, known parsed children and allocator-matched nodes. Its contract is fresh parser-owned trees only, never compiled/shared graphs or borrowed nodes.

Pratt bail releases unattached lhs/rhs; successful construction clears local child ownership. Function failure releases completed arguments; success transfers them to its root. Current dictionary parsing also uses this helper for rejected insertions and failed partial containers, recursively before extraction. The dictionary owner governs container cleanup.

A one-slot regression checks three failed constant parses preserving the cursor, then recovery after release. Seven malformed calls/parentheses/binary forms across three attempts return all temporary slots and preserve an unrelated constant, including nested/reference nodes. Four exhausted unary/infix/function/nested-unary constructors check cleanup with one/two/three free slots. Rejected expression/dictionary/list values likewise return slots over three passes while preserving a held node. Four partial-container failures return slots; the standalone probe qualifies initial list-array allocation/growth failures. Complete heap rollback remains separate.

Pre-extraction failures reclaim section trees. Failed parameter extraction retires new driver borrows before freeing rejected/unprocessed parameter trees. Reader rejection releases extracted parameter/setting/resource/definition metadata and driver containers while retaining expression graphs. Parameter insertion failure releases unattached metadata and new driver borrows without reclaiming expressions. Shared graphs still lack complete rollback; the arena owner governs ownership boundaries.

Sources: components/parser/kest_expr_parser.[ch], kest_dictionary.c and tests/parser/kest_readback_effect_test.c.
