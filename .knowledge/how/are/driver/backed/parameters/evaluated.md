---
status: green
revised_at: "2026-10-05T06:28:05+11:00"
---

A scope-entry driver stores a borrowed key; evaluation resolves and caches its scope entry, then evaluates that entry in a supplied scope. Cloning allocates an independent runtime payload while retaining the key pointer and clearing cached scope/entry. clone_for rebinds these caches to the destination effect.

Cloning rejects a scope-entry driver without a payload with ERR_BAD_ARGS. make_clone frees its wrapper and returns NULL on clone failure. Descriptor instance construction preserves clone errors and frees an unattached driver payload if list append fails, so pipeline append can reclaim the failed unpublished instance rather than publish an incomplete driver.

Host regressions check a malformed payload across three failed pipeline append attempts with no publication, and a valid clone's independent runtime payload, borrowed key and cleared caches. Parameter extraction checks driver construction/append results and frees an uninserted payload on append failure. Rejection retires only newly appended driver payloads; prior drivers remain. Newly created list storage is released. Three cases over three passes, with/without prior drivers, restore slots after fresh parameter trees are reclaimed; the arena owner governs that boundary. The standalone probe injects parameter wrapper, three name copies, driver payload and list allocation failures over three passes; all reject without tracked leaks or borrowed-expression changes, then recover. Complete direct-constructor rollback outside this path remains unqualified.

Sources: components/core/kest_driver.c, kest_effect.c, kest_pipeline.c and tests/core/kest_context_test.c.
