---
status: green
revised_at: "2026-10-06T12:14:51+11:00"
---

components/ui/kest_menu.c implements reusable menu items, page links, parameter widgets, preset listings, menu-page lifecycle and the main menu.

A preset row normally opens its view on release. Long-pressing an inactive row exposes its delete control; release starts a one-shot STANDARD_DEL_BTN_REMAIN_MS timer (1000 ms in kest_button.h) that hides it. Tap delete promptly after release: a delayed tap can hit the ordinary row after the control disappears. The delete callback rejects active presets, then calls cxt_remove_preset and removes the menu item. It does not show a confirmation dialog. UART automation should release/tap without inserting a tree dump in that one-second window.

Sources: components/ui/kest_menu.c preset listing callbacks and components/ui/kest_button.h.
