---
status: green
revised_at: "2026-10-06T09:35:22+11:00"
---

The dedicated FreeRTOS file task runs at priority 8 with a 4096-byte stack and a 16-job queue. State jobs own a complete kest_state value; preset and sequence jobs borrow object pointers. Enqueue returns ERR_UNINITIALISED before queue creation and ERR_CURRENTLY_EXHAUSTED on a failed 1 ms send. State saving skips the first 3000 ms and snapshots context before enqueueing.

Save calls run asynchronously without the LVGL/UI-model lock. David directed reverting the lock addition after a reproduced regression. Dial smoothing queues preset saves whenever a target is reached; page changes queue state saves. The rejected locking implementation performed fourteen saves in 1.006 seconds, holding rendering for 771 ms, with individual holds up to 56 ms. Control computation totaled 13 ms while waiting 803 ms in a neighboring 1.009-second window. Idle spans were below the trace's 1 ms resolution. This bottleneck explains the reported dial FPS collapse from above 100 to 8 (/tmp/kestrel-fps-trace-live.log).

The rollback is installed and temporary tracing removed. Carrier Bass Ring sweep samples report 183/190 FPS at 9/10% displayed CPU versus idle 200/0. David confirms responsiveness is much better. Its Mix value is restored exactly to 0.781000018 and verified through dsp; David's concurrent Carrier-frequency change is retained. /tmp/kestrel-fps-rollback-live.log holds boot, interaction and restoration evidence. This qualifies the reproduced responsiveness repair, not broad SD failure or concurrent save/reload safety. Installed-firmware owner holds image identity.

LVGL's LV_USE_OS=0 indicator derives from lv_timer_get_idle, measuring handler busy time rather than whole-machine CPU utilization. Boot confirms 360 MHz CPU/200 MHz PSRAM. UART opening resets the carrier; startup heap does not diagnose pre-reset state. Diagnostic UART sessions are closed.

Rollback restores previous borrowed-pointer behavior; it does not settle concurrent edits/deletion. Context deletion frees owners/nested allocations; pool reuse is not a lifetime guarantee. Sequence serialization can save referenced presets inline. Do not silently reintroduce a rendering lock to address lifetime concerns.

David intends raw file bytes maintained on each object as it changes, so saving need not walk the model. This remains deferred and unimplemented. Ownership during writes, resizing, edits/deletion, layout and errors need agreement; fresh per-job copies differ. Discuss architecture first.

David prioritizes capability and responsive asynchronous SD work over speculative blanket hardening. Fix demonstrated defects proportionately; preserve separate 100 Hz smoothing and distilled architecture.

Sources: David's rollback, buffer, pool, performance and preservation guidance; components/core/kest_file_task.c, kest_state.h, kest_context.c, kest_preset.c, kest_sequence.c, kest_pool.h, kest_files.c; sdkconfig; LVGL lv_os_none.c/lv_timer.c; the two carrier logs above.
