---
status: green
revised_at: "2026-10-04T05:55:33+11:00"
---

Parameter smoothing stays in its separate 100 Hz task. David explicitly directs restoring and retaining this boundary for now while he thinks about the original reason he rejected a merge. He requires steady high-frequency smoothing without discontinuities. Ask him before architectural decisions such as merging tasks; his resource/readback control-loop direction does not authorize moving smoothing.

Under KEST_USE_FREERTOS, kest_init_parameter_updater creates kest_param_update_task with a 4096-byte stack and priority 8. The task creates its 16-entry RTOS queue, sets queue_initd, waits FPGA_BOOT_MS, then uses xTaskDelayUntil on a nominal 10 ms period. It retains a 64-slot pending ring and 16 active slots, coalescing targets by parameter ID. kest_parameter_trigger_update waits for queue initialization and sends with a 1 ms queue timeout. Without FreeRTOS it assigns directly.

Each smoothing pass resolves parameter/effect pointers by ID, drops failed lookups, evaluates the current value and limits linear change by 10 ms times max_velocity; logarithmic limits additionally scale with the current value. Active-preset notifications go to the separate control-task queue, with existing one-per-effect send selection. Context gains enqueue FPGA updates and widget refreshes. Reaching a target requests state or preset persistence. Control computes dependent register/filter/instruction changes and program batches independently.

David's thread-safety approach includes lookup, partial mutexing and atomic parameter writes. Source confirms repeated ID lookup and atomic value stores/loads on the threaded path. The effect-mutex attempt in the smoothing send gate is currently commented out; retained and queued pointers still need lifetime review. Neither lookup nor an atomic value store alone guarantees an object's lifetime. Preserve the intended architecture when addressing concrete defects; do not infer that partially implemented synchronization warrants replacing task boundaries.

Both smoothing and control tasks have priority 8. Separation permits independent schedules but does not prove cadence under load. David's exact earlier gotcha remains unconfirmed; the inspected earliest implementation, Git 66b6c7ea components/core/m_param_update.c, used a 10 ms task and a nonblocking effect-mutex attempt without documenting why a later merge was rejected. Cadence, pointer lifetime, queued-ID reuse and persistence-job ownership remain qualification questions.

Sources: David's rollback, cadence and architectural-consultation directions and description of his synchronization attempt; components/core/kest_param_update.[ch], kest_parameter.c, kest_update.[ch], kest_init.c and inspected Git history.
