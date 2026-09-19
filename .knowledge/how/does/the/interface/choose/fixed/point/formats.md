---
status: "unverified"
created_at: "2026-09-20T00:04:02+10:00"
scope: "local"
source: "components/fpga/kest_fixed_point.c:7-92; components/fpga/kest_reg_format.c:11-113"
---
Status: Green

`kest_expression_compute_format` gets an expression min/max and chooses the first q format up to fmax whose signed range contains them. `kest_compute_register_formats` assigns formats to active per-block registers and computes a shift from operand formats and instruction shift policy. `float_to_q_nminus1` and the filter-width variant clamp to representable range before rounding. Source: components/fpga/kest_fixed_point.c:7-92; components/fpga/kest_reg_format.c:11-113
