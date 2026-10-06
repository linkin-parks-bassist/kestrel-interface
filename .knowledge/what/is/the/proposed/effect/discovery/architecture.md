---
status: green
revised_at: "2026-10-06T17:20:26+11:00"
---

The candidate is a bounded asynchronous metadata catalogue feeding the existing selector. Production responsibilities remain for David's review; discovery specification and SQLite owners govern requirements/storage measurements.

Desktop-only tools/discovery_preview.c reuses current headers/buttons/fonts/keyboard and preset-add/capacity handling. Entry views are Type/Instrument/Genre/Keywords/All effects with sorted categories. Search matches names/facets without case sensitivity and preserves the category. Focus/repeat taps open one keyboard; dismissal, Filters returns and page exit permit reopening without accumulating callbacks.

Filters clone a draft: nested ALL/ANY, group/condition NOT, removal and Apply/Cancel/Clear. Empty ALL is unfiltered; empty ANY matches nothing. The title shows the whole draft's live count within current search/category, including inside nested groups. Dropdown, negation, insertion, removal and Clear update it; Cancel preserves applied rules. These are proposed interactions, not adopted UX or full first-order logic.

Default rendering creates all matching rows. The opt-in row reuse covers effects, main categories and filter-value pickers; how/does/the/desktop/discovery/prototype/recycle/rows.md owns bindings, category sorting, capacity evidence and limitations. Full descriptors/pointer tables and synchronous scans remain. Widget bounds do not establish bounded catalogue memory or MCU cost. Draft counting also scans loaded descriptors, not a production query implementation.

Checks establish correct effect/category selection and result changes after scrolling, unchanged authored captures and twelve keyboard dismissal/reopen cycles. Filters establish nine bass AND (delay OR chorus) matches/eighteen complements, Cancel, negation/removal and empty/Clear semantics. Live counts show 9→18 before Apply and empty ANY 0→27 when negated. Chorus category and ring search each show 2→0 under NOT; Cancel preserves scope and restores their original pairs. /tmp/kestrel-filter-count-result.json binds evidence; kestrel-discovery-groups-checked.json and kestrel-discovery-search-refocus-result.json retain other checks. Desktop inspection owns procedures.

Proposed production responsibilities:

- A discovery worker owns one SQLite connection/indexing/querying. Preserve asynchronous saves and separate 100-Hz smoothing. Review save-queue reuse because scans/saves could delay each other.
- Store file identity/name/description/facets without compiling every graph. INFO-only reader owner governs grammar/qualification. Review API/context ownership; fully parse selected descriptors before insertion. Huge libraries must not require one pool slot, graph and button per file.
- Publish bounded copied identities/names through kest_ui_async_call; lock scheduling/bounded object changes only. SD/query/parsing stays outside. Superseded results cannot replace current requests.
- Resolve selection to existing/new descriptors, then use current append/capacity handling, preserving presets, audio and reload ownership.
- Follow SD ownership/mutex rules; close before USB handoff and reconcile after APP returns. Prefer a rebuildable immutable snapshot initially; writable VFS/publication/partial-failed scans remain open.

Firmware snapshots loaded descriptors, creates all buttons and lacks selector refresh. Startup loading is synchronous under UI lock. Global parser allocation/reset requires serialization or reviewed reader context; file-task borrows do not settle deferred buffers.

Agree connection placement, publication/request lifetimes, cancellation, reader API, pagination/selection and complete refresh semantics before production implementation. Next: review navigation/responsibilities with David, then a measured vertical slice. Evidence: selector/UI/file-task/init/parser sources, owners and isolated experiments.
