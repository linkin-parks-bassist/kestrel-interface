---
status: green
revised_at: "2026-10-04T08:59:14+11:00"
---

components/fpga/kest_fpga_instr.h declares kest_instr_arg_fmt for authored argument positions and kest_asm_instr_desc for name, opcode, argument layout and numeric policy. kest_arg_numeric_policy defines signed/unsigned permissible fractional-bit sets, overflow policy and an optional send-expression builder. kest_instr_numeric_policy groups three argument policies and a callback resolving the shared field from a concrete block and chosen format tuple.

It includes kest_numeric_format.h, which defines fractional bits, signedness and saturation/rejection; that header also declares kest_numeric_format_bounds and kest_encode_numeric. Name/opcode lookup returns const descriptors, and kest_instr_print formats encoded instructions. KEST_ARG_POS_NONE marks absent syntax positions; shift_pos identifies a literal authored field rather than numeric-conversion policy.

Sources: components/fpga/kest_fpga_instr.h and kest_numeric_format.h.
