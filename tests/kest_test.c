#include "kest_test.h"

kest_test_entry *kest_test_list;

kest_context global_cxt;

int init_sd_card()   {return NO_ERROR;}
int kest_sd_mode_msc()  {return NO_ERROR;}
int kest_sd_mode_local(){return NO_ERROR;}
int kest_sd_toggle_msc(){return NO_ERROR;}

extern const kest_test_entry __start_m_tests[];
extern const kest_test_entry __stop_m_tests[];

static lv_color_t dummy_buf[32];

static void dummy_flush(lv_display_t *disp,
                        const lv_area_t *area,
                        uint8_t *px_map)
{
    lv_display_flush_ready(disp);
}

int main(void)
{
    printf("Running tests...\n\n");

    int count = 0;

	lv_init();
	
	lv_display_t *disp = lv_display_create(1, 1);
	lv_display_set_buffers(
		disp,
		dummy_buf,
		NULL,
		sizeof(dummy_buf),
		LV_DISPLAY_RENDER_MODE_PARTIAL
	);
	lv_display_set_flush_cb(disp, dummy_flush);

    for (kest_test_entry *t = kest_test_list; t; t = t->next)
    {
        printf("  %s\n", t->name);
        t->fn();
        count++;
    }

    printf("\n%d tests passed\n", count);

    return 0;
}


void vApplicationMallocFailedHook(void)
{
    abort();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("Stack overflow in %s\n", pcTaskName);
    abort();
}
void vApplicationTickHook(void)
{
    lv_tick_inc(1);
}
