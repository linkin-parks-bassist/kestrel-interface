---
status: green
revised_at: "2026-10-04T23:59:59+11:00"
---

The obsolete USE_TEENSY switch, disabled message/readback hooks and orphan effect_view_request_parameter_values declaration are removed from Interface first-party source. Their queue_msg_to_teensy, create_m_message and kest_message references had no implementations in this checkout.

Context deletion/effect removal and effect view/settings retain supported existing paths. Current FPGA programming/readback is unchanged. The 165-test host suite and pinned ESP-IDF 5.3.3 build pass after removal; symbol search finds no remaining references. This cleanup is included in the installed image; current identity and carrier qualification are owned by what/firmware/is/installed/on/the/carrier.md.

Sources: main/kest_int.h, components/core/kest_context.c, components/ui/kest_effect_view.[ch], kest_effect_settings.c, symbol search and executed builds/tests.
