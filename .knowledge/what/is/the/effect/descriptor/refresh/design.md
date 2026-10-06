---
status: green
revised_at: "2026-10-06T03:16:03+11:00"
---

Accepted requirements: reusable background startup/SD flush-repopulation, updating everything retained and exact cname identifying updates despite changed files. Full background refresh is unimplemented.

Reload uses Kestrel's established dual-pipeline transition machinery. David reports audibly seamless swaps and deliberately added tails working before 2026; the Core mixing owner describes that behavior. Recent reload tests bound their own schema/population coverage, not the maturity of the transition engine.

eff-reload <name.eff> updates a loaded cname under SD/model ownership, stages affected pipelines, preserves IDs/order, rebinds selector/buttons and requests active reprogramming through the existing updater. Queue rejection restores pointers. Sequence-member pages preserve their sequence parent; constructor regression and physical inactive normal/257-block reloads qualify the former traversal hang repair. UART parser stack is 16 KiB; installed owner governs identity.

Migration matches parameters by name/units and settings by name/type/units. Finite compatible values survive; added/incompatible controls default. Enum clones preserve type; host tests check renamed/removed-choice defaults, surviving number/name pairs after reordering/addition, and narrowed-range rejection preserving the source instance. RHYTHM preserves edited enum values through active reload. Invalid/nonfinite/inverted bounds reject; resources/dependencies rebuild. Wet mix/band mode/cutoffs survive; DSP state resets. Duplicate schemas, cycles, drivers and broader enum variants need review.

Two-preset RHYTHM carrier reload reports affected=2. Active temporary Preset 10 retains Feedback 0.2/Mix 0.4/Damping 800/Quarter 24; original Preset 8 subsequently activates with Feedback 0.65/Mix 0/Damping 2500/Dotted eighth 18. Both retain Tempo 120. Cleanup restores nine presets and all pool baselines with Swamp active/status 0x01 (/tmp/kestrel-rhythm-two-preset-reload.log). This fixture records firmware/resource state; its log does not measure the waveform of that specific reload.

Host staging checks IDs/values, unchanged source pipelines and incompatible-bound rejection. A four-slot effect pool with two source instances and one held slot forces partial staging failure: three attempts reclaim the first clone, preserve source values/IDs/reference counts, then succeed after releasing the held slot. All 207 host tests pass (/tmp/kestrel-reload-pool-failure-tests.log). This is one bounded failure path, not exhaustive allocation/queue qualification. Allocation probes restore rejected heap/expressions. Descriptors capture expression nodes and retain/release ownership; instances borrow strings/graphs, released after the final instance. Payload/static/scope coverage remains incomplete.

Separate 100-Hz smoothing passes and file saves hold the model lock; reload cancels affected positional targets. Control/SPI/UI retirement is asynchronous. Swamp/CROWD swaps reclaim pools and retain controls under their owners.

Persistence stores cname plus positional values without names/counts. Offline schema changes can misassign/reject records. Live reload queues saves. RHYTHM old-descriptor recovery and Damping addition preserve Preset 8 across restart; its owner holds evidence. Arbitrary schema persistence remains unqualified; named/versioned storage is a proposal.

Full-scan proposal: stage/reconcile every preset by cname, including unchanged bytes; add identities/reject duplicates. Filename/display name are not identity; absent explicit cname, title changes can change derived identity. Failure preserves live state; budget old-plus-new memory. Missing in-use identities need retain/block/remove policy. Empty differs from failed/incomplete scans. Publication/startup UI policy remains open; startup may publish partial results/suppress errors.

Remaining: broader populations/schema variants, concurrent pending work/settings navigation, SD loss, peak-memory/queue failure, numerical delivery and reload-specific waveform behavior across those cases, plus wider persistence. Sources: console, model/control/file code and David.
