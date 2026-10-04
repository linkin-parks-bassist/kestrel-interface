---
status: green
revised_at: "2026-10-04T09:05:16+11:00"
---

components/ui/kest_button.c implements generic LVGL button creation, labels, callbacks, visibility, clickability and async enable/disable. Active buttons add play state, reorder/delete gestures and animations. Danger Buttons open a confirmation popup with the existing confirm/cancel callbacks.

The Danger Button popup uses LVGL's standard message-box footer flex layout. Its height is DANGER_BUTTON_POPUP_BUTTON_HEIGHT + 2 * GLOBAL_PAD_WIDTH, with top and bottom padding GLOBAL_PAD_WIDTH. The footer positions both buttons; manual align_to calls on its flex-managed children are unnecessary and removed. This gives the row breathing room below it without a custom widget or callback changes.

At the current 600×1024 desktop geometry, buttons moved from y=594…639 to y=572…617 inside the unchanged popup y=384…639. Automated pointer input opens the dialog and Cancel removes it. David accepted the raised layout as perfect. Desktop and pinned ESP-IDF builds pass, as do the C suite. The carrier image includes this adjustment and is flashed, but its popup layout/cancel path has not been independently checked on hardware. Screenshot/interaction reproduction belongs to how/to/inspect/the/desktop/ui.md.

Sources: components/ui/kest_button.c, LVGL message-box implementation, automated desktop tree/screenshot/cancel checks and David's visual acceptance.
