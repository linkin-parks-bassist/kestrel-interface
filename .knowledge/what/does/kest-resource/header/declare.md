---
status: green
revised_at: "2026-10-04T04:52:14+11:00"
---

components/core/kest_resource.h declares resource initialization/cloning/type mapping/handle assignment, deletion marking/query and instance-clone release, filter creation/cloning, memory-slot creation/address setters/completion callback, delay types, LFO initialization/activation/evaluation and resource reporting. Resource types are LUT, MEM, DELAY, FILTER and LFO; delay units are milliseconds, seconds and samples.

MEM, LFO, DELAY and FILTER payloads contain atomic delete_requested markers, explicitly initialized clear by constructors and clones. Marking does not free anything; the existing control loop owns retirement. kest_dsp_resource_free releases an instance clone's payload and wrapper, including a filter's coefficient pointer container, while preserving borrowed descriptor names/expressions. It must run only after task borrowers have finished; how/does/a/preset/pipeline/manage/effects.md owns that ordering.

kest_mem_slot stores addresses, atomic integer sample and arrival flag, read_enable/read_period_ms and an effect pointer. kest_mem_slot_read_cb publishes a signed sample and marks arrival; a negative transport error or deletion marker leaves both untouched. Creation initializes the atomics and defaults read_period_ms to 10. Cloning copies the sample and metadata but starts with no pending arrival or deletion request. LFO clones do not inherit a timer. There is no polling timer, request token or outstanding-read record.

Sources: components/core/kest_resource.[ch].
