---
status: green
revised_at: "2026-10-06T09:30:46+11:00"
---

Deferred preset/sequence saves borrow pointers without retaining owners. Context deletion disposes of them; sequence serialization traverses and can save referenced presets. Saves run without the UI/model lock after David directed rollback of the rendering regression. The file-task owner holds measured SD-lock evidence and the deferred maintained-byte design. Borrowed-owner lifetime/concurrent edits remain unresolved; do not silently repair them by blocking rendering through SD I/O.

Preset/sequence serializers do not check every write, seek or close. Backup/recovery renames are unchecked; ordinary save_preset uses the direct serializer. Its NULL guard does not settle storage/lifetime questions.

State decoding checks streaming reads and publishes only a complete record. Tests reject every truncated prefix and overlong filenames without modifying the caller.

Preset decoding stages name/effects, checks counts/IDs/parameter and int32 setting reads and bounded strings, and publishes after complete reads and successful close. It avoids editing-time save/updater notifications. Failure reclaims unpublished effects/list/name and preserves the destination. Tests cover every populated prefix, typed resource-slot return, complete readback identity/control values, exhausted parameters and missing descriptors over reuse cycles.

Sequence decoding stages name/reference nodes. Failure preserves destination and backlinks; success appends doubly linked references in order, publishes backlinks and skips missing presets. Tests cover truncated minimal/populated records, overlong names, missing references, existing-list joins, bidirectional playback and malformed header rollback. Carrier HIL covers forward/backward activation and boundary no-ops; malformed SD injection remains unqualified. Installed-image and unit-test owners distinguish evidence scopes.

Pipeline append rejects reported constructor failures and reclaims unpublished owners/nodes. Parameter/setting clone-list failures and partial resource/settings exhaustion return typed slots. Scope insertion failure rejects construction and destroys its temporary dictionary. Further direct/stack constructor rollback, dependency/driver allocation, destination replacement, general sequence destruction, trailing data and physical write failures remain qualification questions.

FPGA programming has bounded outer retries but no explicit bound on while(status.swapping). This differs from best-effort latest-value scratchpad reads. Persistence/programming fault injection remains unqualified.

Sources: components/core/kest_file_task.c, kest_files.c, kest_context.c, kest_preset.c, kest_sequence.c, tests/core/kest_files_test.c and components/fpga/kest_fpga_comms.c.
