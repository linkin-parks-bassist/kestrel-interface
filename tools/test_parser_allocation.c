#define _GNU_SOURCE
#include "kest_lib.h"
#include <assert.h>
#include <dlfcn.h>
static int armed, failures, calls, fail_at, live;
static void *(*real_alloc)(size_t);
static void (*real_free)(void *);
static void *(*real_realloc)(void *, size_t);
static int reject_realloc;
static void *scope_table;
static int refused_descriptors;
static void *refuse_descriptor(void *data, size_t size)
{
    (void)data; (void)size;
    refused_descriptors++;
    return NULL;
}
static kest_allocator descriptor_delegate;
static int metadata_start;
static void *record_metadata_start(void *data, size_t size)
{
    (void)data;
    void *ptr = kest_allocator_alloc(&descriptor_delegate, size);
    metadata_start = calls;
    return ptr;
}
void *kest_alloc(size_t size)
{
    if (!real_alloc) real_alloc = dlsym(RTLD_NEXT, "kest_alloc");
    assert(real_alloc);
    if (armed && (++calls == fail_at ||
        (fail_at == -1 && size == sizeof(kest_scope_entry_dict_bucket) * 32)))
    {
        failures++;
        return NULL;
    }
    void *ptr = real_alloc(size);
    if (armed && ptr) live++;
    if (armed && ptr && size == sizeof(kest_scope_entry_dict_bucket) * 32)
    {
        assert(scope_table == NULL);
        scope_table = ptr;
    }
    return ptr;
}
void kest_free(void *ptr)
{
    if (!real_free) real_free = dlsym(RTLD_NEXT, "kest_free");
    assert(real_free);
    if (armed && ptr) live--;
    if (ptr == scope_table) scope_table = NULL;
    real_free(ptr);
}
void *kest_realloc(void *ptr, size_t size)
{
    if (!real_realloc) real_realloc = dlsym(RTLD_NEXT, "kest_realloc");
    assert(real_realloc);
    if (armed && reject_realloc)
    {
        failures++;
        return NULL;
    }
    return real_realloc(ptr, size);
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    assert(kest_mem_init() == NO_ERROR);
    /* Exercise heap fault injection; typed template exhaustion has its own unit test. */
    kest_parameter_allocator = (kest_allocator){0};
    kest_setting_allocator = (kest_allocator){0};
    kest_dsp_resource_allocator = (kest_allocator){0};
    assert(kest_eff_parser_init_mempool() == NO_ERROR);
    kest_scope scope = {0};
    armed = 1;
    assert(kest_scope_init(&scope) == NO_ERROR);
    int allocations = calls;
    assert(scope.count == 6);
    assert(kest_scope_lookup(&scope, "t")->val.expr == &kest_expression_t);
    kest_scope_entry_dict_destroy(&scope.dict, NULL);
    assert(live == 0);
    armed = 0;
    for (int at = 1; at <= allocations; at++)
    {
        scope = (kest_scope){0};
        calls = 0;
        fail_at = at;
        armed = 1;
        assert(kest_scope_init(&scope) == ERR_ALLOC_FAIL);
        assert(scope.count == 0 && scope.dict.buckets == NULL && live == 0);
        armed = 0;
    }
    kest_expression zero = {0}, one = {0}, ref = {0};
    assert(kest_expr_init_const(&zero, 0) == NO_ERROR);
    assert(kest_expr_init_const(&one, 1) == NO_ERROR);
    ref.type = KEST_EXPR_REF;
    ref.val.ref_name = "borrowed";
    for (int pass = 0; pass < 3; pass++)
        for (int populated = 0; populated < 2; populated++)
            for (int rejection = 0; rejection < 2; rejection++)
            {
                kest_block existing = {0};
                kest_block_pll held = { .data = &existing };
                kest_asm_line line = { .instr = "mov", .n_args = 2, .line_number = 1,
                    .args = { { .type = KEST_ASM_ARG_CHANNEL, .expr = &zero },
                              { .type = rejection ? KEST_ASM_ARG_INT : KEST_ASM_ARG_CHANNEL,
                                .expr = &one } } };
                kest_asm_line_pll lines = { .data = &line };
                kest_eff_parsing_state ps = { .asm_lines = &lines,
                    .blocks = populated ? &held : NULL };
                calls = failures = 0;
                fail_at = rejection ? 0 : 2; // Block, then list cell.
                armed = 1;
                assert(kest_process_asm_lines(&ps) == (rejection ? ERR_BAD_ARGS : ERR_ALLOC_FAIL));
                assert(live == 0 && failures == (rejection ? 0 : 1));
                assert(ps.blocks == (populated ? &held : NULL));
                assert(held.data == &existing && held.next == NULL);
                assert(kest_expression_evaluate(&one, NULL) == 1);
                assert(kest_expression_evaluate(&zero, NULL) == 0);
                armed = 0;
            }
    kest_eff_entry fields[] = {
        { .name = "name", .type = KEST_EFF_ENTRY_TYPE_STR, .value.val_string = "Level" },
        { .name = "default", .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &zero },
        { .name = "min", .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &zero },
        { .name = "max", .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &one },
        { .name = "units", .type = KEST_EFF_ENTRY_TYPE_STR, .value.val_string = "dB" },
        { .name = "driver", .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &ref }
    };
    kest_eff_entry_dict parameter_dict = { .entries = fields, .count = 6, .capacity = 6 };
    for (int pass = 0; pass < 3; pass++)
        for (int at = 1; at <= 6; at++)
        {
            kest_eff_parsing_state ps = {0};
            calls = failures = 0;
            fail_at = at;
            armed = 1;
            assert(kest_extract_parameter(&ps, &parameter_dict, "level") == NULL);
            assert(failures == 1 && live == 0);
            assert(ps.drivers.count == 0 && ps.drivers.entries == NULL);
            assert(ref.type == KEST_EXPR_REF && strcmp(ref.val.ref_name, "borrowed") == 0);
            assert(kest_expression_evaluate(&one, NULL) == 1);
            armed = 0;
        }
    kest_eff_parsing_state recovered = {0};
    calls = failures = fail_at = 0;
    armed = 1;
    kest_parameter *parameter = kest_extract_parameter(&recovered, &parameter_dict, "level");
    assert(parameter && calls == 6 && failures == 0);
    assert(strcmp(parameter->name, "Level") == 0 && strcmp(parameter->name_internal, "level") == 0);
    assert(strcmp(parameter->units, "dB") == 0 && parameter->min_expr == &zero && parameter->max_expr == &one);
    assert(recovered.drivers.count == 1);
    assert(((kest_driver_scope_entry *)recovered.drivers.entries[0].data)->key == ref.val.ref_name);
    kest_free(parameter->name_internal);
    kest_free(parameter->name);
    kest_free(parameter->units);
    kest_free(parameter);
    kest_free(recovered.drivers.entries[0].data);
    kest_driver_list_destroy(&recovered.drivers);
    assert(live == 0);
    armed = 0;
    for (int pass = 0; pass < 3; pass++)
        for (int populated = 0; populated < 2; populated++)
            for (int prior_driver = 0; prior_driver < 2; prior_driver++)
            {
                kest_eff_entry entry = { .name = "level", .type = KEST_EFF_ENTRY_TYPE_SUBDICT,
                    .value.val_dict = &parameter_dict };
                kest_eff_desc_file_section section = { .dict_ = { .entries = &entry,
                    .count = 1, .capacity = 1 } };
                kest_ast_node node = { .type = KEST_AST_NODE_SECTION, .data = &section };
                kest_parameter existing = { .name = "held" };
                kest_parameter_pll held = { .data = &existing };
                kest_parameter_pll *parameters = populated ? &held : NULL;
                kest_eff_parsing_state ps = {0};
                calls = failures = fail_at = 0;
                armed = 1;
                if (prior_driver)
                {
                    kest_driver driver;
                    assert(kest_driver_init_scope_entry(&driver, "held") == NO_ERROR);
                    assert(kest_driver_list_append(&ps.drivers, driver) == NO_ERROR);
                }
                void *held_driver = prior_driver ? ps.drivers.entries[0].data : NULL;
                int initial_live = live;
                calls = failures = 0;
                fail_at = prior_driver ? 6 : 7; // Metadata/driver allocations, then parameter list cell.
                assert(kest_parameters_section_extract(&ps, &parameters, &node) == ERR_ALLOC_FAIL);
                assert(failures == 1 && live == initial_live);
                assert(parameters == (populated ? &held : NULL));
                assert(held.data == &existing && held.next == NULL);
                assert(ps.drivers.count == (size_t)prior_driver);
                if (prior_driver)
                {
                    assert(ps.drivers.entries[0].data == held_driver);
                    assert(strcmp(((kest_driver_scope_entry *)held_driver)->key, "held") == 0);
                    kest_free(held_driver);
                    kest_driver_list_destroy(&ps.drivers);
                }
                else assert(ps.drivers.entries == NULL);
                assert(live == 0 && kest_expression_evaluate(&one, NULL) == 1);
                assert(ref.type == KEST_EXPR_REF && strcmp(ref.val.ref_name, "borrowed") == 0);
                armed = 0;
            }
    kest_eff_entry coefficient = { .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &one };
    kest_eff_entry_list coefficients = { .entries = &coefficient, .count = 1, .capacity = 1 };
    const char *resource_types[] = { "mem", "delay", "lfo", "polynomial" };
    for (int pass = 0; pass < 3; pass++)
        for (int kind = 0; kind < 4; kind++)
        {
            kest_eff_entry attrs[6] = {
                { .name = "type", .type = KEST_EFF_ENTRY_TYPE_STR,
                  .value.val_string = (char *)resource_types[kind] }
            };
            int count = 1;
            if (kind == 1)
                attrs[count++] = (kest_eff_entry){ .name = "delay_samples",
                    .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &one };
            if (kind == 2)
                for (int field = 0; field < 3; field++)
                    attrs[count++] = (kest_eff_entry){
                        .name = (const char *[]){ "frequency", "center", "amplitude" }[field],
                        .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &one };
            if (kind == 3)
                attrs[count++] = (kest_eff_entry){ .name = "coefs",
                    .type = KEST_EFF_ENTRY_TYPE_LIST, .value.val_list = &coefficients };
            attrs[count++] = (kest_eff_entry){ .name = "invalid",
                .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &one };
            kest_eff_entry_dict dict = { .entries = attrs, .count = count, .capacity = 6 };
            kest_eff_parsing_state ps = {0};
            calls = failures = fail_at = 0;
            armed = 1;
            assert(kest_extract_resource(&ps, &dict, "probe") == NULL);
            printf("resource rejection %s retained %d tracked allocations\n", resource_types[kind], live);
            fflush(stdout);
            assert(live == 0 && failures == 0);
            assert(kest_expression_evaluate(&one, NULL) == 1);
            armed = 0;
        }
    for (int pass = 0; pass < 3; pass++)
        for (int populated = 0; populated < 2; populated++)
        {
            kest_eff_entry type = { .name = "type", .type = KEST_EFF_ENTRY_TYPE_STR,
                .value.val_string = "mem" };
            kest_eff_entry_dict attrs = { .entries = &type, .count = 1, .capacity = 1 };
            kest_eff_entry entry = { .name = "new", .type = KEST_EFF_ENTRY_TYPE_SUBDICT,
                .value.val_dict = &attrs };
            kest_eff_desc_file_section section = { .dict_ = { .entries = &entry,
                .count = 1, .capacity = 1 } };
            kest_ast_node node = { .type = KEST_AST_NODE_SECTION, .data = &section };
            kest_dsp_resource existing = { .name = "held" };
            kest_dsp_resource_pll held = { .data = &existing };
            kest_dsp_resource_pll *resources = populated ? &held : NULL;
            kest_eff_parsing_state ps = {0};
            calls = failures = 0;
            fail_at = 4; // wrapper, copied name, payload, then list insertion
            armed = 1;
            assert(kest_resources_section_extract(&ps, &resources, &node) == ERR_ALLOC_FAIL);
            assert(failures == 1 && live == 0);
            assert(resources == (populated ? &held : NULL));
            assert(held.data == &existing && held.next == NULL);
            assert(strcmp(existing.name, "held") == 0);
            armed = 0;
        }
    // Definition metadata owns names/wrappers; the expression is borrowed here.
    for (int pass = 0; pass < 3; pass++)
        for (int at = 1; at <= 4; at++)
        {
            kest_eff_parsing_state ps = {0};
            kest_scope definition_scope = {0};
            fail_at = calls = failures = 0;
            armed = 1;
            assert(kest_scope_init(&definition_scope) == NO_ERROR);
            ps.scope = &definition_scope;
            kest_eff_entry entry = { .name = "held", .type = KEST_EFF_ENTRY_TYPE_EXPR,
                .value.val_expr = &one };
            kest_eff_desc_file_section section = { .dict_ = { .entries = &entry,
                .count = 1, .capacity = 1 } };
            kest_ast_node node = { .type = KEST_AST_NODE_SECTION, .data = &section };
            calls = failures = 0;
            fail_at = at;
            assert(kest_defs_section_extract(&ps, &definition_scope, &node) == ERR_ALLOC_FAIL);
            assert(failures == 1);
            kest_scope_entry_dict_destroy(&definition_scope.dict, NULL);
            for (kest_named_expression_pll *item = ps.def_exprs; item; item = item->next)
                kest_free((void *)item->data->name);
            kest_named_expression_pll_free(ps.def_exprs);
            assert(live == 0 && scope_table == NULL);
            assert(kest_expression_evaluate(&one, NULL) == 1);
            armed = 0;
        }
    kest_eff_entry setting_fields[] = {
        fields[0], fields[1], fields[2], fields[3], fields[4],
        { .name = "type", .type = KEST_EFF_ENTRY_TYPE_STR, .value.val_string = "int" },
        { .name = "bogus", .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &one }
    };
    kest_eff_entry_dict setting_dict = { .entries = setting_fields, .count = 6, .capacity = 7 };
    for (int pass = 0; pass < 3; pass++)
        for (int at = 1; at <= 4; at++)
        {
            kest_eff_parsing_state ps = {0};
            calls = failures = 0;
            fail_at = at;
            armed = 1;
            assert(kest_extract_setting(&ps, &setting_dict, "level") == NULL);
            assert(failures == 1 && live == 0);
            assert(kest_expression_evaluate(&one, NULL) == 1);
            armed = 0;
        }
    for (int pass = 0; pass < 3; pass++)
    {
        kest_eff_parsing_state ps = {0};
        setting_dict.count = 7;
        calls = failures = fail_at = 0;
        armed = 1;
        assert(kest_extract_setting(&ps, &setting_dict, "level") == NULL);
        assert(calls == 4 && failures == 0 && live == 0 && ps.errors == 1);
        armed = 0;
    }
    setting_dict.count = 6;
    recovered = (kest_eff_parsing_state){0};
    calls = failures = fail_at = 0;
    armed = 1;
    kest_setting *setting = kest_extract_setting(&recovered, &setting_dict, "level");
    assert(setting && calls == 4 && failures == 0);
    assert(strcmp(setting->name, "Level") == 0 && strcmp(setting->name_internal, "level") == 0);
    assert(strcmp(setting->units, "dB") == 0 && setting->min == 0 && setting->max == 1);
    kest_free(setting->name_internal); kest_free(setting->name); kest_free(setting->units); kest_free(setting);
    assert(live == 0);
    armed = 0;
    failures = 0;
    kest_eff_entry choice_fields[] = {
        { .name = "name", .type = KEST_EFF_ENTRY_TYPE_STR, .value.val_string = "Quarter" },
        { .name = "value", .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &zero }
    };
    kest_eff_entry_dict choice_dict = { .entries = choice_fields, .count = 2, .capacity = 2 };
    kest_eff_entry choice = { .name = "quarter", .type = KEST_EFF_ENTRY_TYPE_SUBDICT, .value.val_dict = &choice_dict };
    kest_eff_entry_dict choices = { .entries = &choice, .count = 1, .capacity = 1 };
    setting_fields[5].value.val_string = "enum";
    setting_fields[6] = (kest_eff_entry){ .name = "options", .type = KEST_EFF_ENTRY_TYPE_SUBDICT, .value.val_dict = &choices };
    setting_dict.count = 7;
    for (int pass = 0; pass < 3; pass++)
    {
        for (int at = 1; at <= 5; at++)
        {
            kest_eff_parsing_state ps = {0};
            calls = failures = 0;
            fail_at = at;
            armed = 1;
            assert(kest_extract_setting(&ps, &setting_dict, "division") == NULL);
            assert(failures == 1 && live == 0);
            armed = 0;
        }
        kest_eff_parsing_state ps = {0};
        calls = failures = fail_at = 0;
        armed = 1;
        kest_setting *s = kest_extract_setting(&ps, &setting_dict, "division");
        assert(s && calls == 5 && s->n_options == 1 && strcmp(s->options[0].name, "Quarter") == 0);
        gut_setting(s);
        kest_free((void *)s->name_internal); kest_free((void *)s->name); kest_free((void *)s->units); kest_free(s);
        assert(live == 0);
        armed = 0;
    }
    printf("enum choices: five allocation faults and recovery reclaim heap\n");
    for (int pass = 0; pass < 3; pass++)
    {
        assert(kest_eff_parser_reset_mempool() == NO_ERROR);
        calls = 0;
        fail_at = -1;
        armed = 1;
        kest_effect_desc *desc = kest_read_eff_desc_from_file(argv[1]);
        armed = 0;
        assert(failures == pass + 1);
        assert(desc == NULL);
        assert(live == 0);
    }
    assert(kest_eff_parser_reset_mempool() == NO_ERROR);
    calls = fail_at = 0;
    armed = 1;
    assert(kest_read_eff_desc_from_file(argv[1]) != NULL);
    armed = 0;
    assert(scope_table == NULL);
    int baseline = live;
    for (int pass = 0; pass < 3; pass++)
    {
        assert(kest_eff_parser_reset_mempool() == NO_ERROR);
        calls = fail_at = 0;
        armed = 1;
        assert(kest_read_eff_desc_from_file("tests/fixtures/non-dictionary-resource.eff") == NULL);
        armed = 0;
        assert(scope_table == NULL);
        assert(live == baseline);
    }
    kest_allocator saved = kest_expression_allocator;
    kest_expression_pool pool;
    assert(kest_expression_pool_init(&pool) == NO_ERROR);
    assert(kest_expression_pool_reserve(&pool, 16) == NO_ERROR);
    kest_expression_pool_init_allocator(&pool, &kest_expression_allocator);
    for (int pass = 0; pass < 3; pass++)
    {
        assert(kest_eff_parser_reset_mempool() == NO_ERROR);
        char *words[] = { "\n", "mode", ":", "(", "name", ":", "\"Mode\"", ",", "default", ":", "1", ",", "min", ":", "0", ",", "max", ":", "2", ",", "type", ":", "\"int\"", ",", "units", ":", "\"steps\"", ")" };
        kest_token_ll tokens[28] = {0};
        for (int i = 0; i < 28; i++)
        {
            tokens[i].data = words[i];
            tokens[i].next = i + 1 < 28 ? &tokens[i + 1] : NULL;
        }
        kest_eff_desc_file_section sec = { .tokens = tokens };
        kest_ast_node node = { .type = KEST_AST_NODE_SECTION, .data = &sec };
        kest_eff_parsing_state ps = {0};
        assert(kest_parse_entry_section(&ps, &node) == NO_ERROR);
        assert(pool.free_count == 13);
        kest_setting_pll *settings = NULL;
        calls = failures = 0;
        fail_at = 5;
        armed = 1;
        assert(kest_settings_section_extract(&ps, &settings, &node) == ERR_ALLOC_FAIL);
        armed = 0;
        assert(failures == 1 && live == baseline && settings == NULL);
        assert(pool.free_count == 16 && sec.dict_.entries[0].type == KEST_EFF_ENTRY_TYPE_NOTHING);
    }
    const char *rejected_settings[] = {
        "tests/fixtures/rejected-settings.eff", "tests/fixtures/rejected-settings-defs.eff", "tests/fixtures/settings-only.eff", "tests/fixtures/rejected-settings-code.eff", "tests/fixtures/rejected-resource-code.eff", "tests/fixtures/resource-only.eff", "tests/fixtures/enum-settings.eff"
    };
    for (int pass = 0; pass < 3; pass++)
        for (int c = 0; c < 7; c++)
        {
            assert(kest_eff_parser_reset_mempool() == NO_ERROR);
            kest_allocator descriptor_allocator = kest_effect_desc_allocator;
            int refused_before = refused_descriptors;
            if (c == 2 || c == 5 || c == 6) kest_effect_desc_allocator = (kest_allocator){ .alloc = refuse_descriptor };
            calls = failures = fail_at = 0;
            armed = 1;
            assert(kest_read_eff_desc_from_file((char *)rejected_settings[c]) == NULL);
            kest_effect_desc_allocator = descriptor_allocator;
            if (c == 2 || c == 5 || c == 6) assert(refused_descriptors == refused_before + 1);
            printf("reader rejection %s retained %d tracked allocations\n", rejected_settings[c], live - baseline);
            fflush(stdout);
            assert(failures == 0 && live == baseline && scope_table == NULL);
            assert(pool.free_count == 16);
            armed = 0;
        }
    for (int pass = 0; pass < 3; pass++)
    {
        assert(kest_eff_parser_reset_mempool() == NO_ERROR);
        char source[] = "v1.0\n.INFO\nname: \"probe\", extra: {named, (child: 1 + 2)}\n.DEFS\nfirst: named + 3\n.CODE\n";
        kest_eff_parsing_state ps = { .content = source, .file_size = sizeof(source) - 1 };
        assert(kest_tokenize_content(&ps) == NO_ERROR);
        calls = failures = 0;
        fail_at = -1;
        armed = 1;
        assert(kest_parse_tokens(&ps) == ERR_ALLOC_FAIL);
        armed = 0;
        assert(failures == 1 && live == baseline && scope_table == NULL);
        assert(pool.free_count == 16);
        for (kest_ast_node *node = ps.ast->child; node; node = node->next)
            assert(((kest_eff_desc_file_section *)node->data)->dict_.count == 0);
    }
    for (int pass = 0; pass < 3; pass++)
        for (int growth = 0; growth < 2; growth++)
        {
            assert(kest_eff_parser_reset_mempool() == NO_ERROR);
            char *words[] = { "{", "1", ",", "2", "}" };
            kest_token_ll tokens[5] = {0};
            for (int i = 0; i < 5; i++)
            {
                tokens[i].data = words[i];
                tokens[i].next = i < 4 ? &tokens[i + 1] : NULL;
            }
            kest_eff_parsing_state ps = { .current_token = tokens };
            kest_eff_entry entry = {0};
            calls = failures = 0;
            fail_at = growth ? 0 : 1;
            reject_realloc = growth;
            armed = 1;
            assert(kest_parse_eff_entry(&ps, &entry) == ERR_ALLOC_FAIL);
            armed = reject_realloc = 0;
            assert(failures == 1 && live == baseline);
            assert(entry.type == KEST_EFF_ENTRY_TYPE_NOTHING && pool.free_count == 16);
        }
    kest_expression_allocator = saved;
    kest_free(pool.entries); kest_free(pool.buffer);
    for (int pass = 0; pass < 3; pass++)
        for (int refusal = 0; refusal < 2; refusal++)
        {
            assert(kest_expression_pool_init(&pool) == NO_ERROR);
            assert(kest_expression_pool_reserve(&pool, 16) == NO_ERROR);
            kest_expression_pool_init_allocator(&pool, &kest_expression_allocator);
            assert(kest_eff_parser_reset_mempool() == NO_ERROR);
            kest_allocator descriptor_allocator = kest_effect_desc_allocator;
            int refused_before = refused_descriptors;
            if (refusal) kest_effect_desc_allocator = (kest_allocator){ .alloc = refuse_descriptor };
            calls = failures = fail_at = 0;
            armed = 1;
            assert(kest_read_eff_desc_from_file(refusal ? "tests/fixtures/blocks-only.eff" :
                "tests/fixtures/rejected-block-code.eff") == NULL);
            kest_effect_desc_allocator = descriptor_allocator;
            if (refusal) assert(refused_descriptors == refused_before + 1);
            printf("block reader rejection retained %d tracked allocations\n", live - baseline);
            fflush(stdout);
            assert(failures == 0 && live == baseline && scope_table == NULL);
            assert(pool.free_count == 16);
            armed = 0;
            kest_expression_allocator = saved;
            kest_free(pool.entries); kest_free(pool.buffer);
        }
    // Rejection must restore both metadata and descriptor-owned expression storage.
    for (int pass = 0; pass < 3; pass++)
        for (int scenario = 0; scenario < 4; scenario++)
        {
            int refusal = scenario & 1;
            const char *fixtures[] = { "tests/fixtures/rejected-parameter-code.eff",
                "tests/fixtures/parameter-only.eff", "tests/fixtures/rejected-driven-parameter-code.eff",
                "tests/fixtures/driven-parameter-only.eff" };
            assert(kest_expression_pool_init(&pool) == NO_ERROR);
            assert(kest_expression_pool_reserve(&pool, 16) == NO_ERROR);
            kest_expression_pool_init_allocator(&pool, &kest_expression_allocator);
            assert(kest_eff_parser_reset_mempool() == NO_ERROR);
            kest_allocator descriptor_allocator = kest_effect_desc_allocator;
            int refused_before = refused_descriptors;
            if (refusal) kest_effect_desc_allocator = (kest_allocator){ .alloc = refuse_descriptor };
            calls = failures = fail_at = 0;
            armed = 1;
            assert(kest_read_eff_desc_from_file((char *)fixtures[scenario]) == NULL);
            kest_effect_desc_allocator = descriptor_allocator;
            if (refusal) assert(refused_descriptors == refused_before + 1);
            printf("parameter reader rejection retained %d heap allocations, %zu expression slots\n",
                live - baseline, (size_t)(16 - pool.free_count));
            fflush(stdout);
            assert(failures == 0 && live == baseline && scope_table == NULL);
            assert(pool.free_count == 16);
            armed = 0;
            kest_expression_allocator = saved;
            kest_free(pool.entries); kest_free(pool.buffer);
        }
    // Sweep only the published descriptor's metadata copy boundary, not unrelated parser faults.
    assert(kest_eff_parser_reset_mempool() == NO_ERROR);
    descriptor_delegate = kest_effect_desc_allocator;
    kest_effect_desc_allocator = (kest_allocator){ .alloc = record_metadata_start };
    calls = failures = fail_at = 0;
    armed = 1;
    kest_effect_desc *metadata = kest_read_eff_desc_from_file("tests/fixtures/string-list-info.eff");
    assert(metadata && metadata->genres.count == 2);
    int first_copy = metadata_start + 1, last_copy = calls;
    assert(last_copy >= first_copy);
    kest_effect_desc_allocator = descriptor_delegate;
    kest_effect_desc_retire(metadata);
    assert(live == baseline);
    armed = 0;
    for (int pass = 0; pass < 3; pass++)
    for (int at = first_copy; at <= last_copy; at++)
    {
        assert(kest_eff_parser_reset_mempool() == NO_ERROR);
        calls = failures = 0;
        fail_at = at;
        armed = 1;
        assert(kest_read_eff_desc_from_file("tests/fixtures/string-list-info.eff") == NULL);
        assert(failures == 1 && live == baseline && scope_table == NULL);
        fail_at = 0;
        metadata = kest_read_eff_desc_from_file("tests/fixtures/string-list-info.eff");
        assert(metadata && metadata->keywords.count == 3);
        kest_effect_desc_retire(metadata);
        assert(live == baseline);
        armed = 0;
    }
    printf("discovery metadata: %d allocation boundaries rejected/recovered over three passes with no retained heap\n", last_copy - first_copy + 1);
    printf("scope initialization: %d allocation failures reclaimed; descriptor rejection/recovery, temporary-scope cleanup, six parameter/four definition metadata/four setting heap faults/recovery and setting append/pre-extraction/list rollback pass\n", allocations);
    return 0;
}
