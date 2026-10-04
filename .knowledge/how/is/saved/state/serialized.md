---
status: green
revised_at: "2026-10-04T06:14:36+11:00"
---

save_state_to_file writes magic and unfinished status, input/output gains, active preset and sequence filenames, and the current page type, ID and filename. It then marks status finished. file_validity_check checks magic and finished bytes.

load_state_from_file preserves this byte format and uses checked streaming reads into a local kest_state. Float values and the two int32 page fields are read into aligned objects. read_state_filename accepts a NUL only within the destination array capacity, including empty strings and 31-character names in the 32-byte fields. Incorrect magic, unfinished status, missing bytes, overlong/missing-terminator filenames or failed close return ERR_MANGLED_FILE. NULL arguments return ERR_NULL_PTR and an open failure returns ERR_FOPEN_FAIL.

The caller's state is assigned only after the complete record is accepted; rejected input leaves it unchanged. The decoder has no temporary heap buffer and closes the file on all opened paths. Trailing data is ignored as before; numeric semantic/range validation is not added.

Host coverage verifies normal gains/page identity, all three 31-character filename fields, empty filenames, corrupt magic and unfinished status, every one of the 114 truncated prefixes of the valid fixture, and overlong filenames in all three fields. The pre-fix truncation test failed against the old loader. These tests do not inject filesystem transport/close errors or prove physical interrupted-write behavior. Writer error checking remains incomplete.

Sources: components/core/kest_files.c, kest_state.h, kest_page_id.h, tests/core/kest_files_test.c, /tmp/kestrel-state-bounds-red.log and /tmp/kestrel-state-bounds-tests.log.
