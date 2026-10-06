---
status: green
revised_at: "2026-10-05T14:55:46+11:00"
---

components/parser/kest_asm_parser.h declares kest_parse_asm and kest_process_asm_lines. Operand-kind constants are KEST_ASM_ARG_CHANNEL, KEST_ASM_ARG_EXPR, KEST_ASM_ARG_RES and KEST_ASM_ARG_INT; INSTR_MAX_ARGS is four and KEST_ARG_POS_NONE is −1.

kest_asm_arg carries type and expression; kest_asm_operand carries type/address/value/expression/resource/name; kest_asm_instr contains an opcode and operand array; kest_asm_line contains a mnemonic, expression-bearing arguments, source line and argument count. The header declares a linked-pointer list for assembly lines.

The implementation's active processor obtains opcode and argument layout through kest_instr_name_to_desc in the FPGA instruction module. It has no separate string-to-opcode or string-to-argument-layout helpers. Mnemonic validation still uses its own accepted-name array. Consult the parser-error owner for rejection/ownership behavior.

Sources: components/parser/kest_asm_parser.[ch], components/fpga/kest_fpga_instr.[ch].
