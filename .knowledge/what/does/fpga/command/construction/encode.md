---
status: green
revised_at: "2026-10-04T08:59:13+11:00"
---

kest_fpga_cmd.c constructs typed programming, instruction/register, delay/filter, coefficient, commit and tail commands. Register commands retain kest_numeric_format; filter commands retain an integer coefficient format. kest_fpga_command_append_encoded serializes the command and payload, using kest_encode_numeric through the numeric batch helper for register values. It preserves exact append/conversion error codes, and command-list encoding stops at the first error.

Initial and scope-driven register batches use the same command encoder. kest_updater_generate_tx_batch returns conversion errors and clears the failed batch length; the control loop does not send that failed batch. Pipeline program construction propagates register conversion errors instead of returning an apparently successful partial program. Numeric policy tests check rejected writes and discarded updater output.

Sources: components/fpga/kest_fpga_cmd.c, kest_fpga_encoding.c, kest_fpga_io.c, components/core/kest_update.c and tests/core/kest_numeric_policy_test.c.
