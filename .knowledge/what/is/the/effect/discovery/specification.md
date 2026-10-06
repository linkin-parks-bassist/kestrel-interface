---
status: green
revised_at: "2026-10-06T13:59:43+11:00"
---

Effect discovery must be astonishingly frictionless even with a huge library. Slow search is unacceptable; qualify usability and responsiveness at representative scale. Production search/filter UI and catalogue remain future behavior; the proposed-architecture owner covers the desktop navigation review artifact.

The effects list has a search bar at the top. Tapping it focuses keyboard text entry and reveals Filters. Filters opens a dialog for adding/removing selections grouped by instrument, type, genre and other axes. Category-first views let users click through to selected subsets rather than beginning with every effect in one long list. Search and composed filters remain available across axes; qualify populated scrolling as well as query latency. Exact category layout and taxonomy remain open.

Filtering supports union/intersection and composed logical predicates. Do not impose the prototype's OR-within-axis/AND-between-axes as the final contract. David wants first-order-predicate-logic expressiveness. Grouping, negation, quantification, exact semantics, keyboard dismissal, Filters persistence and complex-predicate editing remain open.

David approved optional .INFO description text and keywords/instruments/types/genres string lists. Installed firmware retains descriptor-owned copies independently of parser reset until final release after retirement. Missing fields remain compatible; empty lists are accepted. Wrong known-field types reject. Full text, case, order and duplicates are preserved. Normalization/taxonomy and catalogue materialization remain undecided. Explicit keywords are preferred; FTS5 prose search is optional.

211 host tests cover retention, arena reuse/retirement, 265-character description, empty lists, duplicate/case/order preservation and malformed-field rejection/recovery. Allocation tests reject/recover at thirteen discovery-copy boundaries over three passes without retained heap. Adding/removing metadata leaves the fixture's eight-byte DSP batch identical; temporary INFO arrays are consumed after copying or rejection.

All 27 carrier effects carry authored descriptions, keywords, bass instrument and type labels; genres remain unassigned. Readback/eff-info verify the original 25 after reload and UNDERTOW/SPIRAL after startup discovery. Controls survive; the installed-image owner holds current pools. Edge-fixture reloads qualify malformed-field rejection without replacement. Prior dial sweeps retain 180–200 FPS; idle snapshots are 180 FPS/1% after the metadata batch and 200 FPS/1% after UNDERTOW cleanup. The installed-image owner holds evidence and limits; carrier search/filter UI remains unimplemented.

Adopt SQLite as one self-contained direct-C package if substantial functionality follows cleanly; preserve bare-minimum dependencies. Integration/query owners hold target/package/resource/SD constraints and prototype evidence, not production/UI acceptance. Priority belongs to what/is/the/plan.md.

Sources: David's approved format/discovery requirements, parser/core source, metadata fixtures and host/carrier metadata tests.
