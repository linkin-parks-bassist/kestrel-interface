---
status: green
revised_at: "2026-10-04T08:59:12+11:00"
---

kest_dependent.c constructs records for scope entries, bounded/driven parameters, block registers and filter coefficients. These identify recomputation or FPGA writes after a scope value changes. Block-register records carry the resolved kest_numeric_format encoding, while filter-coefficient records retain their legacy integer format. kest_fpga_write and kest_fpga_command preserve the appropriate member through queued conversion.

Sources: components/core/kest_dependent.[ch], kest_expr_scope.[ch], kest_update.[ch] and components/fpga/kest_fpga_cmd.[ch].
