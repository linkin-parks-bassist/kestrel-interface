---
status: "unverified"
created_at: "2026-09-20T00:07:54+10:00"
scope: "local"
source: "components/fpga/kest_fpga_io.c"
---
Status: Green

`kest_fpga_io.c` owns ESP SPI transfer and growable byte batches. Program send wraps a batch in BEGIN_PROGRAM and END_PROGRAM commands; ordinary transfer sends raw batch bytes. Careful variants print each byte and status flag for diagnostics. It also decodes status flags and requests controller data. Source: components/fpga/kest_fpga_io.c
