---
status: green
revised_at: "2026-10-05T05:02:43+11:00"
---

kest_scope_init creates a 32-bucket dictionary and inserts pi, tau, e, sample_rate and data_width as borrowed global expressions. Any insertion failure destroys dictionary keys/storage, resets count and returns the error. make test-parser-allocation fails all ten allocations individually, checks complete direct-scope reclamation, then separately checks three rejected descriptors and recovery. Whole-descriptor rollback remains incomplete. Evidence: /tmp/kestrel-scope-init-after.log; the old destructor failed the reclamation assertion.

kest_effect_create_scope adds parameters, settings, named definitions, memory and LFO entries, links LFO backlinks and detects dependencies. Memory evaluation loads the atomically published latest sample; the updater consumes a coalesced arrival flag.

The function publishes effect->scope only on success. Insertion/failed LFO lookup returns an error; failure clears LFO backlinks, destroys temporary dictionary/dependency lists and frees the scope. Other unpublished members belong to caller rollback. Replacing an existing scope remains unsupported and leaks its former owner.

The memory-resource regression checks three failed appends without a name, no publication and returned effect/resource slots; restoring the name permits construction/lookup/reclamation. Other insertion/dependency failures and physical pressure remain unqualified. The repair is installed; smoke/flanger lifecycle passes with baseline pool restoration. The installed-firmware owner records identity and physical evidence.

Sources: components/core/{kest_effect,kest_expr_scope,kest_resource,kest_update,kest_dict}, tests/core/kest_context_test.c and tools/test_parser_allocation.c.
