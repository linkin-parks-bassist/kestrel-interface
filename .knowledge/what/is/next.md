---
status: "unverified"
updated_at: "2026-09-20T00:13:24+10:00"
---
Status: Green

For the spec workshop, ask the user which effect lifecycle, control mapping, UI, persistence and live update behaviors they intend; distinguish those decisions from observed implementation. For repairs, prioritize the `make lib` compile break, parser allocation safety, disabled FPGA update path and source-backed stubs according to user goals. Install/locate SDL2 and ESP-IDF only when tests or device work call for them. Revisit relevant code and update leaves as it changes; do not treat an unverified leaf as a runtime guarantee.
