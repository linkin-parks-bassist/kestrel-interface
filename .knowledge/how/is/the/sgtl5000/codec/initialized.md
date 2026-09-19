---
status: "unverified"
created_at: "2026-09-20T00:06:18+10:00"
scope: "local"
source: "components/drivers/kest_sgtl5000.c:45-165,271-350"
---
Status: Green

`kest_sgtl5000_init` configures the I2C bus/device after a startup delay, selects the codec address level, runs `sgtl5000_enable` to write analog/digital power and audio path registers, sets input/output levels, and sets a status flag. Register reads/writes are wrapped by helper functions. Source: components/drivers/kest_sgtl5000.c:45-165,271-350
