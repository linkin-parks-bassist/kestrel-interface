---
status: green
revised_at: "2026-10-04T05:55:36+11:00"
---

components/core/kest_update.h declares the control-loop task, its update queue and notifications for parameters, presets and scope entries. Scratchpad read completions directly publish the memory-slot sample and arrival flag; they do not enter this update queue. Updater state contains the current resource list and an unsigned tick counter, with no read generation or request records.

Public helpers cover state init/clear/destroy, list draining, resource/update/preset/scope-entry handling, command-list and transfer-batch generation, sending, standalone program generation and debug printing. KEST_UPDATE_* types, READY/REPROGRAM states, write types and allocation constants belong here.

Source: components/core/kest_update.h.
