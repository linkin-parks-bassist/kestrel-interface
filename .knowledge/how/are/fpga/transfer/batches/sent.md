---
status: green
revised_at: "2026-10-06T01:43:17+11:00"
---

`kest_fpga_io.c` owns ESP SPI transfer and growable byte batches. Program send wraps a batch in BEGIN_PROGRAM and END_PROGRAM; ordinary transfer sends raw bytes. Careful variants print bytes/status for diagnostics.

Controller read helpers clear the previous sticky command-error flag before issuing READ/READ32, while the controller is at a command boundary. Clearing after the command would become payload. They then send the request/address, wait for data-ready and read the response. Existing early error returns can leave a request incomplete; this change prevents a stale error from causing that abandonment but does not recover arbitrary misframing.

`tools/test_fpga_read_errors.py` in the superproject compiles the actual helpers against a SPI stub: stale errors clear before payload, fresh errors remain reported, address/null rejection emits no bytes. Carrier recovery from 0x61 returns read32 identity/capabilities and status 0x01; active Swamp periodic reads and pool baseline are restored (/tmp/kestrel-sticky-error-live.log). Initial error provenance remains unresolved. Sources: components/fpga/kest_fpga_io.c; Core src/controller.v.
