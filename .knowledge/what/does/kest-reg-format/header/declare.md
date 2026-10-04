---
status: green
revised_at: "2026-10-04T08:59:14+11:00"
---

components/fpga/kest_reg_format.h declares kest_resolve_block_formats for one block and kest_compute_register_formats for a linked block list, with KEST_REG_FORMAT_H_ as its include guard. Both return error codes. The fixed-point-format owner describes joint tuple selection and persisted encodings; consult kest_reg_format.c for behavior.
