---
status: "unverified"
created_at: "2026-09-20T00:03:24+10:00"
scope: "local"
source: "components/core/kest_sequence.c:293-475"
---
Status: Green

`kest_sequence_begin` starts at the first preset; `begin_at` selects a member; regress/advance require an active nonempty sequence and move to adjacent list nodes, doing nothing at ends. These calls activate the selected preset through `set_active_preset_from_sequence`. `stop` clears current sequence and active preset, while `stop_from_preset` clears sequence state without changing the preset. Source: components/core/kest_sequence.c:293-475
