---
status: "unverified"
created_at: "2026-09-20T00:03:23+10:00"
scope: "local"
source: "components/core/kest_effect.c:112-343"
---
Status: Green

`init_effect_from_effect_desc` initializes the instance, clones resources and blocks, clones parameters/settings, creates the effect scope, then relinks resource references in cloned blocks. Resource linkage waits until scope and instance-specific resources exist. Source: components/core/kest_effect.c:112-343
