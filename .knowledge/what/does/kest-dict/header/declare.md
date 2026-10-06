---
status: green
revised_at: "2026-10-05T04:59:51+11:00"
---

kest_dict.h supplies value and pointer dictionary macros, allocator-aware initialization, insertion/lookup/indexing and destruction.

Value dictionaries copy each inserted key with their allocator. All three value insertion variants release that copy if bucket append fails. Value destruction frees stored keys, bucket arrays and the bucket table, invoking an optional destructor only for values. Borrowed expression globals remain borrowed when that destructor is null.

The scope allocation probe covers normal value-key reclamation and every allocation in the five-symbol scope initializer; pointer-dictionary ownership and arbitrary insertion variants are separate qualification. Sources: components/core/kest_dict.h and tools/test_parser_allocation.c.
