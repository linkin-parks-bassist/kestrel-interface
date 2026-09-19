---
status: "unverified"
created_at: "2026-09-20T00:04:02+10:00"
scope: "local"
source: "components/fpga/kest_fpga_encoding.c:11-102"
---
Status: Green

`kest_fpga_block_opcode_format` selects format B for LUT, delay, memory, filter, cascade and polynomial resource opcodes; other opcodes use A. Format A packs opcode, three source selectors/type bits, destination, shift and saturation fields. Format B packs opcode, two source selectors/type bits, destination and a resource handle. Positional encoding offsets memory, delay and filter handles by effect placement and writes effective memory addresses. Source: components/fpga/kest_fpga_encoding.c:11-102
