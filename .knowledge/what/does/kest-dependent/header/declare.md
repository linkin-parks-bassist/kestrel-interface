---
status: green
revised_at: "2026-10-04T08:59:15+11:00"
---

`components/core/kest_dependent.h` declares API names: `kest_dependent_scope_entry`, `kest_dependent_bound_parameter`, `kest_dependent_driven_parameter`, `kest_dependent_block_reg`, `kest_dependent_filter_coef`, `kest_string_append_dependent`, `kest_dependent_is_updatable`; configuration symbols: `KEST_DEPENDER_H_`, `KEST_DEPENDENT_NONE`, `KEST_DEPENDENT_SCOPE_ENTRY`, `KEST_DEPENDENT_BLOCK_REG`, `KEST_DEPENDENT_FILTER_COEF`, `KEST_DEPENDENT_DRIVEN_PARAMETER`, `KEST_DEPENDENT_BOUND_PARAMETER`. Block-register constructors take kest_numeric_format and store encoding; filter coefficients retain an integer format. The tagged record uses the corresponding union member. Consult the C implementation for behavior and ownership.

Source: components/core/kest_dependent.h
