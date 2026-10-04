---
status: green
revised_at: "2026-10-04T04:09:18+11:00"
---

components/ui/kest_preset_view.c creates a page for a preset, handles effect click/move/delete callbacks, save/name controls, play and navigation controls, and updates effect button indices and play/save appearance.

The bottom plus button opens the loaded-effect selector. To remove an effect through normal UI input, long-press its active button to reveal the trash button, release, then tap the trash button before its visibility timer expires. The delete callback removes that effect from the preset; this is not deletion of its .eff descriptor file. Phantom input verified this gesture on a newly created test preset on the carrier.

Sources: components/ui/kest_preset_view.c, components/ui/kest_button.c; /tmp/kestrel-readback-activation-hil.log.
