---
status: green
revised_at: "2026-10-06T14:06:42+11:00"
---

Desktop SQLite 3.45.1 checks cover authored metadata and synthetic fixtures; they do not establish P4/SD latency or adopted UI semantics. The query procedure owns strategy selection/target evidence.

library_check.py uses production compile_eff --info. Fourteen parameterized predicates independently check every page: unions/intersections, groups, negation, idempotence, literal SQL-looking values, unsafe single-branch seeds and empty Boolean groups. The current 27-effect authored library verifies 61 seven-row pages in 40960 bytes (/tmp/kestrel-authored-empty-groups-27.json). Empty ALL/ANY identities also pass nested negation, union/intersection, empty databases and one-/three-row page boundaries.

A 25-template, 400-copy comparison has 10000 rows: roughly 7.24 MB with covering facets versus 3.64 MB without. Studied seven-/fifty-row traversals verify 10986/1557 pages. Fifty-row worst host page times: bass AND (delay OR modulation), ordered 13.785 ms versus covering union 0.640; grouped 6.243 versus 0.752. All rows are bass, so a bass-only seed is unselective. Single-run reports: /tmp/kestrel-authored-union-{1,400}-page{7,50}.json (no 1/page50). Replicas are not a diverse library.

skew_check.py uses seven parser-retained templates, seed 31, fifty-row pages. The current 100k generated mixed/skewed fixture verifies 30943 pages across twenty-two predicates; database 98508800 bytes. Duplicate/case-distinct tags, tied Unicode names, empty axes and matched SQL-looking text pass. Empty ALL/negated empty ANY each match 100000 rows; empty ANY matches none; a nested identity preserves all 58430 bass rows. Late-only ordered→covering worst-page costs: 93218→65 us; rare non-distortion 26252→326. Report /tmp/kestrel-skew-empty-groups-100k.json binds generator/checker/template hashes; a 1000-row run verifies 329 pages. Timings include Python/SQLite queries, exclude import/count planning/oracle/SD/UI, and are single runs. Controlled distributions are not realistic-library qualification.

Synthetic comparisons live in /tmp/kestrel-sqlite-p4-probe. catalogue_probe.py/catalogue-results.json checks fifteen SQLite/FTS5 cases at 1k/10k/100k (largest database 50651136 bytes). predicate_probe.py independently checks ordered pages at 100k:

| Predicate | Matches | EXISTS first page, single ms | ID UNION/INTERSECT, ten-run median ms |
|---|---:|---:|---:|
| Cross-axis union | 63879 | 0.346 | 58.887 |
| Instrument intersection | 12500 | 1.022 | 47.544 |
| Grouped | 10583 | 1.955 | 170.987 |
| Nested union | 3665 | 4.680 | 106.317 |
| Disjoint keywords | 0 | 99.159 | 33.146 |
| Idempotent union | 43750 | 0.301 | 48.958 |

predicate_sets_probe.py preserves pages but uses temporary B-trees/sorts. predicate_seed_probe.py forces the smallest counted facet; twenty-run intersection/empty medians including counts 34.164/41.021 ms, counts 2.322/6.821.

page_query_probe.py twenty-run candidate-ID→ordered-EXISTS first-page medians (ms): bass 26.240→0.109; bass/modulation 43.359→0.324; bass-or-keys/modulation/watery 29.217→2.171; bass/modulation with watery-AND-airy text 45.373→5.827. All pages match.

page-empty-results.json ten-run medians: absent keyword 0.0448→55.2537 ms; common watery 4.0242→0.0996. Facet-query five-run direct grouping 591.04→57.12 ms; forcing primary scans improves bass 303.99→258.96 but hurts narrower cases.

pagination_probe.py twenty-run OFFSET→keyset bass medians at depths 0/5000/40000: 0.121→0.120, 10.128→0.147, 84.292→0.051 ms. Tied-name fixture: 1000 rows/twenty pages without skips/repeats against Python BINARY order. Named scripts/results retain parameters/plans. These bounded groups are not full first-order logic; realistic target concurrency/cancellation/refresh remain unqualified.
