---
status: "unverified"
created_at: "2026-09-20T00:06:18+10:00"
scope: "local"
source: "components/drivers/kest_touch_irq.c:11-50"
---
Status: Green

GPIO 4 IRQ notifies a pinned FreeRTOS touch task. The task waits for notification, takes the I2C mutex with timeout, reads GT911 data in task context while IRQ is low, then releases the mutex. Source: components/drivers/kest_touch_irq.c:11-50
