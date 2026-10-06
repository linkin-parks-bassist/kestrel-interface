---
status: green
revised_at: "2026-10-06T09:50:57+11:00"
---

kest_dictionary.c has legacy named entries and current kest_eff_entry dictionaries/lists holding typed strings, expressions and nested containers. Typed lookup checks the requested type. Current dictionary insertion stores keys separately from values.

Current parsing stops on failed insertion; later success cannot hide duplicates. Entry outputs initialize empty. String/dictionary allocation failures propagate; missing closing parentheses/braces reject. Anonymous list values need no names. Empty lists are accepted and leave the closing-brace cursor; final items need no trailing comma. Adjacent strings without separators reject. Legacy kest_parse_dict_list still constructs qualified keys and checks names.

Failed entries/containers recursively reclaim completed children and reset outputs to NOTHING. Once temporary extraction borrows are retired, kest_free_parsed_eff_entry releases fresh Pratt expressions/reference names and list backing arrays. Strings, keys, dictionaries and container wrappers remain arena-owned. Never use it to destroy extracted/shared graphs. INFO list arrays are consumed by the reader after descriptor copying or rejection.

Regressions preserve the first string and reject duplicates with ERR_DUPLICATE_KEY/one diagnostic, leaving later entries unparsed. Repeated rejected expressions/nested containers, malformed children/missing closers and initial-list/growth allocation faults restore slots/heap while preserving unrelated constants. Anonymous 0/−1 coefficient parsing checks the closing cursor; the removed unused-name check had read uninitialized data and intermittently rejected BASSRING startup.

Disabled PRINTLINES_ALLOWED avoids diagnostic string construction. Enabled entry diagnostics release their wrapper/buffer; recursive/debug formatting remains unqualified. Allocation probes check scope-table reclamation/recovery and non-dictionary resource rejection. The sections/arena owners govern complete rollback and shared-graph limits; installed-image owner governs carrier evidence.

Sources: components/parser/kest_dictionary.[ch], expression parser, 211 host tests and allocation probes.
