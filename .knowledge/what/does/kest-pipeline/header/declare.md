---
status: green
revised_at: "2026-10-05T22:43:21+11:00"
---

components/core/kest_pipeline.h declares pipeline initialization, effect append/move/remove/count/lookup, cloning, private reload staging/discard, capacity preflight, transfer compilation, ID rectification, LFO activation/deactivation and FPGA/position updates. kest_pipeline_check_capacity accepts a pipeline and additional block count, returning an error if existing or proposed blocks exceed capacity. The pipeline implementation owner governs behavior and ownership. KEST_INT_PIPELINE_H_ is the include guard.
