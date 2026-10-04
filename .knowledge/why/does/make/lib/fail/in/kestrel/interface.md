---
status: green
revised_at: "2026-10-04T06:24:26+11:00"
---

make lib succeeds. kest_preset_handle_name_change excludes firmware-only preset/sequence queue-save calls under KEST_LIBRARY, matching other preset operations; the library has no sequence member or persistence task. Firmware/desktop behavior is preserved by that guard.

The shared object explicitly links libm and uses -Wl,-z,defs, rejecting unresolved project symbols rather than leaving a seemingly successful but unusable library. Library objects depend on their production headers. A forced rebuild of the library passes; the standalone C compile_eff executable links to it through an $ORIGIN runpath and compiles the readback fixture into the expected 20-byte programming body. This establishes C compilation/linking and one fixture's command emission, not the older C++ full-top simulation or complete DSP verification. Existing parser const/escape warnings remain.

Sources: Makefile, components/core/kest_preset.c, main/kest_lib.h, tools/compile_eff.c, /tmp/kestrel-lib-check.log, /tmp/kestrel-lib-fresh-build.log, readelf dynamic dependencies and standalone compiler checks.
