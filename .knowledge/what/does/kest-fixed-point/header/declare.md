---
status: green
revised_at: "2026-10-04T09:02:50+11:00"
---

components/fpga/kest_fixed_point.h declares the legacy float_to_q_nminus1, float_to_q15 and filter-width converter, kest_expression_compute_format and kest_filter_compute_format, guarded by KEST_FIXED_POINT_H_. It includes kest_numeric_format.h, which declares the resolved register-format type and bounds/encoding API. Their implementation is in kest_fixed_point.c; instruction register resolution belongs to kest_reg_format.c, while filters retain the legacy integer-format path.
