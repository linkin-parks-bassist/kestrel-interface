#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

static const char *FNAME = "kest_int.c";

#include "bsp/esp32_p4_nano.h"
#include "esp_task_wdt.h"

#include "kest_int.h"

kest_context global_cxt;


void app_main()
{
	int ret_val;
	
	kest_printf_init();
	
	srand(time(0));
	
	esp_task_wdt_deinit();
	
	kest_mem_init();
	
	#ifdef USE_DISPLAY
	lv_disp_t *disp;
	waveshare_dsi_touch_5_a_init(&disp);
	#endif
	
	kest_init_context(&global_cxt);
	
	kest_init_global_pages(&global_cxt.pages);
	
	#ifdef USE_SGTL5000
	xTaskCreate(kest_sgtl5000_init, "kest_sgtl5000_init_task", 8192, NULL, 8, NULL);
	#endif
	
	#ifdef USE_FPGA
	xTaskCreate(kest_fpga_comms_task,   "kest_fpga_comms_task",   4096, NULL, 8, NULL);
	xTaskCreate(kest_param_update_task, "kest_param_update_task", 4096, NULL, 8, NULL);
	#endif
	#ifdef USE_SDCARD
	kest_printf("DOING THE SD CARD STUFF\n");
	init_sd_card();
	kest_init_directories();
	load_effects(&global_cxt);
	init_effect_selector_eff(&global_cxt.pages.effect_selector);
	load_saved_presets(&global_cxt);
	
	context_print_presets(&global_cxt);
	load_saved_sequences(&global_cxt);
	kest_state state;
	ret_val = load_state_from_file(&state, SETTINGS_FNAME);
	
	if (ret_val == NO_ERROR)
	{
		ret_val = kest_cxt_restore_state(&global_cxt, &state);
		
		kest_printf("Restored state from disk with error code \"%s\"\n", kest_error_code_to_string(ret_val));
	}
	else
	{
		kest_printf("Unable to restore state from disk: \"%s\"\n", kest_error_code_to_string(ret_val));
	}
	#endif
	
	init_representation_updater();
	
	#ifdef KEST_SIMULATED
    while (1)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
	#else
	#ifdef USE_DISPLAY
	if (bsp_display_lock(0))
	{
		kest_log_init();
		lv_log_register_print_cb(kest_lv_log_cb);
		kest_create_ui(disp);
		kest_cxt_enter_previous_current_page(&global_cxt, &state);
		#ifdef KEST_PRINT_MEMORY_REPORT
		lv_timer_create(print_memory_report, 2000, NULL);
		#endif
		bsp_display_unlock();
	}
	#endif
	#endif
	
	//init_footswitch_task();
	
	while (1);
}
