#include "kest_int.h"

#define PRINTLINES_ALLOWED 0
#include <SDL2/SDL.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

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

/* Opt-in stdin controls run on the UI task, never on a second LVGL thread. */
static int controlled;
static int pointer_x, pointer_y, pointer_down;
static uint64_t control_resume_at, pointer_release_at;
static char control_input[8192];
static size_t control_input_len;
static char screenshot_path[4096];

static void dump_ui(lv_obj_t *obj, const char *path)
{
    if (!obj || lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) return;
    lv_area_t area;
    lv_obj_get_coords(obj, &area);
    const char *text = lv_obj_check_type(obj, &lv_label_class)
        ? lv_label_get_text(obj) : "";
    printf("UI %s [%d,%d,%d,%d] clickable=%d text=", path,
           (int)area.x1, (int)area.y1, (int)area.x2, (int)area.y2,
           lv_obj_has_flag(obj, LV_OBJ_FLAG_CLICKABLE));
    /* Keep each object on one line even for multiline labels. */
    for (const char *p = text; *p; ++p)
        putchar(*p == '\n' || *p == '\r' || *p == '\t' ? ' ' : *p);
    putchar('\n');
    for (uint32_t i = 0; i < lv_obj_get_child_count(obj); ++i) {
        char child_path[512];
        snprintf(child_path, sizeof(child_path), "%s/%u", path, i);
        dump_ui(lv_obj_get_child(obj, i), child_path);
    }
}

static void desktop_control(int *running)
{
    if (!controlled) return;
    uint64_t now = SDL_GetTicks64();
    if (pointer_release_at && now >= pointer_release_at) {
        pointer_down = 0;
        pointer_release_at = 0;
    }
    if (control_input_len < sizeof(control_input) - 1) {
        ssize_t n = read(STDIN_FILENO, control_input + control_input_len,
                         sizeof(control_input) - 1 - control_input_len);
        if (n > 0) control_input_len += (size_t)n;
        else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            perror("desktop control input");
            *running = 0;
        }
    }
    if (now < control_resume_at || screenshot_path[0]) return;
    control_input[control_input_len] = 0;
    char *newline = strchr(control_input, '\n');
    if (!newline) {
        if (control_input_len == sizeof(control_input) - 1) {
            fprintf(stderr, "Desktop control line too long\n");
            *running = 0;
        }
        return;
    }
    *newline = 0;
    char *line = control_input;
    int x, y, duration;
    if (!line[0] || line[0] == '#') {}
    else if (sscanf(line, "wait %d", &duration) == 1 && duration >= 0)
        control_resume_at = now + (uint64_t)duration;
    else if (sscanf(line, "click %d %d", &x, &y) == 2) {
        if (x < 0 || y < 0 || x >= DISPLAY_HRES || y >= DISPLAY_VRES)
            fprintf(stderr, "Click outside display: %d %d\n", x, y);
        else {
            pointer_x = x; pointer_y = y; pointer_down = 1;
            pointer_release_at = now + 80;
            control_resume_at = now + 180;
        }
    }
    else if (sscanf(line, "touch down %d %d", &x, &y) == 2 ||
             sscanf(line, "touch move %d %d", &x, &y) == 2) {
        if (x < 0 || y < 0 || x >= DISPLAY_HRES || y >= DISPLAY_VRES)
            fprintf(stderr, "Touch outside display: %d %d\n", x, y);
        else {
            pointer_x = x; pointer_y = y; pointer_down = 1;
            pointer_release_at = 0;
            control_resume_at = now + 30;
        }
    }
    else if (strcmp(line, "touch up") == 0) {
        pointer_down = 0;
        pointer_release_at = 0;
        control_resume_at = now + 30;
    }
    else if (strncmp(line, "screenshot ", 11) == 0 && line[11]) {
        if (strlen(line + 11) >= sizeof(screenshot_path))
            fprintf(stderr, "Screenshot path too long\n");
        else strcpy(screenshot_path, line + 11);
    }
    else if (strcmp(line, "tree") == 0) {
        kest_ui_lock();
        lv_obj_update_layout(lv_screen_active());
        dump_ui(lv_screen_active(), "screen");
        dump_ui(lv_layer_top(), "top");
        kest_ui_unlock();
        puts("UI END");
    }
    else if (strcmp(line, "quit") == 0) *running = 0;
    else fprintf(stderr, "Unknown desktop control command: %s\n", line);
    size_t consumed = (size_t)(newline - control_input) + 1;
    memmove(control_input, control_input + consumed, control_input_len - consumed);
    control_input_len -= consumed;
    fflush(stdout);
}

static void desktop_capture(void)
{
    if (!screenshot_path[0]) return;
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0,
        DISPLAY_HRES, DISPLAY_VRES, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!surface || SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888,
                          surface->pixels, surface->pitch) != 0 ||
        SDL_SaveBMP(surface, screenshot_path) != 0)
        fprintf(stderr, "Screenshot failed: %s\n", SDL_GetError());
    else printf("SCREENSHOT %s\n", screenshot_path);
    SDL_FreeSurface(surface);
    screenshot_path[0] = 0;
    fflush(stdout);
}


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
    if (controlled) {
        data->point.x = pointer_x;
        data->point.y = pointer_y;
        data->state = pointer_down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        return;
    }
    int x, y;
    uint32_t buttons = SDL_GetMouseState(&x, &y);

    data->point.x = x;
    data->point.y = y;

    data->state = (buttons & SDL_BUTTON_LMASK)
        ? LV_INDEV_STATE_PRESSED
        : LV_INDEV_STATE_RELEASED;
}

int kest_desktop_init_sdl()
{
	setenv("SDL_VIDEODRIVER", "x11", 0); // Force X11; Wayland compat broke after an update =/
    if (SDL_Init(SDL_INIT_VIDEO) != 0) goto fail;

    window = SDL_CreateWindow("Kestrel",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              DISPLAY_HRES, DISPLAY_VRES,
                              0);

    if (!window) goto fail;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) goto fail;
    texture = SDL_CreateTexture(renderer,
                                 SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 DISPLAY_HRES, DISPLAY_VRES);

    if (!texture) goto fail;
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
	lv_init();
    disp = lv_display_create(DISPLAY_HRES, DISPLAY_VRES);

    static uint32_t buf[DISPLAY_HRES * 40];
    lv_display_set_buffers(disp,
                           buf,
                           NULL,
                           sizeof(buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_display_set_flush_cb(disp, flush_cb);
    
    lv_indev_t *mouse = lv_indev_create();
	lv_indev_set_type(mouse, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(mouse, mouse_read);
	
	return NO_ERROR;
fail:
    fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
    return -1;
}

void main_task(void *arg)
{
	int ret_val;
	
	srand(time(0));
	
	kest_printf_init();
	if (kest_desktop_init_sdl() != NO_ERROR) {
        vTaskEndScheduler();
        return;
    }
	
	kest_event_task_start();
	
	kest_event startup_event;
	startup_event.type = KEST_EVENT_STARTUP;
	
	kest_event_log(startup_event);
	
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
		
		desktop_control(&running);
		lv_timer_handler();
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		desktop_capture();
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
    if (argc == 2 && strcmp(argv[1], "--control") == 0) {
        controlled = 1;
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        if (flags < 0 || fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) < 0) {
            perror("desktop control");
            return 1;
        }
        setvbuf(stdout, NULL, _IOLBF, 0);
    } else if (argc != 1) {
        fprintf(stderr, "Usage: %s [--control]\n", argv[0]);
        return 1;
    }
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
