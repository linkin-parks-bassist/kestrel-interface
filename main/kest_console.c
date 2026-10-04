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
    esp_console_dev_uart_config_t device = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    esp_console_repl_config_t config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    config.prompt = "kest> ";
    config.max_cmdline_length = 256;
    config.max_history_len = 8;
    esp_console_repl_t *repl = NULL;
    esp_err_t rc = esp_console_new_repl_uart(&device, &config, &repl);
    if (rc != ESP_OK) return rc;
    const esp_console_cmd_t commands[] = {
        { .command = "info", .help = "Firmware, tick rate and task count", .func = info_command },
        { .command = "heap", .help = "Free, minimum and largest heap blocks", .func = heap_command },
        { .command = "uptime", .help = "Milliseconds since boot", .func = uptime_command },
        { .command = "tap", .help = "Pointer press and automatic release after 80 ms", .hint = "<x> <y>", .func = pointer_command },
        { .command = "touch", .help = "Pointer input for taps, holds, slider moves and scrolling", .hint = "down <x> <y> | move <x> <y> | up", .func = pointer_command },
        { .command = "ui-tree", .help = "Visible UI objects, bounds and labels", .func = tree_command },
        { .command = "eff-file", .help = "SD effects: list/read/write/publish/move/delete; 8.3 names, writes stage .tmp files", .hint = "list | read/delete/publish <name.eff> | write <name.eff> <offset> <hex> | move <old.eff> <new.eff>", .func = effect_file_command },
        { .command = "dsp", .help = "Active preset DSP effects and resource addresses", .func = dsp_command },
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
