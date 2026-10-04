---
status: green
revised_at: "2026-10-04T06:46:36+11:00"
---

The dedicated FreeRTOS file task runs at priority 8 with a 4096-byte stack and the diagnostic name kest_file_task. It creates a 16-job queue in its entry and blocks waiting for jobs. State jobs carry a complete kest_state value with fixed filename arrays; preset and sequence jobs carry borrowed object pointers. Save calls run without the UI/model lock.

kest_queue_state_save skips the first 3000 ms to avoid persisting incomplete startup state, clones context state, then queues the value. Preset/sequence enqueue functions copy only pointers. Enqueue returns ERR_UNINITIALISED before queue creation and ERR_CURRENTLY_EXHAUSTED when its 1 ms send fails.

David's intended direction is for objects that save to disk to keep their raw file bytes in a memory buffer and mutate that buffer when the object changes. Saving would use already-maintained bytes rather than rebuild the file by walking the model. He expects improved file handling and less repeated work; memory consumption is the main tradeoff and is not currently a major concern to him. This is a design intention, not implemented behavior. It differs from preparing a fresh per-job copy when Save is pressed. Buffer ownership during writes, resizing, changes and deletion, file layout and error behavior remain to be discussed before implementation. The earlier copied-job versus retained-live-owner question is superseded; neither alternative was selected, and queueing deletion was not agreed.

David explains that pools were partly intended to leave decommissioned objects accessible to stale jobs rather than immediately release their backing storage. Some nested pointers still refer to separately allocated memory. Existing cleanup can release those allocations, and reusable pool slots may later hold other objects, so this intention is not a blanket safety guarantee. cxt_remove_preset/cxt_remove_sequence unlink and dispose of objects; preset cleanup also releases its name and pipeline. Sequence serialization walks referenced presets and can save them inline. Queued work and edits can consequently see decommissioned or changing state; the precise consequences depend on allocation and cleanup paths. This is source review, not a reproduced carrier crash.

David prioritizes building capability over blanket hardening at this stage. Do not redesign the file task solely to eliminate theoretical stale access or impose retain/copy machinery without a concrete need. He reports infrequent crashes and that a firmware crash generally leaves FPGA audio running. Fix demonstrated defects proportionately and preserve asynchronous SD work and UI responsiveness. Discuss architectural changes with him first.

Sources: David's maintained-file-buffer intention, pool rationale and development-priority correction; components/core/kest_file_task.c, kest_state.h, kest_context.c, kest_preset.c, kest_sequence.c, kest_pool.h and kest_files.c.
