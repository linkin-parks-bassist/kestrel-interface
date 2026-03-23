#include "kest_int.h"

#define PRINTLINES_ALLOWED 0
#include <SDL2/SDL.h>
#include <time.h>

static const char *FNAME = "kest_desktop.c";

int init_sd_card()   	{return NO_ERROR;}
int kest_sd_mode_msc()  {return NO_ERROR;}
int kest_sd_mode_local(){return NO_ERROR;}
int kest_sd_toggle_msc(){return NO_ERROR;}

kest_context global_cxt;

static lv_display_t *disp;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

static void flush_cb(lv_display_t *d,
                     const lv_area_t *area,
                     uint8_t *px_map)
{
    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);

    SDL_UpdateTexture(texture,
                      &(SDL_Rect){area->x1, area->y1, w, h},
                      px_map,
                      w * 4);

    lv_display_flush_ready(d);
}
static void mouse_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    int x, y;
    uint32_t buttons = SDL_GetMouseState(&x, &y);

    data->point.x = x;
    data->point.y = y;

    data->state = (buttons & SDL_BUTTON_LMASK)
        ? LV_INDEV_STATE_PRESSED
        : LV_INDEV_STATE_RELEASED;
}

void main_task(void *arg)
{
	int ret_val;
	
	srand(time(0));
	
	kest_printf_init();
	
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("M",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              DISPLAY_HRES, DISPLAY_VRES,
                              0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer,
                                 SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 DISPLAY_HRES, DISPLAY_VRES);

	lv_init();
    disp = lv_display_create(DISPLAY_HRES, DISPLAY_VRES);

    static lv_color_t buf[DISPLAY_HRES * 40];
    lv_display_set_buffers(disp,
                           buf,
                           NULL,
                           sizeof(buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_display_set_flush_cb(disp, flush_cb);
    
    lv_indev_t *mouse = lv_indev_create();
	lv_indev_set_type(mouse, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(mouse, mouse_read);
    
	init_representation_updater();
	kest_init_context(&global_cxt);
	kest_context_init_effect_list(&global_cxt);
	kest_init_global_pages(&global_cxt.pages);
	
	xTaskCreate(
		kest_fpga_comms_task,
		NULL,
		4096,
		NULL,
		8,
		NULL
	);
	xTaskCreate(kest_param_update_task, NULL, 4096, NULL, 8, NULL);
	
	kest_init_directories();
	
	//if (load_state_from_file(&global_cxt.state, SETTINGS_FNAME) == ERR_FOPEN_FAIL)
	//		save_state_to_file(&global_cxt.state, SETTINGS_FNAME);
	
	load_effects(&global_cxt);
	init_effect_selector_eff(&global_cxt.pages.effect_selector);
	load_saved_presets(&global_cxt);
	load_saved_sequences(&global_cxt);
	
	kest_create_ui(NULL);
	
	kest_state state;
	ret_val = load_state_from_file(&state, SETTINGS_FNAME);
	
	if (ret_val == NO_ERROR)
	{
		ret_val = kest_cxt_restore_state(&global_cxt, &state);
		
		KEST_PRINTF("Restored state from disk with error code \"%s\"\n", kest_error_code_to_string(ret_val));
	}
	else
	{
		KEST_PRINTF("Unable to restore state from disk: \"%s\"\n", kest_error_code_to_string(ret_val));
	}
	
	int running = 1;
	SDL_Event e;
	
	while (running)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				running = 0;
			}
		}
		
		lv_timer_handler();
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_Delay(1);
	}
	
	SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
	
	vTaskEndScheduler();
	vTaskDelete(NULL);
}

int main(int argc, char **argv)
{	
	xTaskCreate(main_task,
		NULL,
		64 * 1024,
		NULL,
		8,
		NULL);

    vTaskStartScheduler();
    
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
