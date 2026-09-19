---
status: "unverified"
created_at: "2026-09-20T00:07:54+10:00"
scope: "local"
source: "components/fpga/kest_fpga_dma.c:10-120"
---
Status: Green

`kest_fpga_dma.c` defines periodic read specifications for FPGA memory and a callback that updates a memory slot from readback. `kest_begin_periodic_read` returns NO_ERROR immediately before timer creation, leaving its timer path unreachable; LFO/memory polling integration needs review. Source: components/fpga/kest_fpga_dma.c:10-120
