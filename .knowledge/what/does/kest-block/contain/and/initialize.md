---
status: green
revised_at: "2026-10-04T08:59:13+11:00"
---

kest_block represents one DSP instruction with opcode, three typed operands, destination, two optional expression registers and their kest_numeric_format encodings, resolved instruction field, saturation-disable flag, resource reference and the authored descriptor pointer. Retaining that descriptor distinguishes MOV/ADD/SUB despite their common MADD opcode. kest_init_block sets NOP, channel-zero defaults, inactive registers with signed saturating Q15 encodings in the 16-bit build, field zero and null descriptor/resource. Descriptor initialization supplies opcode and descriptor identity; joint numeric resolution supplies concrete encodings and field. Clones copy descriptor identity and resolved register encodings, with the existing resource-clone or resource-unlink behavior.

Sources: components/core/kest_block.[ch], components/fpga/kest_reg_format.c.
