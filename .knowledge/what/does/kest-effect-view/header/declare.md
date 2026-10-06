---
status: green
revised_at: "2026-10-04T23:49:02+11:00"
---

components/ui/kest_effect_view.h declares the effect-view page data and initialization, creation, configuration, refresh, navigation and cleanup entry points. Its guard is KEST_INT_EFFECT_VIEW_H_. The orphan Teensy parameter-read declaration is removed; current readback belongs to the resource/control/SPI path.

Sources: components/ui/kest_effect_view.h and kest_effect_view.c. Behavior and ownership belong to what/does/kest-effect-view/implement.md.
