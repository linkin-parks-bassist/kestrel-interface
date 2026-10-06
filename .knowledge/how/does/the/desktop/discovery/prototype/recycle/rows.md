---
status: green
revised_at: "2026-10-06T17:20:25+11:00"
---

KEST_DISCOVERY_VIRTUAL_ROWS=1 enables row reuse in tools/discovery_preview.c for effect results, main categories and filter-value pickers. Default mode materializes every row. This is desktop proposal code; carrier firmware is unchanged.

ROW_SLOTS is DISPLAY_VRES/STANDARD_BUTTON_SHORT_HEIGHT+2, eighteen on desktop. An invisible extent supplies full scroll height. Scroll callbacks derive the first index, rebind labels/callback data, position slots and hide those beyond the count. Main views retain one row above the viewport; the picker also accounts for its Back row.

Effect slots borrow descriptors and use existing preset-add/capacity handling. Main category slots call show_facet with their rebound name. render disables reuse before deletion, frees result/tag pointer arrays and resets scrolling. Static slots persist; descriptor/string lifetime is unchanged.

facet_rows collects borrowed tag pointers, qsorts case-insensitively with strcmp spelling tie-break and removes exact duplicates. Case-distinct values remain separate. Main views own the sorted array until render; materialized views free it after creating rows. Filter pickers have independent slots/state, preserving underlying results. Selecting a slot inserts its current name into the draft. Extent deletion removes its scroll callback and frees its pointer array on selection, Back or dialog closure; clean_editor_content restores flex layout and scroll origin.

The 1005-tag fixture yields 1003 categories with eighteen tag buttons in either reused view. Main category selection after scrolling and picker selection of rebound tag-0041 pass. Picker Back, Cancel/reopen and Apply from an empty draft return correctly. Seven mixed-case/duplicate values yield ALPHA/Alpha/Beta/beta/zeta. /tmp/kestrel-picker-result.json binds source, logs and seven pixel-identical comparisons: small full/reused picker, four authored filter captures, and effect-results top/Clear. Rule-edit and result-switch scripts also complete.

Earlier main-category checks are in /tmp/kestrel-facet-sort-result.json; five effect-row comparisons, 1000-effect selection and search/filter switching are in /tmp/kestrel-virtual-{rows-result,result-switching}.json. The original full-widget 1000-keyword attempt timed out; exact cause remains unproven.

Full descriptors, tag/result pointer tables and synchronous scans remain. Bounded widgets do not establish bounded catalogue memory, MCU cost or physical scrolling performance. Production architecture/UX remain for review. Inspection procedure owns runners/scripts.
