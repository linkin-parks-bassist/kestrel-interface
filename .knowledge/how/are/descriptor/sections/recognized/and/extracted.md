---
status: green
revised_at: "2026-10-06T09:50:57+11:00"
---

Sections are exactly INFO, RESOURCES, PARAMETERS, SETTINGS, DEFS and CODE after newline-dot-name. Dictionaries precede typed extraction; CODE is separate. Section AST nodes retain the name token's source line; the reporting owner qualifies location initialization.

Failed entry parsing clears accepted fresh values and the dictionary. Before extraction, later-section, INFO-validation or scope-init failure clears every section's fresh values, including initialized-empty unparsed sections. Strings/container wrappers stay arena-owned; fresh expression graphs and list arrays require destruction. Extracted/shared graphs are excluded. Repeated malformed-expression, duplicate-key, delimiter and whole-parser failures preserve unrelated held expressions and restore slots/heap; scope-table injection checks rollback.

.INFO requires string name and optional string cname. Discovery description must be a string; keywords/instruments/types/genres must be lists containing only strings. Validation precedes extraction. After code/format success, discovery strings are copied into the new descriptor before transferring other ownership. Copy failure retires the partial descriptor and performs normal reader rollback. The reader consumes INFO fresh values on success or rejection, freeing transient list arrays; descriptor metadata survives arena reset and retained retirement. Twenty malformed-field combinations reject with cleared section dictionaries. Thirteen copy-allocation faults repeat three times with recovery and no retained heap.

Parameter rejection retires new driver borrows and releases failed/unprocessed values. Insertion failures free unattached strings/wrapper and newly added drivers, preserving prior entries/graphs; empty/populated-list and driver-list injection qualifies this. Common reader rejection releases previously extracted metadata. Setting extraction copies strings/evaluates bounds, consumes original fresh trees on success, and releases failed/unprocessed values on rejection. Failed insertion frees unattached metadata and propagates its error. Tests preserve held expressions/copied data through reset and restore heap/slots after append faults.

Parameter/setting/resource entries must be subdictionaries before reading their union as dictionary pointers. Resource insertion failure frees name/payload/wrapper/coefficient pointer container while leaving expression ownership separate. Tests preserve prior entries and borrowed expressions; reader rejection releases extracted resource containers.

Recorded carrier rejection of three string-entry descriptors reports section line 4, preserves eighteen descriptors/789 expressions and clean FPGA status; temporary files were removed. This establishes neither new discovery retention on the carrier nor broader malformed-input/graph coverage.

Sources: components/parser/{kest_eff_section,kest_eff_parser}.c, parser regressions and allocation probe. The arena/installed-image owners govern lifetime and physical limits.
