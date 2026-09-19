---
status: "unverified"
created_at: "2026-09-20T00:07:13+10:00"
scope: "local"
source: "components/core/kest_pipeline.c"
---
Status: Green

`kest_pipeline.c` owns the linked ordered effect list. Append instantiates from a descriptor and assigns a new ID; remove disables/frees by ID; move relinks nodes by position. It can clone, count, build an FPGA transfer batch, rectify IDs, activate DMA/LFOs and recalculate FPGA positions. Source: components/core/kest_pipeline.c
