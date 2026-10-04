---
status: green
revised_at: "2026-10-04T08:59:15+11:00"
---

`components/fpga/kest_fpga_cmd.h` declares API names: `kest_fpga_command_begin_program`, `kest_fpga_command_end_program`, `kest_fpga_command_write_block_instr`, `kest_fpga_command_write_block_reg_0`, `kest_fpga_command_write_block_reg_1`, `kest_fpga_command_update_block_reg_0`, `kest_fpga_command_update_block_reg_1`, `kest_fpga_command_commit_reg_updates`, `kest_fpga_command_alloc_delay`, `kest_fpga_command_alloc_filter`, `kest_fpga_command_write_filter_coef`, `kest_fpga_command_update_filter_coef`, `kest_fpga_command_commit_filter_coefs`, `kest_fpga_command_enable_tail`, `kest_fpga_command_append_encoded`, `kest_fpga_command_list_append_encoded`, `kest_fpga_command_to_string_`; configuration symbols: `KEST_FPGA_COMMAND_H_`. kest_fpga_command holds a numeric encoding for register values or a legacy integer format for filter coefficients. The four register constructors accept kest_numeric_format, preserving conversion for queued sends. Consult the C implementation for behavior and ownership.

Source: components/fpga/kest_fpga_cmd.h
