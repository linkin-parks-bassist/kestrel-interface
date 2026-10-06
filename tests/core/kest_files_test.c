#include "kest_test.h"
#include <unistd.h>
#include <dirent.h>

static void temporary_save_file(char *path)
{
    int fd = mkstemp(path);
    assert(fd >= 0);
    assert(close(fd) == 0);
}

KEST_TEST(kest_test_save_preset_null)
{
    assert(save_preset(NULL) == ERR_NULL_PTR);
}

KEST_TEST(test_invalid_headers_preserve_populated_destinations)
{
    char path[] = "/tmp/kestXXXXXX";
    temporary_save_file(path);
    kest_effect effect = { .id = 37 };
    kest_effect_pll effect_node = { .data = &effect };
    kest_preset preset = { .name = "Existing", .has_fname = 1,
        .unsaved_changes = 1, .pipeline = { .effects = &effect_node } };
    seq_kest_preset_pll member = { .data = &preset };
    kest_sequence sequence = { .name = "Existing sequence", .presets = &member,
        .position = &member, .has_fname = 1, .unsaved_changes = 1 };
    preset.sequence = &sequence;
    effect.preset = &preset;
    strcpy(preset.fname, "KEEP.PRE");
    strcpy(sequence.fname, "KEEP.SEQ");
    kest_preset saved_preset = preset;
    kest_sequence saved_sequence = sequence;
    for (int kind = 0; kind < 2; kind++)
        for (int header = 0; header < 2; header++)
            for (int attempt = 0; attempt < 3; attempt++)
            {
                unsigned char bytes[] = { kind ? KEST_SEQUENCE_MAGIC_BYTE :
                    KEST_PRESET_MAGIC_BYTE, KEST_WRITE_FINISHED_BYTE, 'N', 0, 0, 0 };
                bytes[header] = header ? KEST_WRITE_UNFINISHED_BYTE : 0;
                FILE *file = fopen(path, "wb");
                assert(file && fwrite(bytes, 1, sizeof(bytes), file) == sizeof(bytes));
                assert(fclose(file) == 0);
                int rc = kind ? read_sequence_from_file(&sequence, path) :
                    read_preset_from_file(&preset, path);
                assert(rc == (header ? ERR_UNFINISHED_WRITE : ERR_BAD_ARGS));
                assert(memcmp(&preset, &saved_preset, sizeof(preset)) == 0);
                assert(memcmp(&sequence, &saved_sequence, sizeof(sequence)) == 0);
                assert(effect.preset == &preset && effect.id == 37);
                assert(effect_node.data == &effect && effect_node.next == NULL);
                assert(member.data == &preset && member.next == NULL && member.prev == NULL);
            }
    assert(unlink(path) == 0);
}

KEST_TEST(kest_test_empty_preset_save_load)
{
    char path[] = "/tmp/kestXXXXXX";
    temporary_save_file(path);
    char name[] = "Quiet preset";
    kest_preset original = { .name = name };

    for (int unnamed = 0; unnamed < 2; unnamed++)
    {
        original.name = unnamed ? NULL : name;
        kest_preset loaded = {0};
        assert(save_preset_as_file(&original, path) == NO_ERROR);
        assert(read_preset_from_file(&loaded, path) == NO_ERROR);
        assert(strcmp(loaded.name, unnamed ? "Unnamed Preset" : name) == 0);
        assert(loaded.pipeline.effects == NULL);
        assert(loaded.has_fname && !loaded.unsaved_changes);
        assert(strcmp(loaded.fname, path) == 0);
        kest_free(loaded.name);
    }
    assert(unlink(path) == 0);
}

