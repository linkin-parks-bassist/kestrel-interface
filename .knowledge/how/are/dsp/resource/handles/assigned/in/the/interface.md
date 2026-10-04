---
status: green
revised_at: "2026-09-20T00:07:13+10:00"
---

`kest_resources_assign_handles` gives separate increasing handle spaces to delays and filters; memory handles advance by each resource mem_size, and memory slot addresses are set. Resource code also creates/clones filters, memory, delay and LFO instances. Source: components/core/kest_resource.c:47-315
