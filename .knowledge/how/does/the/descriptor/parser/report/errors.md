---
status: green
revised_at: "2026-10-04T13:09:36+11:00"
---

Token-based info, warning and error reporters in kest_eff_parser.c format the message and available filename/line location. Errors increment ps->errors. An optional highlighted source excerpt is printed only when the token and token->data exist, its one-based line lies in 1..n_lines, the source-line array exists and lines[line−1] is nonnull. Missing token text therefore cannot reach strlen, and the final source line is eligible for highlighting.

tests/fixtures/polynomial-invalid.eff uses an invalid square-bracket coefficient list. Its parser regression requires failure without crashing; the complete 154-test C suite passes. The earlier reporter dereferenced null token text while attempting to print that malformed-input error. These guards cover token-based reporters; they are not comprehensive malformed-parser qualification.

Sources: components/parser/kest_eff_parser.c token reporters and tests/parser/kest_readback_effect_test.c.
