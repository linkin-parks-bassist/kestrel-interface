---
status: green
revised_at: "2026-10-04T08:59:14+11:00"
---

components/fpga/kest_fpga_instr.c contains assembler instruction descriptors, name/opcode lookup and printing of encoded words. Each descriptor names its argument layout and numeric policy; shared policies define permissible operand formats, expression send transforms and joint field rules. The block retains the authored descriptor rather than reconstructing alias identity from opcode. Compare numeric opcodes with Core include/instr_dec.vh and argument consumption with its execution units. how/does/the/interface/choose/fixed/point/formats.md owns policy behavior.
