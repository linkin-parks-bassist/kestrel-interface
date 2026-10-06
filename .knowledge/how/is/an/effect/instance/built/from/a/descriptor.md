---
status: green
revised_at: "2026-10-05T23:11:10+11:00"
---

init_effect_from_effect_desc initializes the instance, clones resources/blocks/parameters/settings, creates its scope and relinks resources in cloned blocks. Initialization sets Wet Mix to 1, full-spectrum band mode and LP/HP cutoffs to 4000/1 Hz. It does not preserve another instance's controls. Linkage waits for scope/resources, establishes register/coefficient dependencies, clones drivers and transitivizes dependents.

Parameter clones copy values/literal bounds but borrow min_expr/max_expr and names/units. Block clones share register expressions; resource linkage is rebuilt except LUT references. Filter clones own coefficient-pointer containers with shared expressions. LFO/delay payload copies retain expressions. Scopes insert descriptor definitions without graph cloning. Setting clones preserve type, copy values/options arrays and borrow strings including option names. Installed type preservation fixes reload compatibility: host tests default renamed enum choices and retain surviving number/name pairs. Physical enum migration remains unqualified. Drivers own cloned payloads, borrow source keys and resolve entries against instance scope.

Construction retains the descriptor; retired-instance destruction releases it after members. Descriptor retirement waits for the last instance before freeing captured expressions/borrowed strings. This is installed; repeated Swamp reloads reclaim typed pools. Separate smoothing passes and file saves share the UI/model lock. Broader lifetime variants remain unqualified. Instances release owned containers without recursively destroying shared expressions. Expression arenas remain unselected; caching belongs to the expression-values owner.

Resource wrappers use kest_dsp_resource_allocator for allocation/release. Payloads remain heap-backed: memory/LFO/delay constructors receive NULL allocators; filters use the heap clone helper. Parameters/settings use matching typed allocators. The pipeline owner governs release and queued borrows; payload allocator changes require compatible release.

Resource clone failure returns NULL before assigning an effect backlink. Driver errors propagate; failed list insertion frees unattached payload. Scope insertion/lookup failure destroys temporary scope and returns an error. The constructor's disaster-recovery label leaves member cleanup to unpublished pipeline callers or direct/stack callers. Complete dependency/error rollback is unqualified.

Sources: components/core/{kest_effect,kest_parameter,kest_block,kest_resource,kest_driver,kest_pipeline}.c, tests/core/{kest_update_test,kest_context_test}.c and /tmp/kestrel-enum-clone-{before,after}.log. Installed identity/boot/reload evidence belongs to the carrier firmware owner.
