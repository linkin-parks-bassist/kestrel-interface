---
status: "unverified"
created_at: "2026-09-20T00:10:12+10:00"
scope: "local"
source: "observed make lib output; components/core/kest_preset.c:611-626; Makefile"
---
Status: Green

A `make lib` attempt on 2026-09-20 compiled multiple objects, then failed in components/core/kest_preset.c:621: `kest_preset_handle_name_change` references `preset->sequence`, but `kest_preset` has no `sequence` member in the KEST_LIBRARY configuration. The same branch calls queue-save functions without declarations, producing warnings. This is a source build failure, distinct from the host SDL2 prerequisite for desktop tests. Repair the library guards or provide a library-safe name-change path, then rerun make lib. Source: observed make lib output; components/core/kest_preset.c:611-626; KEST_LIBRARY build configuration.
