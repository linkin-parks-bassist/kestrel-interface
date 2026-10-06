---
status: green
revised_at: "2026-10-06T16:15:53+11:00"
---

IDF UART0 is 115200 baud, prompt kest>. Keep one connection:

```bash
python3 tools/uart_console.py --port PORT --log NEW_LOG
```

Requires pyserial/unused log path. Opening commonly resets; wait 12 seconds uptime. Empty lines clear initial PTY input.

Commands include help, info, heap, pools, uptime, ui-tree, ui-profile, tap/touch, dsp, parameter-target, presets, sequences, sequence-step, fpga-status/read/read32, eff-file, eff-reload and eff-info.

eff-info CNAME inspects loaded memory. Name/cname/optional description and list entries emit field=FIELD index=N hex=TEXT_BYTES; list headers emit field=FIELD count=N. Missing description has no row; empty lists count zero. result=0 ends output; unknown cname returns ERR_NOT_FOUND. It retains immutable metadata under UI lock, prints outside it, then releases under lock. Carrier tests qualify unknown/missing fields, long descriptions, lists and repeated/rejected reloads; installed-image owner holds evidence.

pools reports eight typed pools ending KEST pools end; rows lock separately. presets reports IDs/names/files/active selection. sequences reports main(index0), loaded sequences, selected/active flags and ordered preset IDs/current position. Indices are snapshot ordinals. sequence-step next/prev invokes normal active navigation; ends no-op, absent active sequence errors. It never selects/starts a sequence.

dsp locks UI and reports active effects, parameter IDs/values/bounds/driver flags, setting IDs/integer bounds/choice labels and resource addresses/read cadence/latest signed values. Rows establish state, not physical timing. FPGA reads/status are asynchronous; read32 requires aligned 24-bit addresses. Carrier magic0x4b455354/capabilities0x06 establishes transport/build bits, not ROM initialization.

parameter-target PRESET EFFECT PARAMETER VALUE validates uint16 IDs/finite effective-range values. Missing IDs, invalid bounds and non-overridden drivers reject. It queues smoothing/marks dirty without saving or overriding drivers. Observe convergence via dsp; nominal labels may lag. hil_parameter_target.py restores the original value; restoration marks dirty.

UI commands use normal LVGL hit-testing/callbacks under UI lock. tap holds 80 ms; pressed pointer fails. touch down/move X Y and touch up support drags/scrolls within display bounds. ui-tree includes offscreen children and stalls drawing. ui-profile start resets/enables redraw counters; stop disables and prints frames, flushes, pixels, render/flush-call/flush-wait microseconds outside the lock. Flush timings overlap render time. Counters emit nothing during drawing; avoid tree dumps/concurrent physical input.

eff-file list/read/delete/publish, move OLD NEW and write NAME OFFSET HEX enforce SD ownership/FAT8.3 names. Reads emit hex. Writes ≤128 bytes fit256-character commands; offset0 starts tmp/NAME.tmp, later offsets equal length. Publish renames without validation; replace deliberately/verify readback. eff-reload NAME.eff updates loaded cname/instances/UI/program without reboot, retaining compatible controls but resetting DSP state; new identity needs discovery. upload_effects.py --replace/--reload verifies bytes/reloads identical files. Rejection leaves published bytes.

hil_interface.py takes JSON command/expect steps, optional wait_ms≤60000 and optional page title. page checks ui-tree's first-screen header before sending; mismatch aborts without that command. It distinguishes header titles from list rows; this read-then-act guard is not atomic against concurrent navigation. Guard coordinate actions after reboot, wait for transitions, and keep tree checks outside profiling windows. Commands timeout after 10 seconds; failure closes UART without rollback. Captured headers and carrier wrong-page rejection/correct Presets→Preset 9→Bass Ring navigation pass (/tmp/kestrel-page-guard-{reject,nav}.log); controls and nine-preset pools remain unchanged. Review older smoke fixtures against installed state before replay.

Sources: main/kest_console.[ch], README.md, superproject tools/hil_interface.py and named tools.
