/* Desktop-only pixel-equivalence experiment for a future accelerated interior.
 * Desktop keeps the middle software; the opt-in carrier build tries PPA. */
#include "lvgl.h"
#include "src/draw/lv_draw_private.h"
#include "src/draw/sw/lv_draw_sw.h"
#include "src/misc/lv_area_private.h"

#ifdef ESP_PLATFORM
#include "driver/ppa.h"
uint32_t kest_rounded_fill_hw_calls, kest_rounded_fill_hw_errors;

static bool fill_middle(lv_draw_task_t *task, const lv_draw_fill_dsc_t *dsc,
                        const lv_area_t *middle)
{
    static ppa_client_handle_t client;
    lv_layer_t *layer = task->target_layer;
    lv_draw_buf_t *buf = layer->draw_buf;
    if (layer->color_format != LV_COLOR_FORMAT_RGB565 ||
        buf->header.stride != buf->header.w * 2) return false;
    lv_area_t area;
    if (!lv_area_intersect(&area, middle, &task->clip_area)) return true;
    if (!client) {
        ppa_client_config_t cfg = {
            .oper_type = PPA_OPERATION_FILL,
            .max_pending_trans_num = 1,
            .data_burst_length = PPA_DATA_BURST_LENGTH_128,
        };
        if (ppa_register_client(&cfg, &client) != ESP_OK) {
            ++kest_rounded_fill_hw_errors;
            return false;
        }
    }
    lv_area_move(&area, -layer->buf_area.x1, -layer->buf_area.y1);
    ppa_fill_oper_config_t cfg = {0};
    cfg.fill_argb_color.val = lv_color_to_u32(dsc->color);
    cfg.out.block_offset_x = area.x1;
    cfg.out.block_offset_y = area.y1;
    cfg.out.fill_cm = PPA_FILL_COLOR_MODE_RGB565;
    cfg.fill_block_w = lv_area_get_width(&area);
    cfg.fill_block_h = lv_area_get_height(&area);
    cfg.out.buffer = buf->data;
    cfg.out.buffer_size = buf->data_size;
    cfg.out.pic_w = buf->header.w;
    cfg.out.pic_h = buf->header.h;
    cfg.mode = PPA_TRANS_MODE_BLOCKING;
#ifndef KEST_ROUNDED_DRIVER_CACHE_PREVIEW
    lv_draw_buf_flush_cache(buf, &area);
#endif
    esp_err_t result = ppa_do_fill(client, &cfg);
    /* Invalidate before software edges: whole cache lines can overlap them. */
#ifndef KEST_ROUNDED_DRIVER_CACHE_PREVIEW
    lv_draw_buf_invalidate_cache(buf, &area);
#endif
    if (result == ESP_OK) ++kest_rounded_fill_hw_calls;
    else ++kest_rounded_fill_hw_errors;
    return result == ESP_OK;
}
#endif

void __real_lv_draw_sw_fill(lv_draw_task_t *, lv_draw_fill_dsc_t *, const lv_area_t *);

void __wrap_lv_draw_sw_fill(lv_draw_task_t *task, lv_draw_fill_dsc_t *dsc,
                          const lv_area_t *coords)
{
    int32_t radius = LV_MIN(dsc->radius,
                           LV_MIN(lv_area_get_width(coords), lv_area_get_height(coords)) / 2);
    if (radius <= 0 || dsc->opa < LV_OPA_MAX || dsc->grad.dir != LV_GRAD_DIR_NONE) {
        __real_lv_draw_sw_fill(task, dsc, coords);
        return;
    }
    lv_area_t middle = *coords;
    middle.y1 += radius;
    middle.y2 -= radius;
    if (middle.y1 > middle.y2) {
        __real_lv_draw_sw_fill(task, dsc, coords);
        return;
    }
    lv_draw_fill_dsc_t flat = *dsc;
    flat.radius = 0;
    flat.opa = LV_OPA_COVER;
#ifdef ESP_PLATFORM
    if (!fill_middle(task, &flat, &middle))
#endif
        __real_lv_draw_sw_fill(task, &flat, &middle);
    lv_draw_task_t part = *task;
    lv_area_t strip = *coords;
    strip.y2 = middle.y1 - 1;
    if (lv_area_intersect(&part.clip_area, &strip, &task->clip_area))
        __real_lv_draw_sw_fill(&part, dsc, coords);
    strip = *coords;
    strip.y1 = middle.y2 + 1;
    if (lv_area_intersect(&part.clip_area, &strip, &task->clip_area))
        __real_lv_draw_sw_fill(&part, dsc, coords);
}
