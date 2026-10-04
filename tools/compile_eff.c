#include "kest_lib.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s INPUT.eff OUTPUT.bin [PARAMETER=VALUE ...]\n", argv[0]);
        return 2;
    }
    int result = kest_mem_init();
    if (result != NO_ERROR) goto failed;
    kest_effect_desc *desc = kest_read_eff_desc_from_file(argv[1]);
    if (!desc)
    {
        fprintf(stderr, "Could not parse %s\n", argv[1]);
        return 1;
    }
    kest_effect effect;
    result = init_effect_from_effect_desc(&effect, desc);
    if (result != NO_ERROR) goto failed;
    for (int i = 3; i < argc; ++i)
    {
        char *equals = strchr(argv[i], '=');
        char *end;
        if (!equals || equals == argv[i]) return 2;
        float value = strtof(equals + 1, &end);
        if (end == equals + 1 || *end || !isfinite(value)) return 2;
        *equals = '\0';
        result = kest_effect_set_parameter(&effect, argv[i], value);
        if (result != NO_ERROR) goto failed;
    }
    kest_effect_pll node = { .data = &effect, .next = NULL };
    kest_pipeline pipeline = { .effects = &node };
    kest_fpga_transfer_batch batch = {0};
    result = kest_pipeline_create_fpga_transfer_batch(&pipeline, &batch);
    if (result != NO_ERROR) goto failed;

    FILE *file = fopen(argv[2], "wb");
    if (!file)
    {
        perror(argv[2]);
        kest_free_fpga_transfer_batch(batch);
        return 1;
    }
    int written = fwrite(batch.buf, 1, batch.len, file) == (size_t)batch.len;
    if (fclose(file) != 0) written = 0;
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
