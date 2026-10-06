---
status: green
revised_at: "2026-10-06T18:09:07+11:00"
---

212 C tests pass via make tests && ./kest_tests (/tmp/kestrel-mainline-qualified-tests.log), covering blocks/constants, arenas, containers/strings, pools, parameter allocation/cloning, widgets and readback.

Capacity tests accept 256-block/empty chains; reject 257 blocks, oversized effects and independent instance-count/descriptor-increment overflow. Compilation rejection preserves positions/empty output. Append rejects before publication; a full-preset LVGL click preserves pipeline/screen and opens the rejection modal. Direct/context/sequence activation and begin/begin_at/next/previous rejection preserve flags, pointers and cursors. Preset-page construction preserves sequence parent. Physical sequence overflow, queue rollback and sample timing remain open.

Template tests cover exhaustion/retention/labels/rejection/reuse and partial-polynomial cleanup. Typed constructors/pipeline rollback cover exhausted resources, partial settings, missing driver and unnamed-memory scope insertion; naming restores scope. Driver clones borrow keys/clear caches. Complete dependency/constructor rollback and broad heap injection remain uncovered.

Enum reload preserves surviving numbers/names through reorder/addition, defaults removed choices and rejects narrowed ranges. Partial-clone exhaustion reclaims earlier clones/preserves source; retry works after releasing the slot. Choices cover declaration order, LVGL selection, parser reset/retirement, rejection/recovery, integer/range/bounds guards and five allocation faults. Dropdown events rebuild changed active effects, suppress unchanged values and dirty only inactive edits. Tempo/subdivision fixtures cover repeated instance allocations/defaults. Widgets cover dirty initialization, shared configuration, Gain labels and mixed bounds.

Sequence deletion covers saved-preset/file removal, empty references and slot return. Time-reference tests check a two-second epoch-relative value and exact t dependency recognition; failed queue submissions preserve epoch/state. Scope allocation checks cover all twelve initialization allocations with the borrowed t entry. Continuous time modulation, commit-aligned epoch timing and physical performance remain unqualified.

Updater tests cover signed/error-preserving/concurrent publication, coalescing, cadence/disable/reprogram, unstarted queues, rejected submission retention and clone isolation; polynomial coefficients use one commit. Physical streaming remains open. Retirement covers clones, idempotent handoff and retention through reprogram/cache clear/queue rejection with modeled completion/timers. UI covers callbacks/backlinks/repeated cleanup/embedded preservation/dirty memory; full loops/FIFO remain outside tests.

Discovery tests retain all five metadata fields through arena reuse and retired references; preserve a 265-character description, case/order/duplicates and empty lists; and reject twenty malformed combinations with emptied dictionaries/recovery. String lists allow final items without commas but reject adjacent strings. make test-parser-allocation qualifies all thirteen discovery-copy allocation faults over three passes with heap restoration. Reader consumes transient INFO list arrays. Parser tests also cover unnamed delimiters, exhaustion/cursors, Pratt/container/section rollback, invalid attributes/channels, operands/discards and poisoned locations. Scope/parameter/setting/definition/list/block allocation probes preserve prior blocks/borrowed expressions; shared-graph reclamation remains open.

Persistence covers null saves/empty presets, directory closure, gains/filenames/pages, invalid headers/truncated prefixes/overlong names preserving destinations, populated sequences/playback/missing references/joins/rollback, preset identity/values/resources/signed settings and slot return. Exhaustion/missing descriptors preserve destinations; unpublished instances reclaim. Replacement, trailing bytes, queued-save/deletion concurrency and interrupted writes remain open.

Integer-field events cover whole signed input/endpoints, overflow/decimal/junk/empty rejection without mutation/dirtying, retry and clamping. Numeric tests cover signed/unsigned 16/24-bit endpoints, tuples/registers/aliases, transformed ranges/dependencies, initial/queued equivalence, saturation/shifts/limits, encoding errors, ERF/LOG10, subtraction and SVF cutoff. Parser fixtures cover send transforms, expression/channel SVF, shifts, excess registers and polynomial types/counts/malformed lists.

Arbitrary descriptors/resources, complete persistence and physical SPI/codec/UI remain unqualified. Superproject/Core own compiled DSP/hardware evidence. Sources: tests/ and tools/test_parser_allocation.c.
