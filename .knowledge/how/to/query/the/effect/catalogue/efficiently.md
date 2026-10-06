---
status: green
revised_at: "2026-10-06T15:20:45+11:00"
---

Choose by density/storage locality; no production strategy is adopted. Ordered effect(name,id) scans with indexed facet(axis,value,effect_id) EXISTS stop after common matches; rare/empty predicates may scan everything. Covering facet_page(axis,value,name,effect_id) PRIMARY KEY WITHOUT ROWID supplies ordered candidates. Seed conjunctions only from necessary facets.

Merge covering streams for unions. Put ORDER BY/LIMIT at the compound SELECT's top level and tuple cursor bounds in each branch; wrapping UNION can materialize/sort candidates. Nest UNION/INTERSECT to preserve groups. ID sets help rare/empty predicates but may read many SD pages. Covering effect(id,name) avoids description-bearing rows. Measure returned ID/name pages.

Keyset: (name,id)>(last_name,last_id), ORDER BY name,id LIMIT 50; OFFSET grows with depth. Refresh/filter/order-change cursor semantics remain open. Parameterized ANY/ALL/NOT membership queries are bounded Boolean logic, not full first-order logic or selected UX. Experimental empty ALL is true and empty ANY false, including nested/negated groups; SQL, required-facet inference and candidate branches honor these identities. Product empty-filter editing semantics remain open. Explicit keywords need no FTS5; optional FTS5 supplies text candidates.

P4/SD 100k tied-name fixture: 25000 bass IDs/names match across 500 pages; walk 7772833 us including yields. Fresh empty/first pages 58986/59546 us, repeated first 955, after ID200/99800 18189/79603. Five checks allocate 141848 SQLite bytes. Covering snapshot 12136448 versus 7815168 bytes (/tmp/kestrel-sqlite-idf-ties-runtime.log).

Composed scale uses KT_PROBE_ROWS/host_check.py --rows (default 10k). The 100k host verifies 50 traversals/50 immediate checks at about 30.5 MB; not carrier budgeting. Six-predicate 10k carrier coverage uses an aligned internal 4-KiB SD buffer for multi-sector reads; exact costs belong to the composed-query carrier owner. No policy/cache adoption.

Seventy-two alternating visits on one connection/256-KiB cache target verify prefix/exhaustion/cleanup and beginning/midpoint/near-end cursors (/tmp/kestrel-sqlite-deep-runtime.log). Cache target is not a total-memory cap; filter-change cursor retention is experimental. PAGE sums build/prepare/bind/step/reset/finalize; step includes verification. Open/reporting/yields/UI queues are excluded. Host checks verify 73 ordered NAV/PAGE records at one/10k rows and phase sums; P4 build passes but these phases remain unflashed/carrier-unqualified.

Cancellation interrupts at the first progress callback, removes it, finalizes and verifies reuse on host/build/carrier (/tmp/kestrel-sqlite-cancel-runtime.log); no concurrent UI/deadline qualification.

Build make -C kestrel_interface compile-eff; library_check.py FILE.EFF [...] defaults to production --info without DSP encoding. Alternatively --effects-dir DIR enumerates immediate .eff files; input forms are exclusive and empty directories rejected. Optional --info-reader PATH batches at most 64 files through the isolated INFO-only reader; its owner explains validation and ownership limits. Reports identify reader mode/binary hash. Fourteen predicates/all pages verify independently; --copies/--page-size exercise replicas/ties. Source text is preserved; memberships deduplicated. Required facets union across ALL/intersect across ANY; NOT supplies none. Branch planning permits 32 conjunction combinations, chooses counted positive seeds and merges streams; unrestricted branches prevent seeding.

skew_check.py --rows N --output REPORT FILE.EFF [...] verifies twenty-two predicates. Desktop-measurement owner holds evidence. Next qualify realistic metadata/skew, phase costs, cache churn, cancellation/UI/rename/refresh on carrier. Integration owner governs packaging, SD handoff and partitions.
