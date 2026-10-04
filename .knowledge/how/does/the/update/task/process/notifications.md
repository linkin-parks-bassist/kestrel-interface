---
status: green
revised_at: "2026-10-04T05:03:00+11:00"
---

kest_update_task drains a 16-item FreeRTOS queue into an update list; a preset notification first drains pending temporary lists. Each 100 Hz iteration handles notifications, processes resources when READY, generates commands and a transfer batch, sends it and waits for the next tick. It starts only under KEST_USE_FREERTOS.

A rejected programming submission preserves the existing batch and REPROGRAM state. At the next tick, the task attempts that retained batch before consuming more notifications; once accepted it drains the temporary lists and resumes ordinary processing. No extra request records or retry queue are introduced. Failed submission frees its copied send buffer and returns the queue error; successful submission transfers that copy to the SPI task. Ordinary READY-state update batches remain best-effort and are drained after an attempt. An uninitialized SPI queue rejects messages instead of busy-spinning.

Scratchpad reads bypass the update queue: SPI invokes the dispatched callback, which publishes the latest signed sample atomically and sets an arrival flag. Resource processing atomically consumes that flag and propagates current scope dependencies once, coalescing multiple arrivals. Enabled memory resources with positive read_period_ms receive best-effort read attempts at that period rounded up to 10 ms control ticks; shorter periods use each tick. Submission is nonblocking, and a missed/failed read leaves the current sample alone. No request tokens, reply correlation, read deadlines or retry records remain.

Clearing updater state drains the active-resource cache but preserves retired owners. Control collects deletion handoffs before draining notifications, skips marked payloads/dead owners, and schedules final UI-task reclamation only at a READY tick's end after temporary borrowers have been consumed and a FIFO SPI callback has confirmed earlier reads finished. Dead-owner parameter/scope notifications are discarded before queueing. A failed UI scheduling attempt retains the owner for the next tick; non-UI builds reclaim directly. Queue rejection and REPROGRAM retain them. The pipeline lifecycle owner explains paired release and bounded unit/carrier evidence. This does not qualify every concurrent UI callback or list mutation. REPROGRAM queue acceptance is not proof of successful physical programming; that failure behavior remains open.

Sources: components/core/kest_update.[ch], kest_resource.[ch], components/fpga/kest_fpga_comms.c and tests/core/kest_update_test.c.
