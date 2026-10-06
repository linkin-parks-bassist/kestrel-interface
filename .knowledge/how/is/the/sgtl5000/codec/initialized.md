---
status: green
revised_at: "2026-10-05T16:34:45+11:00"
---

kest_sgtl5000_init configures I2C after a startup delay, selects the codec address level, runs sgtl5000_enable for analogue/digital power and audio-path registers, sets input/output levels and marks status. Helpers wrap register accesses. Current gain/readback belongs to how/is/the/sgtl5000/configured.md; source is components/drivers/kest_sgtl5000.c.

David reports a loud crack/pop when SGTL power cycles or reconfigures and identifies prevention as likely a PCB change. No pop-free carrier behavior is established. Review mute/rail/audio sequencing and distinguish deliberate reconfiguration from abrupt rail collapse before proposing a carrier remedy. The superproject what/is/the/audio/mute/circuit.md owns Rev-B's supervised mute/held-DAC-supply design and its pending physical startup/shutdown/brownout qualification; Rev-B replaces SGTL, so it is not an installed carrier fix.
