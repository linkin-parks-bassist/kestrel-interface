/* Opt-in carrier diagnostic; preserve the corner renderer's exact draw calls. */
#define KEST_ROUNDED_FILL_ENTRY kest_profile_fill_backend
#include "rounded_corners_preview.c"
#include "esp_timer.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static bool enabled;
static struct { uint64_t us; uint32_t calls; } counters[6];

void kest_draw_profile_start(void)
{
    memset(counters, 0, sizeof(counters));
    enabled = true;
}

void kest_draw_profile_stop(void)
{
    enabled = false;
}

void kest_draw_profile_report(void)
{
    const char *names[] = {"fill", "border", "label", "arc", "image", "shadow"};
    for (unsigned i = 0; i < 6; ++i)
        printf("KEST draw-profile %s calls=%" PRIu32 " us=%" PRIu64 "\n",
               names[i], counters[i].calls, counters[i].us);
}

#define PROFILE_DRAW(NAME, TYPE, INDEX, BACKEND) \
void __real_lv_draw_sw_##NAME(lv_draw_task_t *, TYPE *, const lv_area_t *); \
void __wrap_lv_draw_sw_##NAME(lv_draw_task_t *task, TYPE *dsc, const lv_area_t *coords) \
{ \
    if (!enabled) { BACKEND(task, dsc, coords); return; } \
    int64_t start = esp_timer_get_time(); \
    BACKEND(task, dsc, coords); \
    counters[INDEX].us += esp_timer_get_time() - start; \
    ++counters[INDEX].calls; \
}

PROFILE_DRAW(fill, lv_draw_fill_dsc_t, 0, kest_profile_fill_backend)
PROFILE_DRAW(border, const lv_draw_border_dsc_t, 1, __real_lv_draw_sw_border)
PROFILE_DRAW(label, const lv_draw_label_dsc_t, 2, __real_lv_draw_sw_label)
PROFILE_DRAW(arc, const lv_draw_arc_dsc_t, 3, __real_lv_draw_sw_arc)
PROFILE_DRAW(image, const lv_draw_image_dsc_t, 4, __real_lv_draw_sw_image)
PROFILE_DRAW(box_shadow, const lv_draw_box_shadow_dsc_t, 5, __real_lv_draw_sw_box_shadow)
