---
status: "unverified"
created_at: "2026-09-20T00:07:13+10:00"
scope: "local"
source: "components/core/kest_resource.c:315-500"
---
Status: Green

`kest_lfo_evaluate_rec` advances phase from elapsed milliseconds, evaluates center/amplitude or min/max expressions, supports linear or logarithmic scale and returns a sinusoid. Activation resets time/phase; async activation returns ERR_FEATURE_DISABLED immediately, deactivation sync returns immediately, and UI timer update returns immediately. Scheduling of ongoing LFO updates is therefore visibly incomplete. Source: components/core/kest_resource.c:315-500
