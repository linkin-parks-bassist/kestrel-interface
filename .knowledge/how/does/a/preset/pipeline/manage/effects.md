---
status: green
revised_at: "2026-10-06T12:24:34+11:00"
---

components/core/kest_pipeline.c owns ordered effects. Append allocates an owner/node and publishes after descriptor construction. Failed node/member/resource/driver/scope construction releases unattached allocations; resource/partial-setting exhaustion, missing-driver and unnamed-memory tests check rollback/returned slots. Direct/stack callers, dependency errors and broad heap injection retain gaps.

Removal disables/marks/unlinks; control/UI/SPI complete retirement. Move relinks nodes. Other operations clone/count, rectify IDs, manage LFOs, calculate positions and compile transfers. The descriptor-refresh owner governs private reload staging/compatible controls.

kest_pipeline_check_capacity checks existing emitted counts and descriptor position increments against 256 slots, optionally reserving extra blocks. Transfer compilation calls it before batch allocation/position changes; overflow returns ERR_PIPELINE_FULL and empty output. Preset append uses it before instance allocation/save/updater notification. The selector prechecks for a capacity explanation and handles null construction before navigation/UI access. Direct pipeline append still serves private staging/loading; it does not enforce admission itself.

Host tests accept two-effect 256-block/empty chains, reject 257/oversized effects and independent count/report overflow without changing positions. Preset append rejects without publication. A real LVGL click on a full synthetic preset preserves pipeline/screen and opens the modal. Installed compiler rejection of a 257-block Swamp candidate preserves live state/values; ordinary reload then succeeds at status 0x01 (/tmp/kestrel-capacity-live.log, preceding image). The follow-on append/selector guard is installed with clean boot and preserved Swamp values (/tmp/kestrel-admission-live.log); installed preset/context/sequence activation preflights capacity before flags/pointers/cursors change, with host rejection tests. Carrier Play/Add Effect rejection preserves running/model state; inactive normal/oversized reloads complete after the sequence-member page-parent fix (/tmp/kestrel-parent-ui-live.log). Physical sequence overflow, queue-failure rollback, direct low-level encoding and worst-case sample timing remain unqualified.

free_effect atomically marks owner/resources and hands control an allocation-free intrusive list. MEM/LFO/DELAY/FILTER markers stop updates/reads; handoff is idempotent. Control and separate smoothing lock model access. Asynchronous file saves run outside the rendering lock; preserve the accepted responsiveness rollback. Repeated earlier-image reloads exercise coordination; broader mutation coverage remains open.

After program/update borrowers finish, control clears resource borrows and queues a SPI FIFO marker. Failed queueing/scheduling retains ownership; completion permits final UI reclamation. No-UI builds reclaim directly; unlinking alone does not return storage.

Final cleanup cancels callbacks/timers and releases UI, typed members/resources, heap payloads/coefficient containers, block/driver/scope/dependency containers, mutex and owner. Descriptor names/expressions/static LUTs stay borrowed. Unpublished objects reclaim immediately; allocator substitution needs agreed lifetime boundaries.

Host retirement tests model signed callbacks, markers, clone isolation, retention and cleanup. Earlier carrier deletion/readback checks bound physical evidence; they do not prove every edit/save/ID reuse/teardown/audio path. Sources: pipeline/effect/resource/update/comms implementations, tests/core/kest_pipeline_test.c, kest_context_test.c and kest_update_test.c.
