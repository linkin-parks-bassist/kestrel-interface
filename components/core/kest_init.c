#include "kest_int.h"

#define PRINTLINES_ALLOWED 1

static const char *FNAME = "kest_init.c";

int kest_init()
{
	int ret_val = NO_ERROR;
	
	kest_mem_init();
	
	init_representation_updater();
	kest_init_context(&global_cxt);
	kest_init_global_pages(&global_cxt.pages);
	
	kest_init_fpga_comms();
	
	xTaskCreate(kest_param_update_task, NULL, 4096, NULL, 8, NULL);
	
	kest_init_directories();
	
	//if (load_state_from_file(&global_cxt.state, SETTINGS_FNAME) == ERR_FOPEN_FAIL)
	//		save_state_to_file(&global_cxt.state, SETTINGS_FNAME);
	
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
	
	kest_active_preset_updater_start();
	kest_update_task_start();
	kest_init_file_task();
	
	return ret_val;
}
