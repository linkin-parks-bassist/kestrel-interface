---
status: "unverified"
created_at: "2026-09-20T00:07:54+10:00"
scope: "local"
source: "components/fpga/kest_fpga_cmd.c"
---
Status: Green

`kest_fpga_cmd.c` constructs typed commands for begin/end program, block instruction/register writes, delay/filter allocation, coefficient updates and commits, and tail enable. `kest_fpga_command_append_encoded` serializes command type and payload into a byte transfer batch. Source: components/fpga/kest_fpga_cmd.c
