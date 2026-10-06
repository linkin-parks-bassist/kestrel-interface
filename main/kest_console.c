#include "kest_int.h"
#include "kest_console.h"
#include "esp_console.h"
#include "esp_app_desc.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_timer.h"
#include <errno.h>
#include <inttypes.h>
#include <dirent.h>
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h>

static int info_command(int argc, char **argv)
{
    const esp_app_desc_t *app = esp_app_get_description();
    printf("KEST info project=%s version=%s idf=%s tick_hz=%d tasks=%u\n",
           app->project_name, app->version, esp_get_idf_version(),
           CONFIG_FREERTOS_HZ, (unsigned)uxTaskGetNumberOfTasks());
    return 0;
}

static int heap_command(int argc, char **argv)
{
    printf("KEST heap free=%u minimum=%u largest=%u internal_free=%u\n",
           (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
           (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
           (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
           (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    return 0;
}

static int pools_command(int argc, char **argv)
{
    if (argc != 1) return 1;
#define PRINT_POOL(X) do { \
    X##_pool *pool = &X##_mem_pool; \
    if (!pool->mutex || X##_pool_lock(pool) != NO_ERROR) return 1; \
    size_t size = pool->size, available = pool->free_count; \
    X##_pool_unlock(pool); \
    printf("KEST pool name=" #X " size=%u free=%u used=%u\n", \
           (unsigned)size, (unsigned)available, (unsigned)(size - available)); \
} while (0)
    PRINT_POOL(kest_dsp_resource);
    PRINT_POOL(kest_effect_desc);
    PRINT_POOL(kest_expression);
    PRINT_POOL(kest_parameter);
    PRINT_POOL(kest_setting);
    PRINT_POOL(kest_effect);
    PRINT_POOL(kest_preset);
    PRINT_POOL(kest_sequence);
#undef PRINT_POOL
    puts("KEST pools end");
    return 0;
}

static int uptime_command(int argc, char **argv)
{
    printf("KEST uptime ms=%" PRId64 "\n", esp_timer_get_time() / 1000);
    return 0;
}

static lv_indev_t *console_pointer;
static lv_point_t pointer_position;
static bool pointer_pressed;
static int64_t release_at;

static void pointer_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (release_at < 0) release_at = esp_timer_get_time() + 80000;
    if (release_at > 0 && esp_timer_get_time() >= release_at)
    {
        pointer_pressed = false;
        release_at = 0;
    }
    data->point = pointer_position;
    data->state = pointer_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static bool coordinate(const char *text, int32_t limit, int32_t *result)
{
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno || !*text || *end || value < 0 || value >= limit) return false;
    *result = value;
    return true;
}

static int pointer_command(int argc, char **argv)
{
    bool tap = strcmp(argv[0], "tap") == 0;
    bool up = !tap && argc == 2 && strcmp(argv[1], "up") == 0;
    bool down = tap || (!tap && argc == 4 && strcmp(argv[1], "down") == 0);
    bool move = !tap && argc == 4 && strcmp(argv[1], "move") == 0;
    if ((!tap && !up && !down && !move) || (tap && argc != 3)) return 1;
    if (!kest_ui_lock()) return 1;
    lv_display_t *display = lv_display_get_default();
    int32_t x = 0, y = 0;
    int offset = tap ? 1 : 2;
    if (!display || (!up &&
        (!coordinate(argv[offset], lv_display_get_horizontal_resolution(display), &x) ||
         !coordinate(argv[offset + 1], lv_display_get_vertical_resolution(display), &y))))
    {
        kest_ui_unlock();
        puts("KEST error invalid touch coordinates");
        return 1;
    }
    if (!console_pointer)
    {
        console_pointer = lv_indev_create();
        if (!console_pointer) { kest_ui_unlock(); return 1; }
        lv_indev_set_type(console_pointer, LV_INDEV_TYPE_POINTER);
        lv_indev_set_display(console_pointer, display);
        lv_indev_set_read_cb(console_pointer, pointer_read);
    }
    if ((tap && pointer_pressed) || (move && !pointer_pressed))
    {
        kest_ui_unlock();
        puts("KEST error incompatible touch state");
        return 1;
    }
    if (!up) { pointer_position.x = x; pointer_position.y = y; }
    pointer_pressed = !up;
    release_at = tap ? -1 : 0;
    kest_ui_unlock();
    puts("KEST touch queued");
    return 0;
}

static void print_object(lv_obj_t *obj, unsigned depth)
{
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) return;
    lv_area_t area;
    lv_obj_get_coords(obj, &area);
    const char *text = lv_obj_check_type(obj, &lv_label_class) ? lv_label_get_text(obj) : "";
    printf("KEST obj depth=%u x=%" PRId32 " y=%" PRId32 " w=%" PRId32 " h=%" PRId32 " clickable=%d text=%s\n",
           depth, area.x1, area.y1, lv_obj_get_width(obj), lv_obj_get_height(obj),
           lv_obj_has_flag(obj, LV_OBJ_FLAG_CLICKABLE), text);
    for (uint32_t i = 0; i < lv_obj_get_child_count(obj); i++)
        print_object(lv_obj_get_child(obj, i), depth + 1);
}

static int tree_command(int argc, char **argv)
{
    if (argc != 1 || !kest_ui_lock()) return 1;
    lv_obj_t *screen = lv_screen_active();
    if (screen)
    {
        print_object(screen, 0);
        print_object(lv_layer_top(), 0);
        print_object(lv_layer_sys(), 0);
    }
    kest_ui_unlock();
    puts("KEST tree end");
    return screen ? 0 : 1;
}

/* Opt-in counters: no UART output in display callbacks. */
static struct {
    bool enabled;
    int64_t render_start, flush_start, wait_start;
    uint64_t render_us, flush_us, wait_us, pixels;
    uint32_t frames, flushes;
} ui_profile;

static void ui_profile_event(lv_event_t *event)
{
    if (!ui_profile.enabled) return;
    int64_t now = esp_timer_get_time();
    switch (lv_event_get_code(event))
    {
        case LV_EVENT_RENDER_START: ui_profile.render_start = now; break;
        case LV_EVENT_RENDER_READY:
            ui_profile.render_us += now - ui_profile.render_start;
            ui_profile.frames++;
            break;
        case LV_EVENT_FLUSH_START: {
            const lv_area_t *area = lv_event_get_param(event);
            ui_profile.flush_start = now;
            ui_profile.pixels += lv_area_get_size(area);
            ui_profile.flushes++;
            break;
        }
        case LV_EVENT_FLUSH_FINISH:
            ui_profile.flush_us += now - ui_profile.flush_start;
            break;
        case LV_EVENT_FLUSH_WAIT_START: ui_profile.wait_start = now; break;
        case LV_EVENT_FLUSH_WAIT_FINISH:
            ui_profile.wait_us += now - ui_profile.wait_start;
            break;
        default: break;
    }
}

static int profile_command(int argc, char **argv)
{
#ifdef KEST_DRAW_PROFILE_PREVIEW
    extern void kest_draw_profile_start(void);
    extern void kest_draw_profile_stop(void);
    extern void kest_draw_profile_report(void);
#endif
    static bool attached;
    if (argc != 2 || (strcmp(argv[1], "start") && strcmp(argv[1], "stop"))) return 1;
    if (!kest_ui_lock()) return 1;
    if (!attached)
    {
        lv_display_add_event_cb(lv_display_get_default(), ui_profile_event, LV_EVENT_ALL, NULL);
        attached = true;
    }
    if (!strcmp(argv[1], "start"))
    {
        memset(&ui_profile, 0, sizeof(ui_profile));
        ui_profile.enabled = true;
#ifdef KEST_DRAW_PROFILE_PREVIEW
        kest_draw_profile_start();
#endif
        kest_ui_unlock();
        puts("KEST ui-profile started");
        return 0;
    }
    ui_profile.enabled = false;
#ifdef KEST_DRAW_PROFILE_PREVIEW
    kest_draw_profile_stop();
#endif
    uint32_t frames = ui_profile.frames, flushes = ui_profile.flushes;
    uint64_t render = ui_profile.render_us, flush = ui_profile.flush_us;
    uint64_t wait = ui_profile.wait_us, pixels = ui_profile.pixels;
    kest_ui_unlock();
#ifdef KEST_DRAW_PROFILE_PREVIEW
    kest_draw_profile_report();
#endif
    printf("KEST ui-profile frames=%" PRIu32 " flushes=%" PRIu32
           " render_us=%" PRIu64 " flush_us=%" PRIu64
           " wait_us=%" PRIu64 " pixels=%" PRIu64 "\n",
           frames, flushes, render, flush, wait, pixels);
    return 0;
}

static int read_complete(kest_fpga_read_spec *read)
{
    if (read->type == KEST_FPGA_READ32)
    {
        unsigned addr = ((unsigned)read->addr[0] << 16) | ((unsigned)read->addr[1] << 8) | read->addr[2];
        if (read->result < 0) printf("KEST fpga-read32 addr=0x%06x error=%" PRId64 "\n", addr, read->result);
        else printf("KEST fpga-read32 addr=0x%06x value=0x%08" PRIx32 "\n", addr, (uint32_t)read->result);
        free(read);
        return NO_ERROR;
    }
    unsigned addr = ((unsigned)read->addr[0] << 8) | read->addr[1];
    printf("KEST fpga-read addr=%u raw=%" PRId64 "\n", addr, read->result);
    free(read);
    return NO_ERROR;
}

static int fpga_read_command(int argc, char **argv)
{
    if (argc != 2) return 1;
    char *end;
    errno = 0;
    unsigned long addr = strtoul(argv[1], &end, 0);
    bool word_read = strcmp(argv[0], "fpga-read32") == 0;
    if (errno || !*argv[1] || *end || addr > (word_read ? 0xffffff : UINT16_MAX) ||
        (word_read && (addr & 3))) return 1;
    kest_fpga_read_spec *read = calloc(1, sizeof(*read));
    if (!read) return 1;
    read->type = word_read ? KEST_FPGA_READ32 : DATA_REQ_MEM;
    read->addr_size = word_read ? 3 : 2;
    read->ret_size = word_read ? 4 : 2;
    for (int i = 0; i < read->addr_size; i++)
        read->addr[i] = addr >> (8 * (read->addr_size - i - 1));
    read->callback = read_complete;
    int rc = kest_fpga_queue_read(read);
    if (rc != NO_ERROR)
    {
        free(read);
        printf("KEST error fpga-read queue=%d\n", rc);
        return 1;
    }
    // SPI owns the specification until read_complete releases it.
    printf("KEST %s queued addr=%lu\n", argv[0], addr);
    return 0;
}

static bool effect_path(const char *name, char *path, size_t size)
{
    size_t len = strlen(name);
    if (len <= 4 || len > 12 || strchr(name, '.') != name + len - 4 ||
        strchr(name, '/') || strchr(name, '\\') ||
        strcasecmp(name + len - 4, ".eff") != 0) return false;
    return snprintf(path, size, "%s%s", KEST_EFFECT_DESC_DIR, name) < size;
}

typedef struct {
    kest_preset *preset;
    kest_pipeline staged;
    kest_effect_pll *previous;
} kest_reload_preset;

static bool pipeline_uses_descriptor(const kest_pipeline *pipeline, const kest_effect_desc *descriptor)
{
    for (kest_effect_pll *node = pipeline ? pipeline->effects : NULL; node; node = node->next)
        if (node->data && node->data->eff == descriptor) return true;
    return false;
}

static void reload_rebind_preset(kest_preset *preset)
{
    if (!preset || !preset->view_page || !preset->view_page->data_struct) return;
    kest_preset_view_str *view = preset->view_page->data_struct;
    kest_effect_pll *node = preset->pipeline.effects;
    for (int i = 0; view->array && i < view->array->n_buttons && node; i++, node = node->next)
    {
        view->array->buttons[i]->data = node->data;
        kest_active_button_change_label(view->array->buttons[i], (char *)kest_effect_name(node->data));
    }
}

static void reload_rebind_selector(kest_effect_desc *previous, kest_effect_desc *replacement)
{
    kest_effect_selector_str *selector = global_cxt.pages.effect_selector.data_struct;
    for (kest_effect_selector_button_pll *node = selector ? selector->buttons : NULL; node; node = node->next)
        if (node->data && node->data->eff == previous)
        {
            node->data->eff = replacement;
            node->data->name = (char *)replacement->name;
            if (node->data->label) lv_label_set_text(node->data->label, replacement->name);
        }
}

static int effect_reload_locked(const char *path, unsigned *affected_out)
{
    kest_effect_desc *replacement = kest_read_eff_desc_from_file((char *)path);
    if (!replacement) return ERR_BAD_ARGS;

    kest_effect_desc_pll *descriptor_node = global_cxt.effects;
    while (descriptor_node && (!descriptor_node->data || !descriptor_node->data->cname ||
           strcmp(descriptor_node->data->cname, replacement->cname) != 0))
        descriptor_node = descriptor_node->next;
    if (!descriptor_node)
    {
        kest_effect_desc_retire(replacement);
        return ERR_NOT_FOUND;
    }
    kest_effect_desc *previous = descriptor_node->data;

    kest_reload_preset *staged = kest_alloc(sizeof(*staged) * MAX_N_PRESETS);
    if (!staged)
    {
        kest_effect_desc_retire(replacement);
        return ERR_ALLOC_FAIL;
    }
    memset(staged, 0, sizeof(*staged) * MAX_N_PRESETS);
    unsigned affected = 0;
    kest_ui_page *replacement_page = NULL;
    kest_fpga_transfer_batch active_batch = {0};
    bool have_active_batch = false;
    int result = NO_ERROR;

    for (kest_preset_pll *node = global_cxt.presets; node; node = node->next)
    {
        kest_preset *preset = node->data;
        if (!preset || !pipeline_uses_descriptor(&preset->pipeline, previous)) continue;
        if (affected == MAX_N_PRESETS) { result = ERR_CURRENTLY_EXHAUSTED; goto reject; }
        staged[affected].preset = preset;
        if ((result = kest_pipeline_stage_reload(&staged[affected].staged, &preset->pipeline,
                                                  previous, replacement)) != NO_ERROR)
            goto reject;
        kest_effect_pll *old_node = preset->pipeline.effects;
        kest_effect_pll *new_node = staged[affected].staged.effects;
        while (new_node)
        {
            if (preset->view_page)
            {
                if ((result = kest_effect_init_view_page(new_node->data, preset->view_page)) != NO_ERROR ||
                    (preset->view_page->ui_created &&
                     (result = create_page_ui(new_node->data->view_page)) != NO_ERROR))
                    goto reject;
            }
            for (kest_ui_page *page = global_cxt.pages.current_page; old_node && page; page = page->parent)
                if (page == old_node->data->view_page)
                {
                    replacement_page = new_node->data->view_page;
                    break;
                }
            old_node = old_node ? old_node->next : NULL;
            new_node = new_node->next;
        }
        if (preset == global_cxt.active_preset)
        {
            result = kest_pipeline_create_fpga_transfer_batch(&staged[affected].staged, &active_batch);
            if (result != NO_ERROR) goto reject;
            have_active_batch = true;
        }
        affected++;
    }

    descriptor_node->data = replacement;
    for (unsigned i = 0; i < affected; i++)
    {
        kest_parameter_cancel_preset_updates(staged[i].preset->id);
        staged[i].previous = staged[i].preset->pipeline.effects;
        staged[i].preset->pipeline.effects = staged[i].staged.effects;
        staged[i].staged.effects = NULL;
        reload_rebind_preset(staged[i].preset);
    }
    reload_rebind_selector(previous, replacement);

    if (have_active_batch)
    {
        kest_free_fpga_transfer_batch(active_batch);
        active_batch.buf = NULL;
    }
    if (have_active_batch && (result = kest_updater_notify_preset(global_cxt.active_preset)) != NO_ERROR)
    {
        reload_rebind_selector(replacement, previous);
        descriptor_node->data = previous;
        for (unsigned i = 0; i < affected; i++)
        {
            staged[i].staged.effects = staged[i].preset->pipeline.effects;
            staged[i].preset->pipeline.effects = staged[i].previous;
            reload_rebind_preset(staged[i].preset);
        }
        goto reject;
    }

    if (replacement_page) enter_ui_page(replacement_page);
    for (unsigned i = 0; i < affected; i++)
    {
        kest_pipeline retired = { .effects = staged[i].previous };
        gut_pipeline(&retired);
        staged[i].preset->unsaved_changes = 1;
        kest_queue_preset_save(staged[i].preset);
    }
    kest_effect_desc_retire(previous);
    kest_free(staged);
    *affected_out = affected;
    return NO_ERROR;

reject:
    if (active_batch.buf) kest_free_fpga_transfer_batch(active_batch);
    for (unsigned i = 0; i < affected + 1 && i < MAX_N_PRESETS; i++)
        kest_pipeline_discard_staged(&staged[i].staged);
    kest_effect_desc_retire(replacement);
    kest_free(staged);
    return result;
}

static int effect_reload_command(int argc, char **argv)
{
    char path[256];
    if (argc != 2 || !effect_path(argv[1], path, sizeof(path))) return 1;
    if (!sd_mutex || sd_msc_mode || xSemaphoreTake(sd_mutex, 0) != pdTRUE)
    {
        puts("KEST error SD unavailable or owned by USB");
        return 1;
    }
    if (!kest_ui_lock()) { xSemaphoreGive(sd_mutex); return 1; }
    unsigned affected = 0;
    int result = effect_reload_locked(path, &affected);
    kest_eff_parser_reset_mempool();
    kest_ui_unlock();
    xSemaphoreGive(sd_mutex);
    printf("KEST eff-reload result=%d affected=%u\n", result, affected);
    return result == NO_ERROR ? 0 : 1;
}

static void print_effect_info_text(const char *field, size_t index, const char *text)
{
    printf("KEST eff-info field=%s index=%u hex=", field, (unsigned)index);
    for (const unsigned char *p = (const unsigned char *)text; *p; p++) printf("%02x", *p);
    putchar('\n');
}

static int effect_info_command(int argc, char **argv)
{
    if (argc != 2 || !kest_ui_lock()) return 1;
    kest_effect_desc *desc = NULL;
    for (kest_effect_desc_pll *node = global_cxt.effects; node; node = node->next)
        if (node->data && node->data->cname && strcmp(node->data->cname, argv[1]) == 0)
        {
            desc = node->data;
            kest_effect_desc_retain(desc);
            break;
        }
    kest_ui_unlock();
    if (!desc)
    {
        printf("KEST eff-info result=%d\n", ERR_NOT_FOUND);
        return 1;
    }
    // Immutable metadata is protected by the borrow; UART output must not hold rendering.
    print_effect_info_text("name", 0, desc->name);
    print_effect_info_text("cname", 0, desc->cname);
    if (desc->description) print_effect_info_text("description", 0, desc->description);
    const char *fields[] = { "keywords", "instruments", "types", "genres" };
    const string_list *lists[] = { &desc->keywords, &desc->instruments, &desc->types, &desc->genres };
    for (size_t i = 0; i < sizeof(lists) / sizeof(*lists); i++)
    {
        printf("KEST eff-info field=%s count=%u\n", fields[i], (unsigned)lists[i]->count);
        for (size_t j = 0; j < lists[i]->count; j++)
            print_effect_info_text(fields[i], j, lists[i]->entries[j]);
    }
    if (!kest_ui_lock()) return 1;
    kest_effect_desc_release(desc);
    kest_ui_unlock();
    puts("KEST eff-info result=0");
    return 0;
}

static int hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int effect_file_command(int argc, char **argv)
{
    char path[256], dest[256];
    bool list = argc == 2 && strcmp(argv[1], "list") == 0;
    bool read = argc == 3 && strcmp(argv[1], "read") == 0;
    bool remove = argc == 3 && strcmp(argv[1], "delete") == 0;
    bool publish = argc == 3 && strcmp(argv[1], "publish") == 0;
    bool move = argc == 4 && strcmp(argv[1], "move") == 0;
    bool write = argc == 5 && strcmp(argv[1], "write") == 0;
    if (!(list || read || remove || publish || move || write)) return 1;
    if (!list && !effect_path(argv[2], path, sizeof(path))) return 1;
    if (move && !effect_path(argv[3], dest, sizeof(dest))) return 1;
    unsigned char bytes[128];
    size_t count = 0;
    long offset = 0;
    if (write)
    {
        char *end;
        errno = 0;
        offset = strtol(argv[3], &end, 10);
        size_t len = strlen(argv[4]);
        if (errno || !*argv[3] || *end || offset < 0 || !len || len % 2 || len > sizeof(bytes) * 2) return 1;
        count = len / 2;
        for (size_t i = 0; i < count; i++)
        {
            int hi = hex_digit(argv[4][i * 2]), lo = hex_digit(argv[4][i * 2 + 1]);
            if (hi < 0 || lo < 0) return 1;
            bytes[i] = (hi << 4) | lo;
        }
    }
    if (!sd_mutex || sd_msc_mode || xSemaphoreTake(sd_mutex, 0) != pdTRUE)
    {
        puts("KEST error SD unavailable or owned by USB");
        return 1;
    }
    errno = 0;
    int result = -1;
    if (list)
    {
        DIR *dir = opendir(KEST_EFFECT_DESC_DIR);
        if (dir)
        {
            struct dirent *entry;
            while ((entry = readdir(dir)))
                if (effect_path(entry->d_name, path, sizeof(path)))
                    printf("KEST eff-file name=%s\n", entry->d_name);
            result = closedir(dir);
        }
    }
    else if (read)
    {
        FILE *file = fopen(path, "rb");
        if (file)
        {
            size_t n;
            while ((n = fread(bytes, 1, sizeof(bytes), file)))
            {
                printf("KEST eff-file data=");
                for (size_t i = 0; i < n; i++) printf("%02x", bytes[i]);
                putchar('\n');
            }
            result = ferror(file) ? -1 : 0;
            if (fclose(file) != 0) result = -1;
        }
    }
    else if (remove) result = unlink(path);
    else if (move) result = rename(path, dest);
    else
    {
        snprintf(dest, sizeof(dest), KEST_EFFECT_DESC_DIR "tmp/%s", argv[2]);
        strcpy(dest + strlen(dest) - 4, ".tmp");
        if (publish) result = rename(dest, path);
        else
        {
            if (mkdir(KEST_EFFECT_DESC_DIR "tmp", 0777) != 0 && errno != EEXIST)
                goto done;
            errno = 0;
            FILE *file = fopen(dest, offset == 0 ? "wb" : "r+b");
            if (file)
            {
                if (fseek(file, 0, SEEK_END) == 0)
                {
                    long size = ftell(file);
                    if (size == offset) result = fwrite(bytes, 1, count, file) == count ? 0 : -1;
                    else if (size >= 0) errno = EINVAL;
                }
                if (fclose(file) != 0) result = -1;
            }
        }
    }
done:
    ;
    int saved_errno = errno;
    xSemaphoreGive(sd_mutex);
    printf("KEST eff-file result=%d errno=%d\n", result, result == 0 ? 0 : saved_errno);
    return result == 0 ? 0 : 1;
}

static int dsp_command(int argc, char **argv)
{
    if (argc != 1 || !kest_ui_lock()) return 1;
    kest_preset *preset = global_cxt.active_preset;
    printf("KEST dsp preset=%d\n", preset ? preset->id : -1);
    for (kest_effect_pll *entry = preset ? preset->pipeline.effects : NULL;
         entry; entry = entry->next)
    {
        kest_effect *effect = entry->data;
        if (!effect) continue;
        printf("KEST dsp effect=%d name=%s blocks=%zu mem_start=%d\n",
               effect->id, kest_effect_name(effect), effect->blocks.count, effect->position_.mem_start);
        for (kest_parameter_pll *node = effect->parameters; node; node = node->next)
        {
            kest_parameter *param = node->data;
            if (!param) continue;
            kest_interval range = kest_parameter_get_range(param);
            printf("KEST dsp parameter=%u.%u.%u name=%s value=%.9g min=%.9g max=%.9g driven=%d override=%d\n",
                   param->id.preset_id, param->id.effect_id, param->id.parameter_id,
                   param->name_internal ? param->name_internal : "",
                   (double)atomic_load_explicit(&param->value, memory_order_relaxed),
                   (double)range.a, (double)range.b,
                   param->driver_index != KEST_PARAMETER_UNDRIVEN, param->driver_override);
        }
        for (kest_setting_pll *node = effect->settings; node; node = node->next)
        {
            kest_setting *setting = node->data;
            if (!setting) continue;
            const char *choice = "";
            for (int i = 0; i < setting->n_options; i++)
                if (setting->options[i].value == setting->value)
                    choice = setting->options[i].name;
            printf("KEST dsp setting=%u.%u.%u name=%s value=%d min=%d max=%d choice=%s\n",
                   setting->id.preset_id, setting->id.effect_id, setting->id.setting_id,
                   setting->name_internal ? setting->name_internal : "",
                   setting->value, setting->min, setting->max, choice);
        }
        for (size_t i = 0; i < effect->resources.count; i++)
        {
            kest_dsp_resource *res = effect->resources.entries[i];
            if (!res) continue;
            printf("KEST dsp resource=%s type=%s handle=%d",
                   res->name ? res->name : "", kest_dsp_resource_type_to_string(res->type), res->handle);
            if (res->type == KEST_DSP_RESOURCE_MEM && res->data)
            {
                kest_mem_slot *mem = res->data;
                printf(" address=%d read_enable=%d read_ms=%d latest=%d",
                       effect->position_.mem_start + res->handle, mem->read_enable,
                       mem->read_period_ms, atomic_load_explicit(&mem->value, memory_order_relaxed));
            }
            putchar('\n');
        }
    }
    kest_ui_unlock();
    return 0;
}

static int parameter_target_command(int argc, char **argv)
{
    int32_t ids[3];
    if (argc != 5)
    {
        puts("KEST parameter-target reason=invalid-input");
        return 1;
    }
    for (int i = 0; i < 3; i++)
        if (!coordinate(argv[i + 1], 65536, &ids[i]))
        {
            puts("KEST parameter-target reason=invalid-input");
            return 1;
        }
    char *end;
    errno = 0;
    float target = strtof(argv[4], &end);
    if (errno || !*argv[4] || *end || !isfinite(target))
    {
        puts("KEST parameter-target reason=invalid-input");
        return 1;
    }
    if (!kest_ui_lock()) return 1;
    kest_parameter *param = cxt_get_parameter_by_id(&global_cxt, ids[0], ids[1], ids[2]);
    int result = ERR_NOT_FOUND;
    const char *reason = "not-found";
    if (param)
    {
        kest_interval range = kest_parameter_get_range(param);
        if (!isfinite(range.a) || !isfinite(range.b) || range.a > range.b ||
            target < range.a || target > range.b)
        {
            result = ERR_BAD_ARGS;
            reason = "out-of-range";
        }
        else if (param->driver_index != KEST_PARAMETER_UNDRIVEN && !param->driver_override)
        {
            result = ERR_OVERRULED;
            reason = "driven";
        }
        else
        {
            result = kest_parameter_trigger_update(param, target);
            reason = result == NO_ERROR ? "queued" : "queue-failed";
            if (result == NO_ERROR && param->effect && param->effect->preset)
                param->effect->preset->unsaved_changes = 1;
        }
    }
    kest_ui_unlock();
    printf("KEST parameter-target id=%ld.%ld.%ld target=%.9g result=%d reason=%s\n",
           (long)ids[0], (long)ids[1], (long)ids[2], (double)target, result, reason);
    return result == NO_ERROR ? 0 : 1;
}

static int presets_command(int argc, char **argv)
{
    if (argc != 1 || !kest_ui_lock()) return 1;
    unsigned count = 0;
    for (kest_preset_pll *node = global_cxt.presets; node; node = node->next)
    {
        kest_preset *preset = node->data;
        printf("KEST preset id=%u active=%d name=%s file=%s\n", preset->id,
               preset == global_cxt.active_preset, preset->name ? preset->name : "",
               preset->has_fname ? preset->fname : "");
        count++;
    }
    kest_ui_unlock();
    printf("KEST presets count=%u end\n", count);
    return 0;
}

static void print_sequence(kest_sequence *sequence, unsigned index)
{
    printf("KEST sequence index=%u main=%d selected=%d active=%d name=%s file=%s\n",
           index, sequence->main_sequence, sequence == global_cxt.sequence,
           sequence->active, sequence->name ? sequence->name : "",
           sequence->has_fname ? sequence->fname : "");
    unsigned position = 0;
    for (seq_kest_preset_pll *node = sequence->presets; node; node = node->next)
    {
        kest_preset *preset = node->data;
        printf("KEST sequence member index=%u position=%u preset=%d current=%d active=%d\n",
               index, position++, preset ? (int)preset->id : -1,
               node == sequence->position, preset && preset == global_cxt.active_preset);
    }
    printf("KEST sequence index=%u members=%u end\n", index, position);
}

static int sequences_command(int argc, char **argv)
{
    if (argc != 1 || !kest_ui_lock()) return 1;
    unsigned count = 1;
    print_sequence(&global_cxt.main_sequence, 0);
    for (kest_sequence_pll *node = global_cxt.sequences; node; node = node->next)
        print_sequence(node->data, count++);
    kest_ui_unlock();
    printf("KEST sequences count=%u end\n", count);
    return 0;
}

static int sequence_step_command(int argc, char **argv)
{
    int step = argc == 2 ? (strcmp(argv[1], "next") == 0 ? 1 :
                           strcmp(argv[1], "prev") == 0 ? -1 : 0) : 0;
    if (!step || !kest_ui_lock()) return 1;
    int result = step > 0 ? kest_sequence_advance(global_cxt.sequence) :
                            kest_sequence_regress(global_cxt.sequence);
    int preset = global_cxt.active_preset ? (int)global_cxt.active_preset->id : -1;
    kest_ui_unlock();
    printf("KEST sequence-step result=%d preset=%d\n", result, preset);
    return result == NO_ERROR ? 0 : 1;
}

static void status_complete(int result, uint8_t raw)
{
    if (result != NO_ERROR)
    {
        printf("KEST fpga-status SPI error=%d\n", result);
        return;
    }
    kest_fpga_status_flags flags;
    kest_fpga_decode_status_flags(&flags, raw);
    printf("KEST fpga-status raw=0x%02x initialized=%d listening=%d timeout=%d programming=%d bad=%d data_ready=%d cmd_err=%d swapping=%d\n",
           raw, flags.initialised, flags.listening, flags.timeout, flags.programming,
           flags.bad, flags.data_ready, flags.cmd_err, flags.swapping);
}

static int fpga_status_command(int argc, char **argv)
{
    if (argc != 1) return 1;
    int rc = kest_fpga_queue_status(status_complete);
    if (rc != NO_ERROR) printf("KEST error fpga-status queue=%d\n", rc);
    else printf("KEST fpga-status queued\n");
    return rc == NO_ERROR ? 0 : 1;
}

esp_err_t kest_console_start(void)
{
#ifdef KEST_ROUNDED_FILL_PREVIEW
    extern int kest_rounded_fill_check_command(int, char **);
#endif
    esp_console_dev_uart_config_t device = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    esp_console_repl_config_t config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    config.prompt = "kest> ";
    config.max_cmdline_length = 256;
    config.max_history_len = 8;
    config.task_stack_size = 16384; // Descriptor parsing and formatted rejection diagnostics.
    esp_console_repl_t *repl = NULL;
    esp_err_t rc = esp_console_new_repl_uart(&device, &config, &repl);
    if (rc != ESP_OK) return rc;
    const esp_console_cmd_t commands[] = {
#ifdef KEST_ROUNDED_FILL_PREVIEW
        { .command = "rounded-fill-check", .help = "Isolated PPA pixel comparison; optional alternate-fill rejection", .hint = "[reject]", .func = kest_rounded_fill_check_command },
#endif
        { .command = "info", .help = "Firmware, tick rate and task count", .func = info_command },
        { .command = "heap", .help = "Free, minimum and largest heap blocks", .func = heap_command },
        { .command = "pools", .help = "Reserved typed pool capacity, free and used slots", .func = pools_command },
        { .command = "uptime", .help = "Milliseconds since boot", .func = uptime_command },
        { .command = "tap", .help = "Pointer press and automatic release after 80 ms", .hint = "<x> <y>", .func = pointer_command },
        { .command = "touch", .help = "Pointer input for taps, holds, slider moves and scrolling", .hint = "down <x> <y> | move <x> <y> | up", .func = pointer_command },
        { .command = "ui-tree", .help = "Visible UI objects, bounds and labels", .func = tree_command },
        { .command = "ui-profile", .help = "Opt-in redraw/flush timing counters; stop prints outside UI lock", .hint = "start | stop", .func = profile_command },
        { .command = "eff-file", .help = "SD effects: list/read/write/publish/move/delete; 8.3 names, writes stage .tmp files", .hint = "list | read/delete/publish <name.eff> | write <name.eff> <offset> <hex> | move <old.eff> <new.eff>", .func = effect_file_command },
        { .command = "eff-info", .help = "Loaded effect names and optional discovery metadata as hex text", .hint = "<cname>", .func = effect_info_command },
        { .command = "eff-reload", .help = "Parse one SD effect and replace the loaded descriptor and every matching instance", .hint = "<name.eff>", .func = effect_reload_command },
        { .command = "dsp", .help = "Active preset effects, parameter IDs/values/ranges and resource addresses", .func = dsp_command },
        { .command = "parameter-target", .help = "Queue an effect parameter target through normal smoothing; finite in-range undriven/overridden values", .hint = "PRESET_ID EFFECT_ID PARAMETER_ID VALUE", .func = parameter_target_command },
        { .command = "presets", .help = "Loaded preset IDs, names, filenames and active selection", .func = presets_command },
        { .command = "sequences", .help = "Main and loaded sequences, ordered preset IDs and current position", .func = sequences_command },
        { .command = "sequence-step", .help = "Move the active sequence through its normal navigation functions", .hint = "next | prev", .func = sequence_step_command },
        { .command = "fpga-status", .help = "Read FPGA status through the SPI task", .func = fpga_status_command },
        { .command = "fpga-read", .help = "Queue one FPGA memory read; result arrives asynchronously", .hint = "<address>", .func = fpga_read_command },
        { .command = "fpga-read32", .help = "Read a word-aligned 24-bit FPGA address (0: magic, 4: build flags)", .hint = "<address>", .func = fpga_read_command },
    };
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        rc = esp_console_cmd_register(&commands[i]);
        if (rc != ESP_OK) { repl->del(repl); return rc; }
    }
    return esp_console_start_repl(repl);
}
