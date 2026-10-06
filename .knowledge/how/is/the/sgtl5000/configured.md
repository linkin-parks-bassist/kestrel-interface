---
status: green
revised_at: "2026-10-05T15:50:16+11:00"
---

components/drivers/kest_sgtl5000.c initializes the carrier codec over I2C on GPIO2 SDA/GPIO3 SCL, selects line input and configures clocks, I2S, routing, power and volume. Startup source now calls sgtl5000_line_in_level(0), selecting 0-dB analogue ADC gain; the pinned ESP-IDF 5.3.3 build passes (/tmp/kestrel-sgtl-unity-build.log). Hash-verified flashing passes (/tmp/kestrel-sgtl-unity-flash.log); installed ELF SHA-256 is 7725a967727f6df42cc82e77a09720d68d5a4a2719bb62852de9bcda299ed4de. Startup readback confirms ADC=0x0000, DAC=0x3C3C and line-output=0x1F1F (/tmp/kestrel-sgtl-unity-live.log). Line-output setting stays 31; DAC volume stays 0x3C3C.

The input setter writes equal channel nibbles to CHIP_ANA_ADC_CTRL and clears the −6-dB range bit. Output updates equal five-bit fields and clamps its argument to 13–31. NXP's [SGTL5000 datasheet](https://www.nxp.com/docs/en/data-sheet/SGTL5000.pdf), tables 22/24, specifies ADC gain in 1.5-dB steps with zero meaning 0 dB and an optional −6-dB shift; DAC code 0x3C means 0 dB.

The prior installed boot dump (/tmp/kestrel-rhythm-restored-smoke-hil.log) confirms ADC=0x0077 (+10.5 dB both channels), DAC=0x3C3C and line-output=0x1F1F. David reports improved sound after removing the boost and now reports no clipping, considering remaining low-B growl plausibly instrument tone. The improvement supports retaining 0-dB codec gain; exact prior clipping stages were not instrumented. David confirms global Kestrel input gain acts in FPGA preprocessing after the SGTL ADC, so lowering it cannot repair upstream clipping. Before the gain change, distortion persisted with RHYTHM Mix zero, −22.4-dB FPGA input/0-dB output and no AudioBox clipping indication.

The current UART console has no live codec register/gain controls; boot dumps are startup readbacks. Retain 0-dB codec gain. No current audible clipping fault remains; broad physical headroom qualification is still incomplete. The superproject RHYTHM owner holds audition/isolation state. Register values and clean FPGA status do not establish undistorted audio.
