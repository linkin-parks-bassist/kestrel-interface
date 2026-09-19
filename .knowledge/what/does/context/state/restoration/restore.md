---
status: "unverified"
created_at: "2026-09-20T00:04:02+10:00"
scope: "local"
source: "components/core/kest_state.c:11-145"
---
Status: Green

`kest_cxt_clone_state` captures input/output gains, active preset and sequence filenames, and current UI page under the context mutex. `kest_cxt_restore_state` looks up and activates the named preset and sets the two gains; `kest_cxt_enter_previous_current_page` separately finds and enters the saved page. The shown restore function does not itself activate the saved sequence. Source: components/core/kest_state.c:11-145
