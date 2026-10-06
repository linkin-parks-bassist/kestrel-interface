---
status: green
revised_at: "2026-10-06T16:20:20+11:00"
---

waveshare_dsi_touch_5_a_init starts BSP MIPI-DSI/LVGL/backlight at RGB565 720×1280; its output display-pointer argument remains unassigned.

Partial rendering uses one screen-sized PSRAM draw buffer (1843200 bytes), buff_spiram=true, buff_dma=false, no software rotation and BSP_LCD_DRAW_BUFF_DOUBLE=0. Two DPI scanout buffers are separate. LVGL uses -O2; firmware retains -Og. Managed-source instrumentation is removed.

Normal sdkconfig enables PPA for opaque rectangles, images disabled, 128-byte bursts; labels/rounded fills remain software. Pinned IDF 5.3.3 requires no additional package. Normal app: 1263456 bytes; installed identity has its own owner.

Rendering dominates flush/wait. Full-buffer software costs about 150 ns/pixel; normal PPA 125–127. Rejected 200-row PPA costs 148 despite saving 1555200 bytes; retain full buffer. ui-profile is opt-in with overlapping phases. /tmp/kestrel-scroll-{comparison,ppa-result,ppa-200-result}.json holds buffer comparisons.

LVGL clips non-intersecting bounds before drawing. Selector instantiates all rows; desktop 27-effect view draws twelve intersecting rows. Bounded results reduce object/layout work. Sources: selector/LVGL src/core/lv_refr.c and /tmp/kestrel-discovery-clipping-evidence.json.

rounded_fill_preview.c wraps software fill with a hardware rectangular middle and original software edge strips. Unsupported formats/strides, opacity/gradients and failures use software. The cache owner governs synchronization, installed evidence and qualification.

rounded_corners_preview.c uses three horizontal flat bands/four software corners for RGB565 only when both shape and clipped width≥400. Narrower redraws retain strips; XRGB8888 retains strips because masked blends preserve its unused byte whereas flat fills overwrite it. Horizontal bands avoid repeating full-row PPA synchronization. Preview options default OFF; task boundaries remain.

The horizontal candidate passes 20256 desktop comparisons and carrier ordinary/rejected-fill sampled checks; the cache owner holds exact counts, matched scroll improvement and remaining physical/dial/fault gates. Build bin/rounded-{fill,corners}-check with tools/discovery_preview.mk. Oracles cover clipping, offsets, tiny/oversized radii and fallbacks, not exhaustive qualification.

Older explicit-cache vertical scroll costs 55.14 versus strip replay 62.95 ns/pixel. Three dial repetitions aggregate 606.65 versus 604.91; individual frame costs 4.73–5.27 ms. Different frame/inertia counts limit comparison. /tmp/kestrel-rounded-wide-clipped-result.json and artifact/identity.json bind image/checks/traces. how/to/profile/carrier/drawing/paths.md owns measured fill/label/border/arc breakdown; instrumented times are not speed comparisons.

Unrestricted corner splitting worsened the short dial trace; a shape-only cutoff still accelerated narrow parent-background redraws. Clipped-width eligibility avoids that case. /tmp/kestrel-rounded-{corners-carrier,wide}-result.json holds evidence. David finds the preceding strip image improved scrolling, not amazingly; sustained physical acceptance remains unresolved. Rollbacks: /tmp/kestrel-rounded-driver-cache-artifact, /tmp/kestrel-rounded-ppa-build and /tmp/kestrel-scroll-software-fullbuffer.
