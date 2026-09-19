---
status: "unverified"
created_at: "2026-09-20T00:07:54+10:00"
scope: "local"
source: "components/parser/kest_tokenizer.c:13-625"
---
Status: Green

`kest_tokenizer.c` is a character-state tokenizer. It recognizes identifiers, decimal/binary/hex numeric tokens, quoted strings with escapes, newline and punctuation tokens, and records source line/index in token list nodes. Its policy rejects newline inside strings. Source: components/parser/kest_tokenizer.c:13-625
