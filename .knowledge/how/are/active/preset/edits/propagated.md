---
status: green
revised_at: "2026-09-20T00:03:23+10:00"
---

Appending, removing or moving an effect in a preset modifies its pipeline; if the preset is active, it notifies the updater to reprogram. Outside KEST_LIBRARY it also queues a preset save. `kest_preset_set_active` marks active and pending and notifies the updater. Source: components/core/kest_preset.c:137-165,256-326
