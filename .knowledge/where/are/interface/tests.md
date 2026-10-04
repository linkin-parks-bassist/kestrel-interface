---
status: green
revised_at: "2026-10-04T13:10:39+11:00"
---

tests/core contains arena, dictionary, pool, block, string/list, numeric encoding/policy, updater readback/retirement, resource-clone failure, preset-save null and preset/state persistence coverage. tests/ui covers parameter widgets. tests/parser/kest_readback_effect_test.c checks the readback, SVF and polynomial fixtures, explicit shift syntax and rejection of three expression operands for two registers; tests/fixtures contains their .eff inputs, including an invalid polynomial list used to check error reporting without a crash.

From the Interface directory run make tests and ./kest_tests. The current suite passes 154 tests. Test objects depend on tests/kest_test.h, main/kest_int.h and application headers, so ordinary make tests rebuilds them after production-header changes; forced rebuilds are no longer required for that case.

Sources: tests inventory, Makefile test rule and executed suite. Detailed coverage and limits belong to what/do/interface/unit/tests/cover.md.
