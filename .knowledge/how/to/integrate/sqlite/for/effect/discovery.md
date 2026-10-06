---
status: green
revised_at: "2026-10-06T13:47:25+11:00"
---

SQLite adoption is planned, conditional on clean single-package integration and substantial discovery functionality. main/idf_component.yml/dependencies.lock contain no SQLite component.

Use sqlite3.c/sqlite3.h directly; optional FTS5 needs no second package ([amalgamation](https://www.sqlite.org/amalgamation.html), [porting](https://www.sqlite.org/custombuild.html)).

Isolated P4 probes use SQLite 3.53.4 from the [official download](https://www.sqlite.org/download.html), archive SHA3-256 628a44cfe82c66aed1ccbbe85a562d2e33ebe64b3288981ed76285612227934e. GCC 13.2 compiles core/FTS5 without diagnostics: -Os, rv32imafc_zicsr_zifencei_xesppie/ilp32f, function/data sections, SQLITE_OS_OTHER=1, SQLITE_THREADSAFE=0, SQLITE_OMIT_LOAD_EXTENSION. These flags do not select production concurrency.

Core object text/data/bss: 511066/6264/658 bytes; FTS5: 617864/6264/658, +106798 bytes. Undefined symbols include libc/compiler helpers and sqlite3_os_init/end.

Standalone --gc-sections harness text/data/bss: baseline 1002/12/208; core 500652/7152/1096; FTS5 602818/7444/1096. Text+data increments: 506790/609248 bytes. Stub OS/nosys harness qualifies retained size only. Evidence: /tmp/kestrel-sqlite-p4-probe/{results,linked-results}.json, link_harness.c/maps/logs.

Carrier sdkconfig/flasher_args select 2 MiB/singleapp_large. Factory: offset 0x10000, 1500 KiB. Installed 1263456-byte PPA image leaves 272544 bytes; what/firmware/is/installed/on/the/carrier.md owns identity/evidence. /tmp/kestrel-sgtl-unity-live.log detects 32768-KiB flash with a 2048-KiB image header; generic-driver access beyond 16 MiB is unsupported. Physical flash is not exhausted.

A temporary table retains NVS/PHY/app offsets and extends factory to 0x1f0000 (1984 KiB), ending at 2 MiB. Pinned gen_esp32part.py verifies it. Current-image headroom would be 768160 bytes, enough for either standalone increment. Evidence: partition-2mb-proposal.csv/bin and partition-proposal-results.json in the probe directory; recorded proposal arithmetic used the previous image. No layout was adopted/flashed. Actual firmware linking/image overhead/runtime qualification remain required.

Repeatable carrier probe: superproject tools/sqlite_probe, checksum-pinned fetch/build. Fresh IDF 5.3.3 build passes (/tmp/kestrel-sqlite-repo-build.log), resolving 360-MHz P4/200-MHz PSRAM/16-KiB stack. capture.py resets, waits for readiness, sends g/checks cleanup. Existing KTPROBE.DB blocks publication; inspect interrupted fixtures before deletion. Default: 100k tied names; KT_PROBE_COMPOSED: 10k overlapping multi-axis memberships. Host checks pass; query owner holds carrier evidence. Force-include sqlite_release_assert.h only for sqlite3.c: IDF NDEBUG assert evaluates omitted SQLite debug identifiers. Default main-stack parser fault; 16-KiB passes. One facet/effect, uniform names: fifty bass IDs/zero absent verified in both strategies:

| Rows | EXISTS bass/absent (us) | Facet JOIN bass/absent (us) | SQLite bytes | Free heap | Stack margin |
|---|---|---|---|---|---|
| 1000 | 3683/12570 | 3652/152 | 149808 | 443916 | 12360 |
| 10000 | 4959/164644 | 34947/906 | 832048 | 33302475 | 12332 |
| 100000 | 6316/2154261 | 310510/1120 | 8203144 | 25910655 | 12332 |

10k/100k enable 200-MHz PSRAM; 100k construction yields every 256 inserts. Images 675616/687328/687344 bytes are not Interface increments. Single runs exclude SD/UI. Evidence: /tmp/kestrel-sqlite-idf-{candidate-runtime,10000-runtime,100000-runtime,100000-build,stack-fault}.log.

Immutable SD snapshot passes identical 100k checks/cleanup: 7815168-byte KTPROBE.DB created exclusively, write/fsync/close, reopened read-only, removed. EXISTS bass/absent: 77275/10752267 us; JOIN: 9485917/14053 us. SQLite allocations grow 141464→2138040 bytes (/tmp/kestrel-sqlite-idf-sd-runtime.log). probe_sd.c is experimental. fdopen publication failed; direct writes pass. Production locking/journals/temp-file limits remain unresolved; FAT lacks fcntl. sd_mutex/TinyUSB requires catalogue close before USB/reconciliation after APP returns.

Proposed: rebuildable metadata index outside DSP/control timing. Query owner governs covering-index evidence. Qualify production SD ownership/handoff, Interface linking/heap, realistic metadata/skew/concurrency. Placement/tasks/refresh/partitions remain unselected. No production dependency/layout changes; installed state belongs to its owner.
