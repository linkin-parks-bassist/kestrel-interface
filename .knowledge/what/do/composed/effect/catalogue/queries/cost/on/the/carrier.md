---
status: green
revised_at: "2026-10-06T03:11:33+11:00"
---

Suitable cached composed queries return pages in about 5–11 ms on the P4. Fresh-page latency is largely SD reading; aligned internal transfer buffers substantially reduce it by enabling multi-sector commands. Poor scan strategies remain slow even cached. No production schema/VFS/query strategy is selected.

SQLite 3.53.4/IDF 5.3.3, 360-MHz P4/200-MHz PSRAM; superproject tools/sqlite_probe. The 2822144-byte synthetic 10k database includes descriptions, overlapping four-axis facets, tied names and covering indexes. Each measured variant passes 50 complete traversals and 50 immediate first-page checks against a C evaluator; cleanup passes, stack margin 3708 bytes. Query-end SQLite allocations span 142040–1493600 bytes, not peak memory; the aligned variant adds a static 4096-byte internal buffer outside SQLite accounting.

Single-run first-page milliseconds including C verification. Fresh means a new SQLite connection, not proven cold FAT/card caches. Cached column is the aligned run's first page after its complete walk.

| Predicate | Strategy | Results | Fresh stdio | Fresh direct | Fresh aligned | Cached |
|---|---|---:|---:|---:|---:|---:|
| Bass AND (delay OR modulation) | Covering seed | 1071 | 120.506 | 93.811 | 37.527 | 6.843 |
| (Bass OR keys) AND ambient | Covering seed | 363 | 143.598 | 113.374 | 45.538 | 7.844 |
| (Ambient AND watery) OR (rock AND crunchy) | Top-level covering UNION | 331 | 165.374 | 127.774 | 48.164 | 6.589 |
| Bass AND keys | Covering seed | 500 | 138.836 | 106.455 | 47.887 | 11.507 |
| Ambient AND watery AND airy | ID sets/table | 4 | 269.138 | 194.699 | 64.623 | 12.263 |
| Bass AND absent keyword | Covering seed | 0 | 35.673 | 30.178 | 13.000 | 0.512 |

Bass seed's ten VFS reads request 40960 bytes. Direct: 86.760 ms, 87 single-sector/one multi-sector command. Aligned: 30.549 ms, eighteen single/ten multi commands. SD bytes are 48640/50176 including filesystem reads. UNION direct/aligned read time is 120.976/41.433 ms for fourteen VFS reads, with 122/23 singles and one/fourteen multis. IDF sdmmc_read_sectors splits unsuitable DMA buffers into individual sectors; this measured transfer-shape change explains much of the bottleneck. The buffer copies each read; it is not a page cache.

All immediate/after-walk first-page repeats perform zero VFS/SD reads. Ordered absent EXISTS still costs about 241 ms cached. Materialized ID/table bass costs about 90 ms cached versus the 6-ms covering seed. Explicit Boolean nesting and top-level ordered UNION remain important; cached UNION whole-walk stepping is about 39 ms versus wrapped 148 ms and EXISTS 607 ms.

On one aligned connection, 72 alternating filter visits pass with a 256-KiB cache target: 36 beginning, 24 midpoint, twelve near-end. Beginning visits span 0.523–54.486 ms, midpoint 0.537–27.744, near-end 0.412–18.335. The beginning maximum is the rare predicate. Each prefix and short-page exhaustion is verified; cleanup passes. SQLite visit allocations are about 270 KiB, not peak or a total-memory cap. Evidence: /tmp/kestrel-sqlite-deep-runtime.log. Synthetic 10k distribution, no UI.

Evidence: /tmp/kestrel-sqlite-{cache,counted,dma}-runtime.log, raw-host.log, counted-build.log and dma-verified-build.log. Open/prepare/bind/UI and inter-page yields excluded. Larger realistic distributions, broader navigation/cache churn, cancellation/UI contention and writable-VFS/USB ownership remain unqualified.
