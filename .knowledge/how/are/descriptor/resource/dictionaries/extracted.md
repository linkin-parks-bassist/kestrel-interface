---
status: green
revised_at: "2026-10-06T11:45:20+11:00"
---

kest_dict_extract.c converts kest_eff_entry_dict dictionaries into parameters, settings, memory, delay, filter, LFO and polynomial resources; unused legacy extraction is removed and installed, with host/boot/reload checks passing. Low/high/band-pass filters require numeric cutoff, accept Q or resonance (default 1/sqrt(2)), and attach coefficient expressions to filter payloads. Authored allocated LUTs are unsupported and excluded by David: type "lut" is rejected. Built-in sine/tanh use handles 0/1; Core owns limitations.

Wrappers use matching typed allocators/releases; names, option arrays and payloads remain heap-backed beyond parser-arena reset. Parameter/setting pooling is installed. Resource-template pooling passes 205 host tests/heap-fault probes; installed Swamp reloads retain 46 slots (/tmp/kestrel-resource-template-live.log). One-slot tests cover retention, exhaustion, rejection, reuse and partial polynomial cleanup.

MEM accepts only type; other dictionary attributes reject. Extraction enables periodic reads; kest_mem_slot_create defaults read_period_ms to 10. An .eff cannot currently disable private-state polling or set its period. Account for this in state-heavy carrier qualification; these defaults do not establish measured UI cost.

Resource type requires a string tag and nonnull value before union interpretation. Invalid tags/null strings report one error and reject. Regressions cover expression/dictionary/list payloads resembling "mem" and null strings; /tmp/kestrel-resource-type-{before,after}.log preserves the reproduced wrong-tag failure.

ASSERT_ATTR guards reject invalid tags, missing payloads, nonconstant values and fractional integer fields. The repaired macro stops extraction; four parameter cases over three passes preserve borrowed references without drivers (/tmp/kestrel-attribute-guard-{before,after}.log).

Parameter allocation failure releases wrapper/name/units/new drivers, restores driver count and newly created list storage; failed append releases its unattached payload. Earlier drivers/borrowed keys survive. Six allocation points over three recovery passes restore heap counts and borrowed expressions (/tmp/kestrel-parameter-allocation-{before,after}.log). Setting failure releases all strings including units; four allocation points, three unknown-attribute rejections and recovery preserve expressions/bounds (/tmp/kestrel-setting-allocation-{before,after}.log, /tmp/kestrel-setting-units-before.log).

Rejected memory/delay/LFO frees payloads. Polynomial rejection frees coefficient-pointer array/filter payload while preserving borrowed expressions. Three rejection cases/type restore heap counts (/tmp/kestrel-resource-payload-after.log). Resource-list and reader failures release unattached containers; section/arena owners hold bounded fault evidence and fresh-tree reclamation. Generated/shared expression graphs remain unqualified.

Polynomials use type: "polynomial", coefs: {0.125,0.25,-0.125}, constant first. kest_filter has feed_forward=count, feed_back=0; explicit KEST_DSP_RESOURCE_FILTER enables allocation/programming. Syntax: poly audio $resource destination. polynomial-state.eff checks types/counts/instructions; superproject test_eff_poly.py checks allocation/coefficient words and all 65536 signed16 inputs through actual unit/core. Core owns fixed-point/static-renderer limits; installed identity/carrier evidence has separate ownership.

Sources: components/parser/kest_dict_extract.c, components/core/kest_resource.c, production encoder, fixtures and allocation probes.
