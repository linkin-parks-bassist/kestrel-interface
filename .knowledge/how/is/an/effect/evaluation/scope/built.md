---
status: green
revised_at: "2026-10-04T01:58:18+10:00"
---

kest_effect_create_scope inserts parameters, settings, named definitions, then memory and LFO return entries; links LFO scope-entry pointers and detects dependencies. Memory scope entries point to slots whose latest sample is atomically published by SPI read callbacks. Evaluation atomically loads that sample; the control-loop updater consumes a coalesced arrival flag to propagate dependencies. There is no queued token/result reply or stored periodic-read scope-entry pointer.

The function sets effect->scope only on success. The failure path explicitly has a TODO for cleanup, and its comment warns that calling it when a scope already exists leaks memory.

Sources: components/core/kest_effect.c, kest_expr_scope.c, kest_resource.c and kest_update.c.
