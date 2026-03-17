#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

static const char *FNAME = "m_int.c";

#include "bsp/esp32_p4_nano.h"
#include "esp_task_wdt.h"

#include "m_int.h"

m_context global_cxt;


void app_main()
{
	int ret_val;
	
	m_printf_init();
	
	srand(time(0));
	
	esp_task_wdt_deinit();
	
	m_mem_init();
	
	#ifdef USE_DISPLAY
	lv_disp_t *disp;
	waveshare_dsi_touch_5_a_init(&disp);
	#endif
	
	m_init_context(&global_cxt);
	m_context_init_effect_list(&global_cxt);
	
	m_init_global_pages(&global_cxt.pages);
	
	#ifdef USE_SGTL5000
	xTaskCreate(m_sgtl5000_init, "m_sgtl5000_init_task", 8192, NULL, 8, NULL);
	#endif
	
	#ifdef USE_FPGA
	xTaskCreate(m_fpga_comms_task,   "m_fpga_comms_task",   4096, NULL, 8, NULL);
	xTaskCreate(m_param_update_task, "m_param_update_task", 4096, NULL, 8, NULL);
	#else
	init_m_msg_queue();
	begin_m_comms();
	#endif
	#ifdef USE_SDCARD
	m_printf("DOING THE SD CARD STUFF\n");
	init_sd_card();
	m_init_directories();
	load_effects(&global_cxt);
	init_effect_selector_eff(&global_cxt.pages.effect_selector);
	load_saved_profiles(&global_cxt);
	
	context_print_profiles(&global_cxt);
	load_saved_sequences(&global_cxt);
	m_state state;
	ret_val = load_state_from_file(&state, SETTINGS_FNAME);
	
	if (ret_val == NO_ERROR)
	{
		ret_val = m_cxt_restore_state(&global_cxt, &state);
		
		m_printf("Restored state from disk with error code \"%s\"\n", m_error_code_to_string(ret_val));
	}
	else
	{
		m_printf("Unable to restore state from disk: \"%s\"\n", m_error_code_to_string(ret_val));
	}
	#endif
	
	init_representation_updater();
	#ifdef M_SIMULATED
    while (1)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
	#else
	#ifdef USE_DISPLAY
	if (bsp_display_lock(0))
	{
		m_log_init();
		lv_log_register_print_cb(m_lv_log_cb);
		m_create_ui(disp);
		m_cxt_enter_previous_current_page(&global_cxt, &state);
		#ifdef M_PRINT_MEMORY_REPORT
		lv_timer_create(print_memory_report, 2000, NULL);
		#endif
		bsp_display_unlock();
	}
	#endif
	#endif
	
	//init_footswitch_task();
	
	while (1);
}
