---
status: green
revised_at: "2026-10-06T00:37:53+11:00"
---

Yes, in source and installed carrier firmware. An enum requires a nonempty options dictionary. Each named entry contains exactly name (display string) and value (constant integer), for example options: (quarter: (name: "Quarter", value: 4), dotted: (name: "Dotted eighth", value: 9)). Declaration order is dropdown order; keys are not labels.

Values must be unique, inside min/max and 0–32767, matching signed widget selection. Labels must be unique, nonempty, shorter than 128 bytes and contain no CR/LF. Default must name a choice. Missing/invalid fields, duplicate values/labels, fractional/nonconstant values and options on non-enums reject. All settings require finite, integral, int-representable default/min/max, ordered bounds and an in-range default; checks use evaluated floats before conversion.

The descriptor owns one packed array/label allocation. Clones copy the array and borrow labels while retaining the descriptor. Retirement waits for instances. Extraction failure, failed insertion and reader rejection release it. No new dependency or ownership flag.

204 host tests pass; source setting wrappers use the typed pool with matched release. One-slot tests qualify exhaustion, retained labels after arena reset, rejection and reuse. Setting-pool integration is installed; three RHYTHM reloads preserve values/four setting slots (/tmp/kestrel-setting-pool-live.log). Fixtures qualify values/order, actual LVGL selection, parser reset/retired-descriptor label lifetime, repeated malformed rejection/recovery and numeric bounds. Five heap-allocation failures over three passes retain no heap; descriptor allocation rejection releases choice metadata. Actual dropdown events qualify changed active rebuild requests, unchanged suppression and inactive dirty-only edits. Sources: tests/parser/kest_readback_effect_test.c, tests/ui/kest_parameter_widget_test.c, tools/test_parser_allocation.c, /tmp/kestrel-enum-options-{tests,allocation}.log and /tmp/kestrel-setting-rebuild-tests.log.

The superproject RHYTHM owner specifies its provisional tempo/subdivision vocabulary/range and descriptor qualification. Delay-sizing owner qualifies instance-setting reevaluation. RHYTHM carrier checks qualify integer entry, enum selection and active reload preservation; its owner holds evidence. Physical delay timing and broader migration remain unqualified; installed-image owner holds identity.
