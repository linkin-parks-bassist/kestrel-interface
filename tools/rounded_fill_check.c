/* Compare the experimental split with LVGL's original fill, including clipping.
 * This is a pixel oracle, not a hardware-performance measurement. */
#include "lvgl.h"
#include "src/draw/lv_draw_private.h"
#include "src/draw/sw/lv_draw_sw.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef ESP_PLATFORM
#include "FreeRTOS.h"
#include "task.h"
void vApplicationMallocFailedHook(void) { abort(); }
void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    abort();
}
void vApplicationTickHook(void) { lv_tick_inc(1); }
#else
#include "esp_lvgl_port.h"
#include "driver/ppa.h"
extern uint32_t kest_rounded_fill_hw_calls, kest_rounded_fill_hw_errors;

/* Reject alternate submissions only during the locked pixel oracle. This tests
 * fallback after successful neighbouring fills, not partial DMA or timeouts. */
static bool reject_fills;
static uint32_t attempts, rejections;
esp_err_t __real_ppa_do_fill(ppa_client_handle_t, const ppa_fill_oper_config_t *);
esp_err_t __wrap_ppa_do_fill(ppa_client_handle_t client, const ppa_fill_oper_config_t *cfg)
{
    if (reject_fills && ++attempts % 2 == 0) {
        ++rejections;
        return ESP_ERR_INVALID_ARG;
    }
    return __real_ppa_do_fill(client, cfg);
}
#endif

void __real_lv_draw_sw_fill(lv_draw_task_t *, lv_draw_fill_dsc_t *, const lv_area_t *);
void __wrap_lv_draw_sw_fill(lv_draw_task_t *, lv_draw_fill_dsc_t *, const lv_area_t *);

static unsigned seed = 31;
static unsigned next(void)
{
    seed = seed * 1664525u + 1013904223u;
    return seed;
}

static int check_fills(unsigned count)
{
    seed = 31;
    const lv_color_format_t formats[] = {LV_COLOR_FORMAT_RGB565, LV_COLOR_FORMAT_XRGB8888};
    unsigned cases = 0;
    for (unsigned wide = 0; wide < 2; ++wide) {
        unsigned width = wide ? 640 : 80, height = 80;
        unsigned iterations = wide ? (count < 128 ? count : 128) : count;
        for (unsigned f = 0; f < 2; ++f) {
            lv_draw_buf_t *a = lv_draw_buf_create(width, height, formats[f], 0);
            lv_draw_buf_t *b = lv_draw_buf_create(width, height, formats[f], 0);
            if (!a || !b) {
                if (a) lv_draw_buf_destroy(a);
                if (b) lv_draw_buf_destroy(b);
                return 2;
            }
            for (unsigned i = 0; i < iterations; ++i) {
                lv_layer_t layer = {0};
                layer.color_format = formats[f];
                layer.buf_area = (lv_area_t){-9, 13, width - 10, height + 12};
                lv_draw_task_t task = {0};
                task.target_layer = &layer;
                int x = -20 + next() % 70, y = next() % 70;
                lv_area_t coords = {x, y, x + next() % width, y + next() % 80};
                task.clip_area = layer.buf_area;
                if (i % 3) {
                    task.clip_area.x1 += next() % 40;
                    task.clip_area.y1 += next() % 40;
                    task.clip_area.x2 -= next() % 40;
                    task.clip_area.y2 -= next() % 40;
                }
                lv_draw_fill_dsc_t dsc;
                lv_draw_fill_dsc_init(&dsc);
                dsc.color = lv_color_hex(next() & 0xffffff);
                dsc.radius = i % 7 ? next() % 90 : LV_RADIUS_CIRCLE;
                dsc.opa = i % 4 ? LV_OPA_COVER : next() % 256;
                if (i % 11 == 0) {
                    dsc.grad.dir = LV_GRAD_DIR_VER;
                    dsc.grad.stops_count = 2;
                    dsc.grad.stops[0].color = dsc.color;
                    dsc.grad.stops[0].frac = 0;
                    dsc.grad.stops[0].opa = LV_OPA_COVER;
                    dsc.grad.stops[1].color = lv_color_hex(0xabcdef);
                    dsc.grad.stops[1].frac = 255;
                    dsc.grad.stops[1].opa = LV_OPA_COVER;
                }
                for (unsigned j = 0; j < a->data_size; ++j) a->data[j] = next() >> 24;
                memcpy(b->data, a->data, a->data_size);
                layer.draw_buf = a;
                __real_lv_draw_sw_fill(&task, &dsc, &coords);
                layer.draw_buf = b;
                __wrap_lv_draw_sw_fill(&task, &dsc, &coords);
                if (memcmp(a->data, b->data, a->data_size)) {
                    fprintf(stderr, "Mismatch: format=%d case=%u radius=%d opacity=%u\n",
                            formats[f], i, (int)dsc.radius, dsc.opa);
                    fprintf(stderr, "coords=(%d,%d)-(%d,%d) clip=(%d,%d)-(%d,%d)\n",
                            coords.x1, coords.y1, coords.x2, coords.y2,
                            task.clip_area.x1, task.clip_area.y1, task.clip_area.x2, task.clip_area.y2);
                    for (unsigned j = 0; j < a->data_size; ++j) {
                        if (a->data[j] != b->data[j]) {
                            fprintf(stderr, "first differing byte=%u stride=%u original=%u candidate=%u\n",
                                    j, (unsigned)a->header.stride, a->data[j], b->data[j]);
                            break;
                        }
                    }
                    lv_draw_buf_destroy(a);
                    lv_draw_buf_destroy(b);
                    return 1;
                }
                ++cases;
            }
            lv_draw_buf_destroy(a);
            lv_draw_buf_destroy(b);
        }
    }
    printf("PASS: %u pixel comparisons; RGB565/XRGB8888, clipped/offset/tiny/oversized-radius and fallback fills\n", cases);
    return 0;
}

#ifndef ESP_PLATFORM
int main(void)
{
    lv_init();
    int result = check_fills(10000);
    lv_deinit();
    return result;
}
#else
int kest_rounded_fill_check_command(int argc, char **argv)
{
    bool inject = argc == 2 && !strcmp(argv[1], "reject");
    if ((argc != 1 && !inject) || !lvgl_port_lock(1000)) return 1;
    attempts = rejections = 0;
    reject_fills = inject;
    uint32_t calls = kest_rounded_fill_hw_calls;
    uint32_t errors = kest_rounded_fill_hw_errors;
    int result = check_fills(128);
    reject_fills = false;
    calls = kest_rounded_fill_hw_calls - calls;
    errors = kest_rounded_fill_hw_errors - errors;
    lvgl_port_unlock();
    printf("KEST rounded-fill result=%d hardware_calls=%lu errors=%lu injected=%lu\n",
           result, (unsigned long)calls, (unsigned long)errors, (unsigned long)rejections);
    return result || !calls || errors != rejections || (inject && !rejections);
}
#endif
