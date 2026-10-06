---
status: green
revised_at: "2026-10-06T16:23:08+11:00"
---

The opt-in rounded renderer uses blocking PPA fills followed by original software corners. Unsupported formats/strides, opacity/gradients and failed operations fall back to LVGL. Default explicit synchronization flushes before each fill and invalidates afterwards.

Pinned ESP-IDF 5.3.3 components/esp_driver_ppa/src/ppa_fill.c independently writes back and invalidates an aligned output window spanning full picture-width rows for fill_block_h, even for narrow blocks. LVGL's PPA fill directly calls blocking ppa_do_fill without extra synchronization. Other SDK versions require verification.

KEST_ROUNDED_CACHE_PREVIEW defaults OFF, requires both rounded previews and defines KEST_ROUNDED_DRIVER_CACHE_PREVIEW to omit duplicate wrapper synchronization. Current source/installed candidate splits wide interiors into horizontal middle/top/bottom rectangles. Row ranges are disjoint before cache alignment, avoiding the vertical split's repeated row synchronization; eligibility and four software corners are unchanged.

Installed horizontal app d9f26836d16fc3ee9b2a65568cb12b438fa5f59660e6ee80f9fc3c527e3cdff1 passes 20256 desktop comparisons. Two ordinary carrier checks each match 512 seeded cases with 113 hardware successes/zero errors; an intervening rejection check matches with 57 successes/56 injected errors. /tmp/kestrel-rounded-horizontal-artifact holds identities; /tmp/kestrel-horizontal-candidate.log and /tmp/kestrel-rounded-horizontal-result.json hold evidence. A matched UART scroll costs 51.15 versus vertical baseline 56.06 ns/pixel, about 8.8% less rendering; both render 15 frames and approximately 7.214 million pixels. Three repeated UART-injected frequency sweeps per image measure 5.081/4.715/4.618 ms per rendered frame for horizontal versus 5.106/4.721/4.598 for vertical diagnostic app17c6a74b. Frame counts differ; this shows no meaningful penalty in these bounded traces, not sustained physical FPS/feel. /tmp/kestrel-rounded-horizontal-dial-result.json binds images/traces; both restore frequency/pools/status. Controls/pools/magic/mask/status remain correct.

Fresh previews accept rounded-fill-check reject: the checker rejects alternate ppa_do_fill submissions before DMA while the oracle holds the UI lock, clearing injection on every return. Expected errors equal injected count; ordinary mode requires zero errors. Preview-only harness uses existing SDK esp_driver_ppa. It tests mixed hardware/software rectangles, not partial DMA/timeouts or registration/allocation failure.

Vertical rollback app 60126a9f6034f41394d283be3c71a23ec4f7104404f4297f8bf9525377eaf5ec and /tmp/kestrel-rounded-driver-cache-{artifact,result.json,carrier.log} retain prior checks/metrics. Vertical diagnostic app 17c6a74b87acac50957b4d787284546265c167bee1d4251c581a010a103a69f4 passes two injected and three ordinary checks under /tmp/kestrel-rounded-reject-{artifact,result.json,carrier.log}; these are repeated fixed-seed cases, not unique coverage.

Shared explicit synchronization was discarded for lack of gain; /tmp/kestrel-rounded-cache-artifact/result.json retains that variant. Before adoption qualify physical appearance, sustained scrolling/dials, broader clipping, partial DMA/timeouts, registration/allocation failures and SDK dependence. Display/installed-image owners govern selection/rollback. Sources: wrappers/CMake/checker, pinned IDF/LVGL and named artifacts.
