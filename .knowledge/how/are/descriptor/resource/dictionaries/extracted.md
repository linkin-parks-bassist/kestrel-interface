---
status: green
revised_at: "2026-10-04T13:09:36+11:00"
---

kest_dict_extract.c converts descriptor dictionaries into parameters, settings, memory, delay, filter, LFO and polynomial resources. Low/high/band-pass filter extractors require numeric cutoff, accept Q or resonance (default 1/sqrt(2)), build coefficient expressions and attach filter resource data. Several allocation sites retain TODOs to move parser-created objects into the parser arena.

A polynomial uses type: "polynomial" and a curly-braced coefficient list, for example coefs: {0.125, 0.25, -0.125}, ordered constant term first. Its payload is the existing kest_filter structure with feed_forward equal to coefficient count and feed_back zero. The resource is explicitly KEST_DSP_RESOURCE_FILTER so existing allocation/coefficient programming runs; merely attaching the payload without its type produces an instruction body with no coefficient setup. Instruction syntax is poly audio $resource destination.

tests/fixtures/polynomial-state.eff and its parser regression check resource types, coefficient counts and polynomial instructions. Superproject tools/test_eff_poly.py additionally checks the emitted allocation/coefficient words and all 65,536 signed16 inputs through the actual polynomial unit and core. The Core filter owner specifies fixed-point power truncation and the renderer's static-programming limits.

Source: components/parser/kest_dict_extract.c, production encoder and checked fixtures.
