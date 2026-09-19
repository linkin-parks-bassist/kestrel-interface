---
status: "unverified"
created_at: "2026-09-20T00:04:02+10:00"
scope: "local"
source: "components/core/kest_files.c:82-103,272-317"
---
Status: Green

`save_state_to_file` writes magic and unfinished status, input/output gains, active preset and sequence filenames, and the current page identifier; it then marks status finished. `file_validity_check` checks magic and finished bytes. Source: components/core/kest_files.c:82-103,272-317
