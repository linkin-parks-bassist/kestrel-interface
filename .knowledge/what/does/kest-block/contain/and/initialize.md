---
status: "unverified"
created_at: "2026-09-20T00:07:11+10:00"
scope: "local"
source: "components/core/kest_block.c:15-127"
---
Status: Green

A `kest_block` represents one DSP instruction with opcode, three typed operands, destination, two optional register expressions/formats, shift and saturation policy, and resource reference. `kest_init_block` starts it as a NOP; descriptor initialization supplies opcode/shift policy; clone helpers copy a block with or without resource linkage. Source: components/core/kest_block.c:15-127