KEST_TEST(kest_test_state_save_load_and_invalid_header)
{
    char path[] = "/tmp/kestXXXXXX";
    temporary_save_file(path);
    kest_state original = {
        .input_gain = -3.25f,
        .output_gain = 1.5f,
        .active_preset_fname = "0123456789012345678901234567890",
        .active_sequence_fname = "abcdefghijklmnopqrstuvwxyzabcde",
        .current_page = { .type = KEST_UI_PAGE_EFFECT_VIEW, .id = 42, .fname = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDE" }
    };
    kest_state loaded = {0};
    assert(save_state_to_file(&original, path) == NO_ERROR);
    assert(load_state_from_file(&loaded, path) == NO_ERROR);
    assert(loaded.input_gain == original.input_gain);
    assert(loaded.output_gain == original.output_gain);
    assert(strcmp(loaded.active_preset_fname, original.active_preset_fname) == 0);
    assert(strcmp(loaded.active_sequence_fname, original.active_sequence_fname) == 0);
    assert(loaded.current_page.type == original.current_page.type);
    assert(loaded.current_page.id == original.current_page.id);
    assert(strcmp(loaded.current_page.fname, original.current_page.fname) == 0);

    FILE *file = fopen(path, "rb");
    assert(file != NULL);
    unsigned char bytes[128];
    size_t length = fread(bytes, 1, sizeof(bytes), file);
    assert(length < sizeof(bytes) && feof(file));
    assert(fclose(file) == 0);
    for (size_t prefix = 0; prefix < length; prefix++)
    {
        file = fopen(path, "wb");
        assert(file != NULL);
        assert(fwrite(bytes, 1, prefix, file) == prefix);
        assert(fclose(file) == 0);
        kest_state rejected = original;
        assert(load_state_from_file(&rejected, path) == ERR_MANGLED_FILE);
        assert(memcmp(&rejected, &original, sizeof(original)) == 0);
    }

    // A filename must fit its destination, including the terminating NUL.
    size_t terminators[3];
    terminators[0] = 2 + 2 * sizeof(float) + strlen(original.active_preset_fname);
    terminators[1] = terminators[0] + 1 + strlen(original.active_sequence_fname);
    terminators[2] = terminators[1] + 1 + 2 * sizeof(int32_t) + strlen(original.current_page.fname);
    for (int field = 0; field < 3; field++)
    {
        file = fopen(path, "wb");
        assert(file != NULL);
        bytes[terminators[field]] = 'A';
        assert(fwrite(bytes, 1, length, file) == length);
        assert(fclose(file) == 0);
        bytes[terminators[field]] = 0;
        kest_state rejected = original;
        assert(load_state_from_file(&rejected, path) == ERR_MANGLED_FILE);
        assert(memcmp(&rejected, &original, sizeof(original)) == 0);
    }

    kest_state empty = original;
    empty.active_preset_fname[0] = 0;
    empty.active_sequence_fname[0] = 0;
    empty.current_page.fname[0] = 0;
    assert(save_state_to_file(&empty, path) == NO_ERROR);
    assert(load_state_from_file(&loaded, path) == NO_ERROR);
    assert(!loaded.active_preset_fname[0] && !loaded.active_sequence_fname[0]);
    assert(!loaded.current_page.fname[0]);
    assert(loaded.current_page.id == original.current_page.id);
    assert(loaded.input_gain == original.input_gain && loaded.output_gain == original.output_gain);

    for (int header_byte = 0; header_byte < 2; header_byte++)
    {
        assert(save_state_to_file(&original, path) == NO_ERROR);
        FILE *file = fopen(path, "r+b");
        assert(file != NULL);
        assert(fseek(file, header_byte, SEEK_SET) == 0);
        assert(fputc(header_byte ? KEST_WRITE_UNFINISHED_BYTE : 0, file) != EOF);
        assert(fclose(file) == 0);
        kest_state rejected = {0};
        assert(load_state_from_file(&rejected, path) == ERR_MANGLED_FILE);
        assert(rejected.input_gain == 0 && rejected.output_gain == 0);
        assert(rejected.current_page.type == 0 && rejected.current_page.id == 0);
        assert(!rejected.active_preset_fname[0] && !rejected.active_sequence_fname[0]);
    }
    assert(unlink(path) == 0);
}

KEST_TEST(test_preset_and_sequence_reject_truncated_records)
{
    char path[] = "/tmp/kestXXXXXX";
    temporary_save_file(path);
    for (int sequence_file = 0; sequence_file < 2; sequence_file++)
    {
        unsigned char record[] = {0, KEST_WRITE_FINISHED_BYTE, 'N', 0, 0, 0};
        record[0] = sequence_file ? KEST_SEQUENCE_MAGIC_BYTE : KEST_PRESET_MAGIC_BYTE;
        for (size_t length = 0; length <= sizeof(record); length++)
        {
            FILE *file = fopen(path, "wb");
            assert(file && fwrite(record, 1, length, file) == length);
            assert(fclose(file) == 0);
            kest_preset preset = {0};
            kest_sequence sequence = {0};
            int rc = sequence_file ? read_sequence_from_file(&sequence, path)
                                   : read_preset_from_file(&preset, path);
            assert((rc == NO_ERROR) == (length == sizeof(record)));
            kest_free(preset.name);
            kest_free(sequence.name);
        }
        FILE *file = fopen(path, "wb");
        assert(file);
        assert(fwrite(record, 1, 2, file) == 2);
        for (int i = 0; i < 129; i++) assert(fputc('x', file) != EOF);
        assert(fclose(file) == 0);
        kest_preset preset = {0};
        kest_sequence sequence = {0};
        int rc = sequence_file ? read_sequence_from_file(&sequence, path)
                               : read_preset_from_file(&preset, path);
        assert(rc == ERR_MANGLED_FILE);
        assert(preset.name == NULL && sequence.name == NULL);
    }
    assert(unlink(path) == 0);
}

KEST_TEST(test_populated_sequence_roundtrip_and_truncated_references)
{
    char path[] = "/tmp/kestXXXXXX";
    temporary_save_file(path);
    kest_preset first = { .has_fname = 1 }, second = { .has_fname = 1 };
    strcpy(first.fname, "FIRST.PRS");
    strcpy(second.fname, "SECOND.PRS");
    kest_preset_pll second_node = { .data = &second };
    kest_preset_pll first_node = { .data = &first, .next = &second_node };
    kest_preset_pll *saved = global_cxt.presets;
    global_cxt.presets = &first_node;
    seq_kest_preset_pll tail = { .data = &second };
    seq_kest_preset_pll head = { .data = &first, .next = &tail };
    kest_sequence original = { .name = "Set", .presets = &head };
    assert(save_sequence_as_file(&original, path) == NO_ERROR);
    unsigned char bytes[128];
    FILE *file = fopen(path, "rb");
    assert(file);
    size_t length = fread(bytes, 1, sizeof(bytes), file);
    assert(length < sizeof(bytes) && fclose(file) == 0);
    for (size_t prefix = 0; prefix <= length; prefix++)
    {
        file = fopen(path, "wb");
        assert(file && fwrite(bytes, 1, prefix, file) == prefix);
        assert(fclose(file) == 0);
        first.sequence = second.sequence = &original;
        kest_sequence loaded = {0};
        int rc = read_sequence_from_file(&loaded, path);
        assert((rc == NO_ERROR) == (prefix == length));
        if (rc == NO_ERROR)
        {
            assert(strcmp(loaded.name, "Set") == 0);
            assert(loaded.presets && loaded.presets->data == &first);
            assert(loaded.presets->next && loaded.presets->next->data == &second);
            assert(loaded.presets->next->next == NULL);
            assert(loaded.has_fname && !loaded.unsaved_changes);
            assert(first.sequence == &loaded && second.sequence == &loaded);
            kest_context saved_context = global_cxt;
            global_cxt.active_preset = NULL;
            global_cxt.sequence = NULL;
            assert(kest_sequence_begin_at(&loaded, &second) == NO_ERROR);
            assert(global_cxt.active_preset == &second);
            assert(kest_sequence_regress(&loaded) == NO_ERROR);
            assert(global_cxt.active_preset == &first);
            assert(loaded.position == loaded.presets);
            assert(kest_sequence_advance(&loaded) == NO_ERROR);
            assert(global_cxt.active_preset == &second);
            global_cxt = saved_context;
        }
        else
        {
            assert(loaded.name == NULL && loaded.presets == NULL);
            assert(!loaded.has_fname);
            assert(first.sequence == &original && second.sequence == &original);
        }
        while (loaded.presets)
        {
            seq_kest_preset_pll *next = loaded.presets->next;
            kest_free(loaded.presets);
            loaded.presets = next;
        }
        kest_free(loaded.name);
        first.sequence = second.sequence = NULL;
    }
    kest_preset missing = { .has_fname = 1 }, preceding = {0};
    strcpy(missing.fname, "MISSING.PRS");
    missing.sequence = &original;
    seq_kest_preset_pll skipped = { .data = &missing, .next = &tail };
    head.next = &skipped;
    assert(save_sequence_as_file(&original, path) == NO_ERROR);
    file = fopen(path, "rb");
    assert(file);
    length = fread(bytes, 1, sizeof(bytes), file);
    assert(length < sizeof(bytes) && fclose(file) == 0);
    for (size_t prefix = 0; prefix <= length; prefix++)
    {
        file = fopen(path, "wb");
        assert(file && fwrite(bytes, 1, prefix, file) == prefix);
        assert(fclose(file) == 0);
        seq_kest_preset_pll anchor = { .data = &preceding };
        kest_sequence loaded = { .presets = &anchor };
        first.sequence = second.sequence = &original;
        int rc = read_sequence_from_file(&loaded, path);
        assert((rc == NO_ERROR) == (prefix == length));
        assert(loaded.presets == &anchor && anchor.prev == NULL);
        assert(missing.sequence == &original);
        if (rc == NO_ERROR)
        {
            assert(anchor.next && anchor.next->data == &first);
            assert(anchor.next->next && anchor.next->next->data == &second);
            assert(anchor.next->next->next == NULL);
            kest_context saved_context = global_cxt;
            global_cxt.active_preset = NULL;
            assert(kest_sequence_begin_at(&loaded, &second) == NO_ERROR);
            assert(kest_sequence_regress(&loaded) == NO_ERROR);
            assert(global_cxt.active_preset == &first);
            assert(kest_sequence_regress(&loaded) == NO_ERROR);
            assert(global_cxt.active_preset == &preceding);
            assert(kest_sequence_advance(&loaded) == NO_ERROR);
            assert(global_cxt.active_preset == &first);
            global_cxt = saved_context;
        }
        else
        {
            assert(anchor.next == NULL && loaded.name == NULL && !loaded.has_fname);
            assert(first.sequence == &original && second.sequence == &original);
        }
        while (anchor.next)
        {
            seq_kest_preset_pll *node = anchor.next;
            anchor.next = node->next;
            kest_free(node);
        }
        kest_free(loaded.name);
        first.sequence = second.sequence = NULL;
    }
    global_cxt.presets = saved;
    assert(unlink(path) == 0);
}

static void release_unpublished_loaded_preset(kest_preset *preset)
{
    while (preset->pipeline.effects)
    {
        kest_effect_pll *node = preset->pipeline.effects;
        preset->pipeline.effects = node->next;
        kest_effect_free_retired(node->data);
        kest_free(node);
    }
    kest_free(preset->name);
}

static void populated_preset_roundtrip(const char *fixture, int with_settings)
{
    char path[] = "/tmp/kestXXXXXX";
    temporary_save_file(path);
    kest_effect_desc *desc = kest_read_eff_desc_from_file(fixture);
    assert(desc);
    kest_effect_desc_pll node = { .data = desc };
    kest_effect_desc_pll *saved = global_cxt.effects;
    global_cxt.effects = &node;
    kest_allocator saved_allocator = kest_dsp_resource_allocator;
    kest_dsp_resource_pool resources;
    assert(kest_dsp_resource_pool_init(&resources) == NO_ERROR);
    assert(kest_dsp_resource_pool_reserve(&resources, 2) == NO_ERROR);
    kest_dsp_resource_pool_init_allocator(&resources, &kest_dsp_resource_allocator);
    kest_allocator saved_settings = kest_setting_allocator;
    kest_setting_pool settings;
    if (with_settings)
    {
        assert(kest_setting_pool_init(&settings) == NO_ERROR);
        assert(kest_setting_pool_reserve(&settings, 4) == NO_ERROR);
        kest_setting_pool_init_allocator(&settings, &kest_setting_allocator);
    }
    kest_preset original = {0};
    original.name = kest_strndup("Loaded", 16);
    kest_effect *effect = kest_pipeline_append_effect_eff(&original.pipeline, desc);
    assert(effect && effect->parameters && effect->parameters->data);
    effect->id = 37;
    effect->parameters->data->value = 0.625f;
    if (with_settings)
    {
        assert(effect->settings && effect->settings->next && !effect->settings->next->next);
        effect->settings->data->value = 0x01020304;
        effect->settings->next->data->value = -123456789;
    }
    assert(save_preset_as_file(&original, path) == NO_ERROR);
    unsigned char bytes[256];
    FILE *file = fopen(path, "rb");
    assert(file);
    size_t length = fread(bytes, 1, sizeof(bytes), file);
    assert(length < sizeof(bytes) && fclose(file) == 0);
    assert(resources.free_count == 1);
    for (size_t prefix = 0; prefix < length; prefix++)
    {
        file = fopen(path, "wb");
        assert(file && fwrite(bytes, 1, prefix, file) == prefix);
        assert(fclose(file) == 0);
        kest_preset rejected = {0};
        assert(read_preset_from_file(&rejected, path) != NO_ERROR);
        assert(rejected.name == NULL && rejected.pipeline.effects == NULL);
        assert(!rejected.has_fname);
        assert(resources.free_count == 1);
        if (with_settings) assert(settings.free_count == 2);
    }
    file = fopen(path, "wb");
    assert(file && fwrite(bytes, 1, length, file) == length);
    assert(fclose(file) == 0);
    kest_allocator saved_parameters = kest_parameter_allocator;
    kest_parameter_pool parameters;
    assert(kest_parameter_pool_init(&parameters) == NO_ERROR);
    assert(kest_parameter_pool_reserve(&parameters, 1) == NO_ERROR);
    kest_parameter_pool_init_allocator(&parameters, &kest_parameter_allocator);
    kest_parameter *held = kest_parameter_pool_obtain(&parameters);
    assert(held);
    for (int pass = 0; pass < 3; pass++)
    {
        kest_effect_pll *existing = original.pipeline.effects;
        char *existing_name = original.name;
        assert(read_preset_from_file(&original, path) != NO_ERROR);
        assert(original.name == existing_name && original.pipeline.effects == existing);
        assert(existing->next == NULL && !original.has_fname);
        assert(parameters.free_count == 0 && resources.free_count == 1);
        if (with_settings) assert(settings.free_count == 2);
    }
    assert(kest_parameter_pool_return(&parameters, held) == NO_ERROR);
    kest_parameter_allocator = saved_parameters;
    kest_free(parameters.entries);
    kest_free(parameters.buffer);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(parameters.mutex);
#endif
    global_cxt.effects = NULL;
    for (int pass = 0; pass < 3; pass++)
    {
        kest_effect_pll *existing = original.pipeline.effects;
        char *existing_name = original.name;
        assert(read_preset_from_file(&original, path) == ERR_MANGLED_FILE);
        assert(original.name == existing_name && original.pipeline.effects == existing);
        assert(existing->next == NULL && !original.has_fname);
        assert(resources.free_count == 1);
        if (with_settings) assert(settings.free_count == 2);
    }
    global_cxt.effects = &node;
    kest_preset loaded = {0};
    assert(read_preset_from_file(&loaded, path) == NO_ERROR);
    assert(strcmp(loaded.name, "Loaded") == 0);
    assert(loaded.pipeline.effects && !loaded.pipeline.effects->next);
    kest_effect *copy = loaded.pipeline.effects->data;
    assert(copy->eff == desc && copy->id == 37);
    assert(copy->parameters->data->value == 0.625f);
    if (with_settings)
    {
        assert(copy->settings && copy->settings->next && !copy->settings->next->next);
        assert(copy->settings->data->value == 0x01020304);
        assert(copy->settings->next->data->value == -123456789);
    }
    assert(copy->resources.count == effect->resources.count);
    release_unpublished_loaded_preset(&loaded);
    release_unpublished_loaded_preset(&original);
    assert(resources.free_count == 2);
    if (with_settings)
    {
        assert(settings.free_count == 4);
        kest_setting_allocator = saved_settings;
        kest_free(settings.entries);
        kest_free(settings.buffer);
#ifdef KEST_USE_FREERTOS
        vSemaphoreDelete(settings.mutex);
#endif
    }
    kest_dsp_resource_allocator = saved_allocator;
    kest_free(resources.entries);
    kest_free(resources.buffer);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(resources.mutex);
#endif
    global_cxt.effects = saved;
    assert(unlink(path) == 0);
}

KEST_TEST(test_populated_preset_roundtrip)
{
    populated_preset_roundtrip("tests/fixtures/readback.eff", 0);
}

KEST_TEST(test_populated_preset_integer_settings_roundtrip)
{
    populated_preset_roundtrip("tests/fixtures/preset-settings.eff", 1);
}

static int open_directory_fd_count(void)
{
    DIR *directory = opendir("/proc/self/fd");
    assert(directory);
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(directory)))
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) count++;
    assert(closedir(directory) == 0);
    return count;
}

KEST_TEST(test_directory_scan_releases_handles)
{
    char directory[] = "/tmp/kest-scan-XXXXXX";
    assert(mkdtemp(directory));
    char prefix[128], path[128];
    snprintf(prefix, sizeof(prefix), "%s/", directory);
    int baseline = open_directory_fd_count();
    assert(list_files_in_directory(prefix) == NULL);
    assert(open_directory_fd_count() == baseline);
    snprintf(path, sizeof(path), "%s/file.eff", directory);
    FILE *file = fopen(path, "w");
    assert(file && fclose(file) == 0);
    for (int attempt = 0; attempt < 3; attempt++)
    {
        string_ll *files = list_files_in_directory(prefix);
        assert(files && !files->next && strcmp(files->data, path) == 0);
        char_pll_free(files);
        assert(open_directory_fd_count() == baseline);
    }
    assert(unlink(path) == 0);
    assert(rmdir(directory) == 0);
}
