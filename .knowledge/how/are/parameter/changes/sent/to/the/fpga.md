---
status: green
revised_at: "2026-10-04T05:55:35+11:00"
---

The separate 100 Hz parameter-smoothing task advances target values and queues parameter notifications. The 100 Hz control task collects parameter, scope-entry and preset notifications, consumes latest memory arrivals, computes dependent register/filter/instruction writes and resource allocations, then emits FPGA command lists. Smoothing retains its original task boundary by David's explicit direction.

With UI enabled, each control tick holds the existing recursive UI lock while borrowing model lists and objects and releases it before sleeping, including a rejected-program retry. This serializes ordinary UI model edits and final UI reclamation with control work. It does not protect the separate smoothing task, persistence jobs or arbitrary background model mutations; their lifetimes remain qualification questions.

Sources: components/core/kest_update.c and kest_param_update.c; David's rollback direction. Focused smoothing and pipeline lifecycle owners govern detailed contracts.
