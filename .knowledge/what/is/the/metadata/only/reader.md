---
status: green
revised_at: "2026-10-06T15:20:59+11:00"
---

tools/info_reader_preview.c is an isolated host experiment for extracting .INFO without constructing effect graphs. It includes the current parser implementation to reuse its tokenizer, section-header recognition, dictionary parser, discovery validation and cname transformation; it is not a production reader API.

Build/run from the Interface checkout:

```bash
make -f Makefile -f tools/info_reader_preview.mk bin/lib/info-reader-preview
bin/lib/info-reader-preview ../effects/LEVEL.EFF
```

Multiple file arguments are processed sequentially in one process. Stdout uses the compiler's KEST eff-info records. Stderr reports parser-arena use and live descriptor/expression/parameter/resource pool counts before cleanup. Parsed INFO values and captured expressions are released; the global arena resets between files. The tool prints metadata before retiring its borrowed strings; it does not return persistent records or provide concurrent reader contexts.

All 27 authored effects match compile_eff --info metadata exactly, with zero use of those typed pools and a maximum 53476 arena bytes. A 1,000-file sequential fixture preserves every expected name, uses no typed slots and peaks at 4492 arena bytes per file. Existing pools are still reserved by host initialization; these figures do not measure total heap or production SD cost.

Ten focused cases check optional fields, explicit cname, empty/duplicate lists, long names/descriptions, INFO after CODE and wrong field types. An unknown instruction is deliberately accepted as metadata but rejected by the full compiler: listing metadata does not establish a valid executable effect.

The experiment tokenizes the entire file and shares the existing global parser arena. Metadata-reader API, serialization/context ownership, persistent copying, background work and complete scan/activation policy remain for architectural review. library_check.py --info-reader imports through this protocol in batches of at most 64 files; --effects-dir enumerates immediate .eff files without shell glob expansion. Reader/protocol failure rejects import before database construction. The direct reader still accepts sequential arguments in one process. The query checker defaults to the full parser. The 27 authored imports have identical metadata, database size and all fourteen predicates' page/seed results across both readers. A 1,000-file single-template import passes fourteen independent query checks across 186 pages in a 598016-byte database; it is not a diverse library or carrier performance measurement. Bounded imports preserve all 1,000-file metadata/database/query results. A 40,000-path repeated-file API check reproduces E2BIG without batching and succeeds with batching; it is not a diverse-library performance measurement. A missing file in the second batch rejects import. Reports bind reader binary hashes. No carrier integration or new dependency is adopted.

Evidence: /tmp/kestrel-info-reader-library.json, /tmp/kestrel-info-reader-edges.json, /tmp/kestrel-info-reader-batch.json and batch.stats; /tmp/kestrel-sqlite-reader-{full,info}-27.json and /tmp/kestrel-sqlite-info{,-bounded}-1000.json; /tmp/kestrel-info-reader-bounded-40000.json; prototype/Makefile and checked production parser/compiler source.
