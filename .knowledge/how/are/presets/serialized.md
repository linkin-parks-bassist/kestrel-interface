---
status: green
revised_at: "2026-10-04T06:09:53+11:00"
---

save_preset_as_file writes a preset magic byte and initially unfinished status, the display name and effect-list node count, then each effect's descriptor cname, ID, parameter floats and setting int32 values. A missing effect or description is represented by KEST_PRESET_BROKEN_EFFECT. It seeks back to mark the status finished after writing.

save_preset assigns a filename if needed and calls this writer directly. It rejects NULL with ERR_NULL_PTR before dereferencing or accessing files. tests/core/kest_files_test.c exercises this contract; the unfixed test terminated with SIGSEGV and the current suite passes.

Empty-preset round-trip coverage verifies an explicit name and the Unnamed Preset fallback, the empty pipeline, restored filename/has_fname and cleared unsaved_changes. Files are isolated temporary host files. Populated effects/settings and deferred-save concurrency are not established by that test.

The writer traverses live model lists and does not lock or take a snapshot. Individual fputc/fwrite, final fseek and fclose results are not checked, so a success return does not establish every byte reached storage. Filename assignment and serialization are distinct from the queued job's pointer-lifetime contract, owned by how/does/the/file/task/queue/persistence.md.

Sources: components/core/kest_files.c, tests/core/kest_files_test.c, /tmp/kestrel-persistence-null-red.log and /tmp/kestrel-persistence-roundtrip-tests.log.
