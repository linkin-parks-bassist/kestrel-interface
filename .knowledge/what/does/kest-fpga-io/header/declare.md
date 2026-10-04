---
status: green
revised_at: "2026-10-04T09:02:50+11:00"
---

components/fpga/kest_fpga_io.h declares SPI initialization/byte transfers, input/output gain and register commits, transfer-batch creation/encoding/draining/sending (including kest_fpga_batch_append_numeric for resolved signed/unsigned register conversion), program-batch sending, status decoding/printing, generic data requests and typed block/delay queries.

The carrier implementation uses SPI2 mode 0 at 10 MHz with MISO14, MOSI6, SCK5 and CS4. Desktop simulation substitutes fixed receive bytes and reports the feature disabled; it is not DSP simulation.

Generic data requests issue READ, the request ID and address bytes, then poll status for data_ready before issuing READOUT. Negative results distinguish null input (-1), command errors at successive phases (-2, -3, -4, -6) and no data_ready observed within the polling bound (-5). The parameterized reader uses a 32-try bound. Both generic read loops now guard their existing vTaskDelay(1) with KEST_USE_FREERTOS, matching firmware configuration; the obsolete ENABLE_FREERTOS guard omitted the intended wait. This correction builds with pinned ESP-IDF 5.3.3, is flashed and boots on the carrier. Two subsequent address-0 reads still return -5, so restoring the wait did not resolve the observed failure.

Physical qualification and installed-image evidence belong to how/to/build/and/run/the/interface.md and what/is/the/status/of/periodic/fpga/memory/reads.md. SPI error propagation/status reliability still require source and hardware review; a negative result alone does not locate the physical fault.

Source: components/fpga/kest_fpga_io.[ch], main/kest_int.h and executed desktop/embedded builds.
