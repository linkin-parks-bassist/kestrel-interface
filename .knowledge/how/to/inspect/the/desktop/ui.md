---
status: green
revised_at: "2026-10-03T23:12:57+10:00"
---

Build the desktop app with make, then run the isolated software-in-the-loop utility from the Interface checkout:

```bash
python3 tools/desktop_ui.py --headless --script tools/ui_scripts/danger_button.txt --output /tmp/kestrel-ui-inspection
```

Use a fresh output directory: the runner refuses to overwrite requested screenshots. It launches kest --control against a temporary copy of sdcard, records stdout/stderr in control.log and collects BMP screenshots. Pillow, when available, also emits PNGs. --headless selects SDL dummy video and software rendering; omit it for a visible window. Without --script, it captures the initial Presets page.

Commands are wait MS, click X Y, tree, screenshot PATH and quit, one per line; blank lines and # comments are accepted. Coordinates refer to the current desktop display, 600×1024. tree emits visible LVGL object paths, bounds, clickability and label text for choosing further input. click uses a normal pointer press/release. Commands and captures execute on the UI task; screenshots read the rendered SDL frame before presentation. The renderer falls back to software and uses a 32-bit pixel buffer matching its texture.

The sample navigates to Main Menu, opens Erase SD card confirmation, captures it, cancels and captures the returned menu. It never confirms erase. Headless navigation, preset creation, dialog opening and cancellation have been exercised. Current accepted Danger Button footer layout is owned by what/does/kest-button/implement.md; screenshots reflect those actual styles.

The desktop LVGL gitlink is a0341331ed9d5d0af2fbd93f5b2a307b581d4477, self-identifying as 9.6.0-dev and described as v9.5.0-20-ga0341331e. Firmware uses the pinned 9.6.0~1 component. This may explain differences from older recollections, but no older-build comparison establishes that cause.

The harness exercises UI/control code with simulated FPGA communications, not DSP audio or physical HIL. Desktop geometry differs from USE_5A carrier geometry. Temporary SD-card isolation protects checkout state; timeouts and missing screenshots fail the runner. Sources: desktop/kest_desktop.c, tools/desktop_ui.py, sample script, LVGL version/gitlink and observed captures/trees.
