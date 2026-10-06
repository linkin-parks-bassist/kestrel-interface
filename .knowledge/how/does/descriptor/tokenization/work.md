---
status: green
revised_at: "2026-10-06T09:50:04+11:00"
---

kest_tokenizer.c recognizes identifiers, decimal/binary/hex numeric tokens, quoted strings with escapes, newline/punctuation, and records source line/index in token nodes. Newline inside strings is rejected. Missing source or fewer than four version bytes reports a version error before reading the prefix.

Tokens are copied from contiguous source spans into parser storage. There is no fixed 256-byte token stack buffer; a 265-character description passes without truncation or overflow. This uses no second file-sized scratch buffer. The existing tokenizer policies and numeric conversion remain unchanged.

Expression parsing converts numeric tokens through token_to_float, accumulating digits in float and dividing for fractional digits. Numeric text therefore has float precision and accumulated rounding, rather than exact integer semantics; range checks apply to evaluated values. Setting extraction checks finite/integral/int-representable default/min/max before conversion and validates bounds, without introducing a different numeric representation.

Sources: components/parser/kest_tokenizer.c, kest_expr_parser.c, kest_dict_extract.c and tests/parser/kest_readback_effect_test.c; 211 host tests pass.
