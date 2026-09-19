---
status: "unverified"
created_at: "2026-09-20T00:07:11+10:00"
scope: "local"
source: "components/core/kest_driver.c:19-146"
---
Status: Green

A scope-entry driver stores a key; `kest_driver_evaluate` resolves and caches its scope entry on first use, then evaluates the entry in a supplied scope. Clone functions copy the driver but reset cached scope/entry pointers for the destination effect. Source: components/core/kest_driver.c:19-146
