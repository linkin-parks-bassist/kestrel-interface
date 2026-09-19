---
status: "unverified"
created_at: "2026-09-20T00:04:03+10:00"
scope: "local"
source: "components/core/kest_file_task.c:25-140; kest_files.c:760-818; components/fpga/kest_fpga_comms.c:100-180"
---
Status: Green

The file job queue stores preset and sequence pointers for deferred use, so their lifetime matters until processed. `safe_file_write` does not check rename success. Core communication has bounded program retries and flag-based recovery. These are source-level boundaries; no fault-injection verification was run. Source: components/core/kest_file_task.c:25-140; kest_files.c:760-818; components/fpga/kest_fpga_comms.c:100-180
