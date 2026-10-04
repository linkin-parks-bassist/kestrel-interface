---
status: green
revised_at: "2026-10-04T09:02:51+11:00"
---

The assembly instruction tables recognize nop, move/arithmetic/shift/limit operations, accumulator MAC operations, delay/memory/filter operations, LUT-backed tanh4/sin2pi, stateful SVF update/read operations and poly. Processing uses kest_instr_name_to_desc to obtain the authored descriptor, argument positions and numeric policy. arsh/lsh/rsh require four arguments: a b shift dest, with fields from 0 through 15 under their policy. A block has two expression registers; a third bracketed expression is rejected instead of overwriting the second register. Expression send transforms are composed before register assignment, and effect parsing propagates numeric-resolution errors.

Sources: components/parser/kest_asm_parser.c, kest_eff_parser.c, components/fpga/kest_fpga_instr.c and tests/parser/kest_readback_effect_test.c.
