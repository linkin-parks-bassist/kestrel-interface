---
status: green
revised_at: "2026-10-06T17:20:26+11:00"
---

Build the desktop app with make, then run the isolated utility from the Interface checkout:

```bash
python3 tools/desktop_ui.py --headless --script tools/ui_scripts/danger_button.txt --output /tmp/kestrel-ui-inspection
```

Use a fresh output directory; screenshots are never overwritten. The runner launches kest --control against a temporary sdcard copy, records control.log and collects BMPs, plus PNGs with Pillow. --headless uses SDL dummy/software rendering; omit for a visible window. Without --script it captures Presets. --binary selects another executable; --sdcard chooses a source fixture that is still copied.

Commands are wait MS, click X Y, touch down X Y, touch move X Y, touch up, tree, screenshot PATH and quit. Blank lines/comments are allowed. Coordinates use the 600×1024 desktop display. tree prints LVGL paths/bounds/clickability/text, including offscreen rows. click supplies ordinary press/release; touch retains contact for drags. Controls/captures run on the UI task; screenshots read SDL's software-rendered 32-bit frame. Missing captures/timeouts fail.

Danger Button opens Main Menu/Erase SD confirmation, captures, cancels and captures the menu; it never confirms erase. Navigation, preset creation and dialog cancellation are exercised. The button owner governs David's accepted footer appearance.

Run the desktop-only discovery proposal:

```bash
python3 tools/discovery_preview.py --headless --output /tmp/kestrel-discovery-review
```

It builds bin/discovery-preview with a replacement selector and fresh empty preset/sequence directories plus authored effects carrying types metadata. --script chooses a sequence under tools/ui_scripts. discovery_preview.txt captures browsing/search/addition; discovery_filters.txt exercises nested bass AND (delay OR chorus), live draft count and group negation; discovery_filter_edits.txt covers Cancel, condition negation/removal, empty groups and Clear. discovery_search_refocus.txt covers keyboard dismissal/reopening, Filters returns, ring typing and page exit/return. Architecture owner governs behavior/production limits.

Set KEST_DISCOVERY_VIRTUAL_ROWS=1 for recycled effects, main categories and filter-value pickers. discovery_virtual_rows.txt covers scroll/selection; discovery_virtual_search.txt and discovery_virtual_filters.txt cover result changes after scrolling. discovery_virtual_categories.txt opens keywords, captures a native scroll and selects a rebound category. discovery_virtual_picker.txt exercises keyword-picker scrolling/selection, Back, Cancel/reopen and Apply; use a large vocabulary fixture for its scrolling check. For a custom vocabulary fixture, run desktop_ui.py with --binary bin/discovery-preview --sdcard SOURCE and that script. how/does/the/desktop/discovery/prototype/recycle/rows.md owns allocation/bindings/evidence and limitations.

KEST_DISCOVERY_TRACE_DRAW=1 with discovery_draw_clip.txt logs row bounds/names; unsuitable for timing. /tmp/kestrel-discovery-draw-clip-checked/control.log and /tmp/kestrel-discovery-clipping-evidence.json establish 27 instantiated effects/twelve intersecting rows initially, not scrolling throughput or carrier timing.

Rounded experiments use tools/discovery_preview.mk: bin/rounded-fill-check gives seeded pixel comparisons; bin/discovery-rounded-preview works with --binary. Display owner governs results/hardware limits.

Desktop LVGL gitlink a0341331ed9d5d0af2fbd93f5b2a307b581d4477 identifies as 9.6.0-dev (v9.5.0-20-ga0341331e); firmware pins 9.6.0~1. No older-build comparison proves the cause of older recollections.

This is simulated FPGA/UI testing, not audio/physical HIL. Geometry differs from USE_5A; copied fixtures protect checkout state. Sources: desktop/kest_desktop.c, tools/desktop_ui.py, discovery tools/scripts, LVGL version and checked captures.
