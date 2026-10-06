---
status: green
revised_at: "2026-10-05T13:39:18+11:00"
---

Startup is synchronous. After memory/context/pages, FPGA tasks, SD and directory initialization, kest_init holds the UI lock for load_effects, effect-selector initialization, saved presets and saved sequences. It then queues UI creation, restores saved page/state, starts file work and initializes footswitches.

list_files_in_directory closes successfully opened directories, including allocation exits. Returned filename strings/list cells are owned; failed append frees the unattached string. All three loaders release temporary lists. Preset/sequence readers copy filenames into fixed arrays; the parser copies its diagnostic filename into its arena.

The scanner cannot prove completion: NULL represents empty, failed-open or unavailable storage; allocation failure returns a partial list. It does not inspect errno around readdir, so read errors look like completion. It skips only entries tagged DT_DIR, performs no extension filtering and concatenates the supplied directory prefix directly with d_name, requiring a trailing separator. Unknown entry types are admitted. Refresh needs explicit scan status and file-admission rules; source behavior is not a selected future contract.

load_effects appends accepted descriptors directly into cxt->effects, resets the global parser arena between files and returns NO_ERROR despite individual parse/append failures. Appending failure lacks complete descriptor retirement. Complete/empty/failed scans and unpublished-descriptor ownership are prerequisites for reusable refresh.

load_saved_presets initializes typed owners, decodes files, rectifies IDs, appends accepted reads and creates views; failed reads release owners. load_saved_sequences reads main/additional sequences and creates views. Completion flags follow complete loops. The main-sequence page represents references, not the whole preset collection. Staged decoder/pipeline/persistence owners govern remaining rollback gaps.

The selector builds button wrappers from global descriptors; refresh_effect_selector returns ERR_UNIMPLEMENTED. Dependent loading/publication touches UI state. USB file commands do not refresh descriptors.

Required behavior is background loading and reusable SD reconciliation, updating everything retained with exact preserved cname identifying an updated effect. Private worker scanning/parsing followed by UI publication is a candidate. SD/parser serialization, startup screen and publication policy require review.

Instances borrow descriptor names/graphs; changing effect->eff does not rebuild controls/resources/blocks/scope. The refresh-design owner holds migration, missing/invalid/duplicate identity, state, persistence, failure and concurrency proposals. Saved positional values lack old field identity for offline schema migration.

Sources: core initialization/files/effect/parameter code, parser and selector/sequence UI. Host directory tests check handles after empty and repeated populated scans. Allocation exits, read-error semantics and complete startup consumers are source-reviewed without injected scan failures.
