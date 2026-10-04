---
status: green
revised_at: "2026-10-04T09:05:17+11:00"
---

The deprecated kest_representation framework is removed: its source/header, Make/CMake entries, feature switch, object fields, list allocations, registration calls and unused callbacks are absent. The unused representation-based context-save wrapper is also removed. No replacement observer framework is introduced.

Supported update paths remain direct parameter/widget links, the existing kest_ui_async_call scheduling, control-loop notifications and the file-task state/preset save queues. The symbolic DSP instruction-sequence representation is a different mechanism and remains unaffected.

Fresh first-party desktop/test compilation passes the C suite; pinned ESP-IDF 5.3.3 builds the image. The isolated desktop Danger Button interaction check verifies navigation, popup opening and cancellation after field removal. The cleanup is flashed on the carrier and boots; phantom back navigation and a Mix control movement from 0.35 to 0.72 and back are verified by UI-tree labels. These bounded checks do not establish every UI path, DSP parameter application or audio. how/to/build/and/run/the/interface.md owns installed-image identity and capture evidence.

Sources: components/core and components/ui source/declarations, build manifests, executed fresh desktop/IDF builds/tests, tools/desktop_ui.py interaction capture and /tmp/kestrel-current-hil-console.log.
