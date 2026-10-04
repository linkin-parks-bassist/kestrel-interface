---
status: green
revised_at: "2026-10-04T13:10:40+11:00"
---

The intended firmware work is governed by what/is/the/spec.md. David prioritizes deprecated-code cleanup/allocation integration, then USB UART control/diagnostics, followed by the remaining work in its existing order. Acceptance details remain partial. Add focused regression coverage alongside changes even where broader test-suite expansion is staged later.


1. Complete useful integration of preallocated typed pools and the existing allocation strategies, plus remaining deprecated-code cleanup. Put the allocator selection to use where it saves repeated heap work for known important structs; preserve the existing architecture and conventions.
2. Extend useful USB UART controls and diagnostics: filesystem operations for .effs, ordinary UI input for full-system HIL, and practical device-driving utilities. Add focused checks for the behavior being delivered; broad failure-path qualification is not a prerequisite for capability work.
3. Extend C regression coverage beyond the current suite, prioritizing parser/compiler and allocation contracts needed by cleanup/loading work. Extend truncated/malformed preset and sequence input coverage and repair their decoder bounds; extend populated-preset and sequence round trips. Retain the pinned embedded SDK baseline.
4. Implement background .eff discovery/loading at startup with a reusable flush/repopulate operation for SD swaps or clears. Reuse the existing loading and UI machinery, and discuss any necessary architectural change with David.
5. Extend the working host-compiler/sample-model/actual-core loop beyond arithmetic/SVF/built-in-LUT/scratchpad/delay/static-polynomial to live coefficient updates, remaining allocated LUT/resource programming and full one-pipeline transport/controller coverage. Extend verified USB descriptor transfer and startup discovery to automated UI activation and physical numerical/audio checks under the superproject specification.
6. Carry out the existing integrated-board firmware requirements: display/touch initialization, footswitch behaviour, controlled SPI enable, backlight PWM, processor revision configuration and audio-backend migration, with circuit-specific qualification.

Capability-aware instruction acceptance is planned alongside the active RTL work: define automatic partner probing and unsupported-instruction rejection or applicable classic-biquad lowering with David. The shared protocol and Core filter owners govern the address map, bit assignments and finite SVF conversion limits. This does not reorder the prior firmware priorities.

Setlists are an additional planned feature with priority not yet assigned: define their progression, editing, persistence and sequence-reference lifecycle, then implement and test the sequence-of-sequences layer governed by what/is/the/spec.md. Preserve the numbered ordering above until David places this feature.

Boot splash screen is a future firmware feature with no assigned priority. Define its appearance and startup/dismissal behavior before implementation; preserve the numbered order. Frame-rate tuning remains deferred.

Horizon ticket: use the symbolic .eff instruction-sequence representation David reports is now available to reorder instructions for the actual hardware. Establish dependency and observable-effect constraints, model the relevant pipeline/operand readiness costs, then verify equivalence and compare cycles, stalls, throughput and emitted SPI traffic against the original ordering. Inspect the current representation before choosing a scheduling mechanism. This is deferred, with no assigned priority; preserve the numbered ordering above.

Preserve smoothing as a separate 100 Hz task and consult David before architectural changes. David prioritizes quick capability return per token and time over revisiting lifecycle considerations he deliberately left out. Broader queued-ID, teardown, concurrent-edit and stale-save hardening is deferred unless a concrete failure obstructs planned work. Fix such failures proportionately; do not expand them into a framework or a mandatory cleanup programme.

The maintained-file-buffer persistence design is a future direction: saveable objects keep file bytes in memory and mutate them on changes. Its file-task owner records the remaining design questions. It does not block the feature work above.

Revisit other source-backed defects when the owning feature needs them; the source inventory does not authorize additional product features. Keep knowledge and focused validation current with implementation.
