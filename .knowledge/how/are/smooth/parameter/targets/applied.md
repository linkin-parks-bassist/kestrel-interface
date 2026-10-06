---
status: green
revised_at: "2026-10-06T09:21:32+11:00"
---

Parameter smoothing stays in its separate 100 Hz task. David requires steady high-frequency smoothing without discontinuities and consultation before merging tasks or changing architecture.

Under KEST_USE_FREERTOS, kest_init_parameter_updater creates a priority-8 task with a 4096-byte stack. It creates a 16-entry RTOS queue, waits FPGA_BOOT_MS and runs on nominal 10 ms xTaskDelayUntil periods. A 64-slot pending ring and 16 active slots coalesce targets by parameter ID. Triggering waits for initialization and uses a 1 ms queue-send timeout. Without FreeRTOS, assignment is direct.

Each pass holds the UI/model lock, resolves parameter/effect pointers by ID, drops failed lookups, evaluates values and limits linear change by 10 ms times max_velocity; logarithmic limits scale with the current value. Atomic parameter stores/loads do not alone protect lifetime. The effect-mutex send gate remains commented out. Reaching a target requests persistence; active-preset notifications go to the separate control queue, retaining one-per-effect send selection. Context gains enqueue FPGA changes and asynchronous widget refreshes.

kest_parameter_cancel_preset_updates removes matching active, pending-ring and RTOS updates while its caller holds the same UI/model lock. This permits live descriptor migration to cancel targets before replacing parameters. The whole-pass lock also couples smoothing cadence and rendering; interaction lock duration and contention need measurement alongside the file-task SD-lock regression.

Smoothing and control both have priority 8. Independent schedules do not establish cadence under load. David's original reason for rejecting a merge remains unconfirmed; Git 66b6c7ea components/core/m_param_update.c used a 10 ms task and nonblocking effect-mutex attempt. Broader cadence, pointer lifetime, ID reuse and persistence ownership remain qualification questions.

Sources: David's task-boundary, cadence and architectural directions; components/core/kest_param_update.[ch], kest_parameter.c, kest_update.[ch], kest_init.c and inspected Git history.
