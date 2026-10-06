---
status: green
revised_at: "2026-10-06T15:54:18+11:00"
---

Use the isolated corner renderer with KEST_ROUNDED_FILL_PREVIEW=ON, KEST_ROUNDED_CORNERS_PREVIEW=ON and KEST_DRAW_PROFILE_PREVIEW=ON in a separate IDF build. All default OFF; the third requires the first two. Normal firmware has no draw-path instrumentation. The build procedure owns SDK/configuration and safe UART/flash sequencing.

tools/draw_profile_preview.c wraps software fill, border, label, arc, image and shadow entry points. Fill retains the corner backend, including its PPA interior. ui-profile start resets/enables counters under UI lock; stop disables under lock and reports calls/microseconds outside it. The counters allocate nothing and emit no UART output during drawing. Do not sum these counters as complete frame time: unwrapped draw tasks, task generation, transfers and dispatch remain outside them; instrumentation adds overhead.

Pinned-IDF carrier image 5e69294a2d7335fa9130bae51becd081c4722e3d97a8fb1d0c38e0e9561bd55f passes 512 pixel comparisons/112 hardware fills/zero errors. The unchanged corner entry-point refactor also passes 20256 desktop comparisons. /tmp/kestrel-draw-profile-artifact preserves image/ELF and identity-and-traces.json; /tmp/kestrel-draw-profile-result.json binds hashes and measured totals.

One injected populated-list scroll records 54 rendered frames, 755631 render microseconds and 13963007 pixels: fill 339098 us/454 calls, labels 132185 us/316, borders 29441 us/56; no arcs/images/shadows. These are about 45%/17%/4% of render time, not proof that all remaining cost belongs to object walking.

One dial sweep records 38 frames/179236 render us: arcs 36543 us/102 calls, fills 32060/128, labels 22390/40, borders 10553/64. Different gestures/frame counts and instrumentation preclude an uninstrumented speed comparison. Use fills as the next scrolling target; evaluate arcs separately for dial changes. Physical sustained feel and appearance remain unqualified.

Sources: current wrappers/CMake/console, successful IDF build/hash-verified diagnostic flash, /tmp/kestrel-draw-profile-carrier-retry.log and pixel-check output.
