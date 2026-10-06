---
status: green
revised_at: "2026-10-06T10:01:29+11:00"
---

The parser uses one serialized global 512-KiB bump arena for source, transient tokens/sections/strings and its scope wrapper. Initialization is idempotent; reset requires initialization; deinit destroys storage. Reset does not reclaim persistent metadata or expression-pool nodes. Parameter/setting/resource templates use matching typed allocation/release.

File parsing captures new expressions in a descriptor-owned pointer list. Ordinary constructors and generated LPF/HPF/BPF coefficients use capture. Fresh-tree cleanup untracks freed nodes; rejection destroys remaining captured nodes/owned reference names. Success transfers the list with copied names/extracted metadata. Capture rejects nesting; serialize concurrent parsing/evaluation allocation before reload.

Discovery strings/list entries are copied to descriptor-owned heap storage after validation. Reader cleanup consumes fresh INFO values/list arrays on success or rejection. Partial copying retires the unpublished descriptor and releases other parser outputs. Thirteen allocation boundaries reject/recover over three passes without retained heap; metadata survives arena reuse/retirement, including long strings and empty lists.

Retained instances delay descriptor destruction until final release. Destruction frees owned metadata/payloads/strings/captured nodes once without traversing shared children or freeing borrowed/static nodes. This is allocation provenance, not an expression arena. Payload variants, partial construction and scope ownership still need focused review/fault tests.

The temporary scope borrows persistent/global values and is not transferred. Its dictionary keys/storage are destroyed on success/failure; reset reclaims the wrapper. Assembly lines/cells are arena-owned. Reader rejection releases parameter/setting/resource/definition metadata; fresh trees are reclaimed on parse/insertion/container failure. List arrays use their stored allocator.

Installed capture/retirement and repeated Swamp reloads restore pools. Discovery metadata additionally passes three Swamp descriptor and three active Bass Ring reloads, malformed-field rejection preserving valid metadata, exact-file restoration and pool-baseline recovery. The carrier owner holds identity/logs/limits. 211 host tests/allocation probes qualify narrower ownership variants; generated graphs/payloads/exhaustive faults remain open.

David suggests arenas for long-lived expressions with extensive traversal and rare individual death; storage architecture remains unselected. Refresh must manage instance/UI/control/SPI/smoothing/file borrows and publication under its design owner.

Sources: parser/expression/descriptor code, parser regressions and allocation/carrier metadata probes.
