---
status: "unverified"
created_at: "2026-09-20T00:07:53+10:00"
scope: "local"
source: "components/parser/kest_dictionary.c"
---
Status: Green

`kest_dictionary.c` stores typed entries in growable dictionary buckets and provides add/lookup helpers for strings, integers, floats, expressions, nested dictionaries and lists. The typed lookup path checks the requested entry type before returning a value. Source: components/parser/kest_dictionary.c
