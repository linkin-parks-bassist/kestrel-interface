---
status: "unverified"
created_at: "2026-09-20T00:04:02+10:00"
scope: "local"
source: "components/core/kest_files.c:998-1160"
---
Status: Green

`load_effects` scans KEST_EFFECT_DESC_DIR, parses each descriptor and appends successful results, resetting the parser arena between files. `load_saved_presets` scans preset files, initializes and reads each preset, rectifies IDs, appends successful reads and creates views. `load_saved_sequences` reads the main sequence and scans additional sequence files similarly. Source: components/core/kest_files.c:998-1160
