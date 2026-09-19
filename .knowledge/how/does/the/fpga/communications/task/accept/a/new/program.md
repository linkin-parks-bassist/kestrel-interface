---
status: "unverified"
created_at: "2026-09-20T00:04:03+10:00"
scope: "local"
source: "components/fpga/kest_fpga_comms.c:7-180"
---
Status: Green

The comms task initializes SPI and a 32-message queue, delays for FPGA boot in hardware, and handles program messages with up to three attempts. It checks flags after transfer, handles incomplete programming/misalignment, clears timeout or bad-health flags and retries, waits for swapping to finish, then clears the active preset pending flag on acceptance. In simulated mode it clears pending without hardware transfer. Source: components/fpga/kest_fpga_comms.c:7-180
