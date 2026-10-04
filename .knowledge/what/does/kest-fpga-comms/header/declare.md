---
status: green
revised_at: "2026-10-04T04:52:14+11:00"
---

components/fpga/kest_fpga_comms.h declares task initialization/communication, queueing transfer/program batches, input/output gains, register commit, generic/memory reads, asynchronous status reads and an ordered task callback.

kest_fpga_queue_callback(callback, data) copies the callback/data tuple into the existing SPI FIFO. The SPI task invokes it after all earlier messages and their callbacks finish; it makes no wire transaction. Resource retirement uses one such callback per retired owner to establish payload lifetime, not per-read request tracking. Callback data must stay alive until invocation. A null callback or unavailable/full queue is rejected.

kest_fpga_queue_status(callback) copies a function pointer into the existing queue; SPI transfers 0xFF and calls it with the driver result and raw status byte. No per-query allocation is needed.

kest_fpga_mem_read_spec holds an address, callback data pointer and void (*callback)(void*, int64_t). kest_fpga_queue_mem_read(address, data, callback) copies that request into the SPI queue without waiting. SPI calls the callback with the unsigned two-byte payload or a negative transport error. Data is borrowed until completion; the memory path has no per-request allocation or tokens.

kest_fpga_read_spec holds type/id/address/length/result/data and a specification callback for the separate generic read API, which borrows the specification itself. KEST_FPGA_MSG_TYPE_* constants and KEST_FPGA_COMMS_H_ are declared here.

Source: components/fpga/kest_fpga_comms.[ch].
