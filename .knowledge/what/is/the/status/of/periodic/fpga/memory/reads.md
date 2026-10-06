---
status: green
revised_at: "2026-10-05T16:42:10+11:00"
---

Scratchpad readback follows David's latest-value model. kest_fpga_queue_mem_read copies an address/data/callback into the SPI queue without waiting or per-read allocation. The callback atomically publishes signed data/arrival; transport errors preserve the last good sample.

The 100-Hz control loop coalesces arrivals and propagates dependencies. Enabled positive read_period_ms rounds up to 10-ms ticks; shorter periods attempt each tick. REPROGRAM suppresses processing; missing/rejected arrivals wait for ordinary future attempts. There are no correlation tokens, deadlines or retry records. LVGL DMA remains absent; LFO timers are independent.

Callback data remains borrowed through completion. Deletion markers stop work/ignore arrivals; control retains owners until an ordered SPI callback completes, then UI reclamation cancels callbacks/timers. Other owners govern remaining lifetime questions. Creation defaults to 10 ms; the legacy extractor defaults to 7 ms and accepts read_ms, while kest_extract_mem accepts only type. Clones clear pending arrival.

Prior KTPROBE/readback.eff HIL checked default 8192 and changed signed values/direct reads, plus opened-view/settings-page active deletion/re-add without observed panic/reboot. Its SD probe was removed; source remains. UI labels are rounded. Cadence and complete queued-ID/save/audio behavior remain unqualified.

The silent KTPOLY fixture computes input 0.5 against {shape,0.25,-shape}, adds a separate -0.25 polynomial, writes scratchpad zero and zeros audio c0. Shape 0.125/+0.5/-0.5/0/+0.5 gives signed -1024/8192/-16384/-4096/8192 and direct unsigned 64512/8192/49152/61440/8192. UART-target and touch testing on earlier Interface images match exactly with clean status 0x01. This checks unchanged coefficients/other-handle preservation across commits; it does not qualify every format/degree, pacing, transient continuity or audio quality.

Repeat with tests/fixtures/KTPOLY.EFF (374 bytes, SHA-256 7a14b1e62d5ea010e3c622ecacbf348842472fc1b0c754ff599e581f83ed2075). tools/test_eff_poly.py prepares carrier-probe/results.json for verified uploading. Run tools/hil_interface.py with carrier_polynomial_readback.json (44 touch-driven steps) or carrier_polynomial_targets.json (40 steps using parameter-target). Both use ordinary UI activation/cleanup with seven presets, temporary ID 8 and 720×1280 UI. The prior collection with TREMOLO/CHORUS/RHYTHM uses probe label x=242/y=1051 and tap360/1065. SWAMP changes ordering; review both scripted guards and restore the seven-preset fixture before reuse. Current user audition has nine presets; no revised guard is qualified.

Prior-image UART-target evidence: /tmp/kestrel-rhythm-poly-targets-retry-hil.log passes all forty steps, including exact values, temporary-preset deletion, Talking Vowel restoration, instance baselines and SD probe absence. Upload/readback is /tmp/kestrel-rhythm-poly-upload.log. Prior touch evidence remains /tmp/kestrel-chorus-poly-touch-hil.log with /tmp/kestrel-chorus-poly-touch-upload.log. Installed identity/boot evidence has its own owner.

Address-zero errors without an established write do not establish hardware failure; listen=0 between reads is normal. Core's scratchpad-return owner governs semantics.

Sources: update/resource/effect/scope/parser/FPGA code, fixtures and HIL scripts.
