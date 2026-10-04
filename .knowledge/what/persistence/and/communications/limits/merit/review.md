---
status: green
revised_at: "2026-10-04T06:14:36+11:00"
---

Deferred preset/sequence saves hold borrowed pointers without retaining or protecting their owners. Context deletion immediately frees those owners, so a queued save can dereference freed storage; sequence serialization also traverses referenced presets. File-task serialization does not hold the UI/model lock. The file-task owner describes this source-established gap and the architectural decision required before repairing it.

Preset/sequence serializers do not check each write, seek or close result. safe_file_write and save_preset_as_file_safe do not check backup/recovery rename success; save_preset normally calls the direct serializer rather than the safe wrapper. The current NULL preset guard prevents one invalid-input crash, but does not solve these storage/lifetime questions.

State loading uses checked streaming reads into a local value and only publishes a complete accepted record. Host coverage rejects every truncated prefix of the state fixture and overlong filenames, leaving the caller unchanged; normal state and empty-preset round trips pass. Preset/sequence decoder bounds and write failures still need qualification. These host checks are not physical fault injection.

FPGA programming has a bounded outer retry loop and flag-based handling, but its while(status.swapping) wait has no explicit bound. This is distinct from scratchpad reads' best-effort latest-value callback model. No physical fault-injection qualification was run for these persistence or programming failure cases.

Sources: components/core/kest_file_task.c, kest_files.c, kest_context.c, kest_preset.c, kest_sequence.c and components/fpga/kest_fpga_comms.c.
