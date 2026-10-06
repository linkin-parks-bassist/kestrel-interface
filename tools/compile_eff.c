#include "kest_lib.h"
#include <limits.h>

static void print_info_text(const char *field, size_t index, const char *text)
{
    printf("KEST eff-info field=%s index=%u hex=", field, (unsigned)index);
    for (const unsigned char *p = (const unsigned char *)text; *p; p++) printf("%02x", *p);
    putchar('\n');
}

static void print_info(const kest_effect_desc *desc)
{
    print_info_text("name", 0, desc->name);
    print_info_text("cname", 0, desc->cname);
    if (desc->description) print_info_text("description", 0, desc->description);
    const char *fields[] = { "keywords", "instruments", "types", "genres" };
    const string_list *lists[] = { &desc->keywords, &desc->instruments, &desc->types, &desc->genres };
    for (size_t i = 0; i < sizeof(lists) / sizeof(*lists); i++)
    {
        printf("KEST eff-info field=%s count=%u\n", fields[i], (unsigned)lists[i]->count);
        for (size_t j = 0; j < lists[i]->count; j++)
            print_info_text(fields[i], j, lists[i]->entries[j]);
    }
    puts("KEST eff-info result=0");
}

static int write_batch(const char *path, kest_fpga_transfer_batch batch)
{
    FILE *file = fopen(path, "wb");
    if (!file) { perror(path); return 0; }
    int written = fwrite(batch.buf, 1, batch.len, file) == (size_t)batch.len;
    if (fclose(file) != 0) written = 0;
    return written;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s INPUT.eff OUTPUT.bin [--update UPDATE.bin] [PARAMETER=VALUE ...] [setting.NAME=INTEGER ...]\n       %s --info INPUT.eff\n", argv[0], argv[0]);
        return 2;
    }
    int info = strcmp(argv[1], "--info") == 0;
    if (info && argc != 3) return 2;
    const char *input = argv[info ? 2 : 1];
    int result = kest_mem_init();
    if (result != NO_ERROR) goto failed;
    kest_effect_desc *desc = kest_read_eff_desc_from_file(input);
    if (!desc)
    {
        fprintf(stderr, "Could not parse %s\n", input);
        return 1;
    }
    if (info)
    {
        print_info(desc);
        kest_effect_desc_retire(desc);
        return ferror(stdout) ? 1 : 0;
    }
    kest_effect effect;
    result = init_effect_from_effect_desc(&effect, desc);
    if (result != NO_ERROR) goto failed;
    int live = argc > 3 && strcmp(argv[3], "--update") == 0;
    if (live && (argc < 6 || strcmp(argv[2], argv[4]) == 0)) return 2;
    kest_effect_pll node = { .data = &effect, .next = NULL };
    kest_pipeline pipeline = { .effects = &node };
    kest_fpga_transfer_batch batch = {0};
    kest_updater_state state;
    if (live)
    {
        result = kest_pipeline_create_fpga_transfer_batch(&pipeline, &batch);
        if (result != NO_ERROR) goto failed;
        if (!write_batch(argv[2], batch)) { kest_free_fpga_transfer_batch(batch); return 1; }
        kest_free_fpga_transfer_batch(batch);
        result = kest_updater_state_init(&state);
        if (result != NO_ERROR) goto failed;
    }
    for (int i = live ? 5 : 3; i < argc; ++i)
    {
        char *equals = strchr(argv[i], '=');
        char *end;
        if (!equals || equals == argv[i]) return 2;
        float value = strtof(equals + 1, &end);
        if (end == equals + 1 || *end || !isfinite(value)) return 2;
        *equals = '\0';
        if (strncmp(argv[i], "setting.", 8) == 0)
        {
            /* Settings change configuration, never a parameter-only live batch. */
            if (live) return 2;
            kest_setting_pll *s = effect.settings;
            while (s && strcmp(s->data->name_internal, argv[i] + 8) != 0) s = s->next;
            if (!s || value != truncf(value) || (double)value < INT_MIN || (double)value > INT_MAX ||
                (double)value < s->data->min || (double)value > s->data->max)
                return 2;
            if (s->data->type == EFFECT_SETTING_ENUM)
            {
                int found = 0;
                for (int j = 0; j < s->data->n_options; j++)
                    if (s->data->options[j].value == value) found = 1;
                if (!found) return 2;
            }
            s->data->value = (int)value;
            continue;
        }
        result = kest_effect_set_parameter(&effect, argv[i], value);
        if (result != NO_ERROR) goto failed;
        if (live)
        {
            kest_parameter_pll *param = effect.parameters;
            while (param && strcmp(param->data->name_internal, argv[i]) != 0) param = param->next;
            if (!param) { result = ERR_BAD_ARGS; goto failed; }
            kest_update update = { .type = KEST_UPDATE_PARAM, .data.param = param->data };
            result = kest_updater_handle_update(&state, update);
            if (result != NO_ERROR) goto failed;
        }
    }
    if (live)
    {
        result = kest_updater_generate_command_list(&state);
        if (result == NO_ERROR) result = kest_updater_generate_tx_batch(&state);
        if (result != NO_ERROR) { kest_updater_state_destroy(&state); goto failed; }
        int written = write_batch(argv[4], state.batch);
        kest_updater_state_destroy(&state);
        return written ? 0 : 1;
    }
    result = kest_pipeline_create_fpga_transfer_batch(&pipeline, &batch);
    if (result != NO_ERROR) goto failed;

    int written = write_batch(argv[2], batch);
    kest_free_fpga_transfer_batch(batch);
    if (!written)
    {
        fprintf(stderr, "Could not write %s\n", argv[2]);
        return 1;
    }
    return 0;

failed:
    fprintf(stderr, "Compilation failed: %s\n", kest_error_code_to_string(result));
    return 1;
}
