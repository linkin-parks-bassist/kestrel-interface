/* Isolated alternative: accelerate three flat rectangles, software corners.
 * Reuse the checked hardware/cache helper; do not change the installed wrapper. */
#define __wrap_lv_draw_sw_fill kest_strip_fill_preview
#include "rounded_fill_preview.c"
#undef __wrap_lv_draw_sw_fill

#ifndef KEST_ROUNDED_FILL_ENTRY
#define KEST_ROUNDED_FILL_ENTRY __wrap_lv_draw_sw_fill
#endif
void KEST_ROUNDED_FILL_ENTRY(lv_draw_task_t *task, lv_draw_fill_dsc_t *dsc,
                          const lv_area_t *coords)
{
    int32_t radius = LV_MIN(dsc->radius,
        LV_MIN(lv_area_get_width(coords), lv_area_get_height(coords)) / 2);
    lv_area_t visible;
    if (!lv_area_intersect(&visible, coords, &task->clip_area)) return;
    /* Masked XRGB8888 blends preserve its unused byte; flat fills overwrite it. */
    if (task->target_layer->color_format != LV_COLOR_FORMAT_RGB565 ||
        /* Carrier list rows are 553 px; smaller control backgrounds keep strips. */
        lv_area_get_width(coords) < 400 ||
        lv_area_get_width(&visible) < 400 ||
        radius <= 0 || dsc->opa < LV_OPA_MAX || dsc->grad.dir != LV_GRAD_DIR_NONE ||
        lv_area_get_width(coords) <= 2 * radius ||
        lv_area_get_height(coords) <= 2 * radius) {
        kest_strip_fill_preview(task, dsc, coords);
        return;
    }
    lv_draw_fill_dsc_t flat = *dsc;
    flat.radius = 0;
    lv_area_t rects[3] = {
        /* Pinned PPA synchronizes full-width rows, so avoid overlapping rows. */
        {coords->x1, coords->y1 + radius, coords->x2, coords->y2 - radius},
        {coords->x1 + radius, coords->y1, coords->x2 - radius, coords->y1 + radius - 1},
        {coords->x1 + radius, coords->y2 - radius + 1, coords->x2 - radius, coords->y2},
    };
    /* Complete hardware/cache work before touching overlapping cache lines. */
    for (unsigned i = 0; i < 3; ++i) {
#ifdef ESP_PLATFORM
        if (!fill_middle(task, &flat, &rects[i]))
#endif
            __real_lv_draw_sw_fill(task, &flat, &rects[i]);
    }
    for (unsigned y = 0; y < 2; ++y) {
        for (unsigned x = 0; x < 2; ++x) {
            lv_area_t corner = {
                x ? coords->x2 - radius + 1 : coords->x1,
                y ? coords->y2 - radius + 1 : coords->y1,
                x ? coords->x2 : coords->x1 + radius - 1,
                y ? coords->y2 : coords->y1 + radius - 1,
            };
            lv_draw_task_t part = *task;
            if (lv_area_intersect(&part.clip_area, &corner, &task->clip_area))
                __real_lv_draw_sw_fill(&part, dsc, coords);
        }
    }
}
