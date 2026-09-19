---
status: "unverified"
created_at: "2026-09-20T00:02:29+10:00"
scope: "local"
source: "components/core/kest_update.c:1015,1075"
---
Status: Green

`kest_updater_generate_command_list` appends allocations, instruction writes, register writes, then filter coefficient writes. It appends one filter commit per distinct updated filter, a register commit if register updates exist, and tail enable if total allocated delay exceeds KEST_TAIL_CUTOFF. Expression-backed values are evaluated when converting staged writes to commands. Source: components/core/kest_update.c:1015,1075
