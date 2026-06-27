#include "kest_int.h"

#define PRINTLINES_ALLOWED 1

static const char *FNAME = "kest_init.c";

int kest_init()
{
	int ret_val = NO_ERROR;
	
	kest_mem_init();
	
	kest_update_task_start();
	
	kest_ui_lock();
	kest_init_context(&global_cxt);
	kest_init_global_pages(&global_cxt.pages);
	kest_ui_unlock();
	
	#ifdef USE_FPGA
	kest_init_fpga_comms();
	kest_init_parameter_updater();
	#endif
	
	#ifdef USE_SDCARD
	init_sd_card();
	#endif
	
	#ifdef USE_SGTL5000
	xTaskCreate(kest_sgtl5000_init, "kest_sgtl5000_init_task", 8192, NULL, 8, NULL);
	#endif
	
	kest_init_directories();
	
	load_effects(&global_cxt);
	init_effect_selector_eff(&global_cxt.pages.effect_selector);
	load_saved_presets(&global_cxt);
	load_saved_sequences(&global_cxt);
	
	kest_create_ui_async();
	
	kest_state state;
	ret_val = load_state_from_file(&state, SETTINGS_FNAME);
	
	if (ret_val == NO_ERROR)
	{
		ret_val = kest_cxt_restore_state(&global_cxt, &state);
		kest_cxt_enter_previous_current_page(&global_cxt, &state);
		
		KEST_PRINTF("Restored state from disk with error code \"%s\"\n", kest_error_code_to_string(ret_val));
	}
	else
	{
		KEST_PRINTF("Unable to restore state from disk: \"%s\"\n", kest_error_code_to_string(ret_val));
	}
	
	kest_init_file_task();
	
	return ret_val;
}
