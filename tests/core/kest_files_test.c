#include "kest_test.h"
#include <unistd.h>

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
