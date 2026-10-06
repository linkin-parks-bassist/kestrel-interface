---
status: green
revised_at: "2026-10-05T22:54:49+11:00"
---

components/ui/kest_preset_view.c creates preset pages and implements effect click/move/delete, save/name, play/navigation and button-index/appearance updates.

init_preset_view allocates its data and settings-page wrapper with kest_alloc, matching kest_free in constructor failure cleanup. The wrapper previously used raw malloc, incompatible with tracked-heap release. The constructor repair is flashed; current installed qualification belongs to what/firmware/is/installed/on/the/carrier.md. free_preset_view still returns ERR_UNIMPLEMENTED, so this repair does not establish complete page teardown or allocation-failure rollback.

Play calls preset or containing-sequence activation. Installed capacity rejection preserves the running configuration and opens a closable 'Cannot play preset' explanation asking the user to remove an effect. Host activation tests cover model rejection; carrier Play rejection shows the popup and retains the previous running preset (/tmp/kestrel-parent-ui-live.log). Sequence-member page creation preserves preset->sequence->view_page as parent, matching configuration; it no longer assigns the page to itself. A constructor regression checks this link. The repaired inactive reload ancestor walk completes physically; complete page teardown remains separate.

The bottom plus opens the loaded-effect selector. Normal effect deletion uses long-press to reveal trash, release, then tap trash before its timer expires. It removes the preset instance, not the descriptor file. The scripted carrier flanger fixture verifies ordinary activation and active deletion followed by return to baseline typed-pool counts and original preset restoration. The UART procedure owns that script's current layout/preconditions and evidence.

Sources: components/ui/kest_preset_view.c, kest_button.c, tests and /tmp/kestrel-scripted-flanger-hil.log.
