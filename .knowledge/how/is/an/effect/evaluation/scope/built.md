---
status: "unverified"
created_at: "2026-09-20T00:03:23+10:00"
scope: "local"
source: "components/core/kest_effect.c:723-854"
---
Status: Green

`kest_effect_create_scope` inserts parameters, settings, named definitions, then memory and LFO return entries; links resource readback pointers to those entries and detects dependencies. It sets `effect->scope` only on success. The failure path explicitly has a TODO for cleanup, and its comment warns that calling it when a scope already exists leaks memory. Source: components/core/kest_effect.c:723-854
