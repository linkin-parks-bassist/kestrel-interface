---
status: green
revised_at: "2026-10-06T09:30:27+11:00"
---

save_preset_as_file writes magic/status, display name and effect count, then descriptor cname, ID, parameter floats and int32 settings per effect. Field names/counts and instance wet-mix/band controls are absent. Decoding derives field order/count from the current descriptor. Adding settings can cause truncation rejection; rearranging fields can silently misinterpret values. Offline migration needs the original schema. Preserve the old descriptor until live instances migrate and save, or convert with a known original schema. Missing descriptors reject loading; startup skips rejected presets.

The reader checks header/status, bounded strings and every count/ID/value read. It stages name/effects and publishes only after complete reads and successful close. Failed reads reclaim unpublished instances and preserve the destination. Broken-effect writer handling remains incompletely qualified.

Host round trips cover empty named/fallback presets, production readback, signed settings 0x01020304/-123456789, ID 37, parameter 0.625 and resources. Every populated prefix rejects without publication/slot retention. Truncated/overlong names and exhausted parameters/missing descriptors preserve populated destinations. The unit-test owner governs coverage; these are host files, not physical SD faults.

RHYTHM carrier evidence establishes one schema recovery: added settings caused Preset 8 to be skipped; the old descriptor recovered /sdcard/pre/O13N.PRE. Live reload migrated and queued its save; fresh startup retained nine presets/four main members and two setting slots. The RHYTHM owner holds logs and physical limits; this is not general offline compatibility.

Writer assigns filenames, traverses live lists, seeks back to mark completion and closes. Individual write/seek/close results remain unchecked. The FreeRTOS file task runs saves without the UI/model lock; David directed reverting the added lock after measured SD writes blocked rendering. Borrowed-pointer lifetime and concurrent edits remain unresolved. The file-task owner governs that boundary and deferred maintained-file-byte design.

Sources: components/core/kest_files.c, kest_file_task.c, tests/core/kest_files_test.c and tests/fixtures/readback.eff, preset-settings.eff; /tmp/kestrel-rhythm-recovery-{old-live,migrate-upload,new-live}.log.
