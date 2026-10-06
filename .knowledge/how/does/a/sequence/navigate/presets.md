---
status: green
revised_at: "2026-10-05T22:54:49+11:00"
---

kest_sequence_begin starts at the first preset; begin_at finds a member. Regress/advance require an active nonempty sequence and do nothing at ends. Installed navigation calls set_active_preset_from_sequence before committing the new cursor; begin/begin_at similarly admit the target before changing active/global-sequence state. Capacity rejection propagates and preserves the running preset, old sequence pointer and cursor. Host tests cover all four paths. Preset Play capacity rejection is physically exercised; sequence next/previous overflow and queue-failure rollback remain unqualified. Sequence-member preset pages now retain the containing sequence page as parent; the previous self-parent cycle caused reload to hang and is repaired/host-tested/physically checked through inactive reload.

stop clears sequence/current preset; stop_from_preset clears sequence without changing the preset. read_sequence_from_file stages bidirectional links and joins accepted references to the destination tail, skipping missing references. Host persistence tests navigate joins and reject incomplete prefixes without changing ownership.

Sequence-page Play starts an inactive sequence; member-preset Play calls begin_at. UART sequences reports membership/current/active flags; sequence-step next/prev invokes navigation under the UI lock without selecting/starting a sequence. Earlier carrier_sequence_selection.json evidence covers 25 steps through order 1/2, endpoints and ordinary UI selection, returning pools to baseline (/tmp/kestrel-sequence-step-boundaries-hil.log). It does not qualify new capacity rejection, footswitches or other sequences.

remove_preset detaches/queues save. delete_preset rejects null/removal failure, delegates global deletion to cxt_remove_preset for unlink/file deletion/slot return, and frees nonglobal presets without duplicate save. Context tests check one reference, unrelated preset, file removal and slot return. Other references, saves and general linked maintenance remain unqualified.

Sources: core sequence/files/context, UI sequence/preset callbacks, tests/core and carrier_sequence_selection.json. Installed identity belongs to its owner.
