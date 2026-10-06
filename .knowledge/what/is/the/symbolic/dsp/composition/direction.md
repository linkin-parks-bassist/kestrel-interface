---
status: green
revised_at: "2026-10-05T22:54:52+11:00"
---

David wants channel-agnostic .eff programs with symbolic values, register allocation/scratchpad spills and eventual userland DSP graphs. Full graphs are V2.0; near-term priority is quality effects preserving the existing simple, beautiful, usable linear UI. He reports developing pre-encoding symbolic representation before his job; inspect block/operand/batch machinery first. The logo/graph specification owns named inputs, 1–4 outputs and grid interaction.

Per-effect band application, previously working on deprecated Teensy, is a concrete compiler consumer: select all/above/below/band plus wet/dry; process the selected band and recombine its blend with the clean complement. Existing initialization/UI is scaffolding, not DSP splitting/recombination.

Establish values/ports/transformations/resource/state ownership; implement liveness/allocation/spills and lower wrappers/later graph edges. Define explicit-channel compatibility, updates, scheduling dependencies, spill hazards, budgets, fan-in headroom, feedback/state and sample latency. Verify equivalence, bypass/complementary reconstruction and sound. Preserve capability/minimal dependencies; infrastructure does not block library work.

Admission must reject capacity/time overflow before publication, preserving the running configuration. Installed compilation and preset append share 256-slot checks; the selector explains rejected additions without changing the preset/page. Pipeline/selector/installed-image owners govern host and physical evidence. Installed activation/sequence admission rejects oversized loaded presets before flags, context pointers or cursors change; host tests cover the entry points. Carrier Play/Add Effect rejection is qualified; physical sequence overflow, queue-failure rollback and other edit boundaries remain planned. Direct private pipeline construction remains distinct from admission.

Worst-case timing is open. David proposes MCU simulation of one sample with conservative arbitration/SDRAM bounds: last-served reads and maximum reachable refresh stalls. Include dependencies, operations, spills, concurrent clients and cross-sample state; bound overlap/starvation. Uncached/nonburst reads waste time. Slot counts/fixed-input maxima are not worst-case proofs. Define fidelity, bounds, margin, MCU cost/diagnostics and validate adversarial RTL/full-system/physical timing. Priority/model details remain open.
