---
status: "unverified"
created_at: "2026-09-20T00:04:02+10:00"
scope: "local"
source: "components/core/kest_files.c:760-818"
---
Status: Green

`safe_file_write` renames an existing target to a `.bak` sibling, invokes the writer on the target path, and on a returned write error removes the new target and renames the backup back. On success the `.bak` remains until a later write replaces it. This protects only errors returned by the writer; it does not check every rename result. Source: components/core/kest_files.c:760-818
