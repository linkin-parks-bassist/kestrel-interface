---
status: "unverified"
created_at: "2026-09-20T00:04:01+10:00"
scope: "local"
source: "components/core/kest_files.c:105-204"
---
Status: Green

`save_preset_as_file` writes a preset magic byte and initially unfinished status, the display name, effect count, then each effect cname, ID, parameter float values and setting int32 values. A missing effect description is represented by KEST_PRESET_BROKEN_EFFECT. It seeks back and marks the status finished only after writing. Source: components/core/kest_files.c:105-204
