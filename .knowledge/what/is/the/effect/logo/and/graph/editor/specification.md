---
status: green
revised_at: "2026-10-05T16:52:01+11:00"
---

David requests logos embedded as text in .eff files, approximately 64×64 pixels: candidate encodings are 0/1 bitmaps or letters representing palette colors. Render compact logos where effect names are too long. Exact dimensions, palette/transparency, section grammar, validation, fallback appearance and memory/rendering limits remain to be chosen; no logo parser or renderer is established.

Full DSP graphs are deferred to V2.0. The near-term product priority is a large library of quality effects and preserving its already simple, beautiful, usable linear UI. Logos have no assigned release priority.

The proposed preset graph editor is a four-column grid scrolling downward as needed, with connected squares and flow arrows determining data routing. Buttons switch between graph and linear views. Effects/blocks expose 1–4 named outputs by writing symbolic output values inside .eff; inputs use symbolic input names rather than assuming c0, potentially with multiple inputs. Define port naming/count declarations, connection editing, fan-in/fan-out, disconnected ports, feedback edges and persistence, plus what linear view can represent without losing graph topology. Finite hardware capacity remains visible despite an extensible canvas.

The compiler allocates physical channels/registers from symbolic values, reorders instructions subject to dependency/state constraints, and allocates scratchpad spill storage. what/is/the/symbolic/dsp/composition/direction.md owns lowering, band wrappers and time/capacity admission. This GUI must make powerful routing approachable enough for a five-year-old and meet David's exceptionally high reliability ambition; those are acceptance goals, not achieved guarantees or evidence that FPGA use alone assures reliability. Prototype interaction and define testable usability, safe-edit and reliability criteria before adoption. Detailed UI/grammar remain open; graph work does not block the linear product.
