#include "kest_test.h"
#include <unistd.h>

KEST_TEST(test_pipeline_rejects_failed_resource_scope)
{
    kest_mem_slot mem = {0};
    kest_dsp_resource resource = { .type = KEST_DSP_RESOURCE_MEM, .data = &mem };
    kest_dsp_resource_pll node = { .data = &resource };
    kest_effect_desc desc = { .resources = &node };
    kest_allocator saved_effects = kest_effect_allocator;
    kest_allocator saved_resources = kest_dsp_resource_allocator;
    kest_effect_pool effects;
    kest_dsp_resource_pool resources;
    assert(kest_effect_pool_init(&effects) == NO_ERROR);
    assert(kest_dsp_resource_pool_init(&resources) == NO_ERROR);
    assert(kest_effect_pool_reserve(&effects, 1) == NO_ERROR);
    assert(kest_dsp_resource_pool_reserve(&resources, 1) == NO_ERROR);
    kest_effect_pool_init_allocator(&effects, &kest_effect_allocator);
    kest_dsp_resource_pool_init_allocator(&resources, &kest_dsp_resource_allocator);
    kest_pipeline pipeline = {0};
    for (int pass = 0; pass < 3; pass++)
    {
        assert(kest_pipeline_append_effect_eff(&pipeline, &desc) == NULL);
        assert(pipeline.effects == NULL);
        assert(effects.free_count == 1 && resources.free_count == 1);
    }
    resource.name = "probe";
    kest_effect *effect = kest_pipeline_append_effect_eff(&pipeline, &desc);
    assert(effect && effect->scope && kest_scope_lookup(effect->scope, "probe"));
    assert(effects.free_count == 0 && resources.free_count == 0);
    kest_effect_free_retired(effect);
    kest_free(pipeline.effects);
    assert(effects.free_count == 1 && resources.free_count == 1);
    kest_effect_allocator = saved_effects;
    kest_dsp_resource_allocator = saved_resources;
    kest_free(effects.entries); kest_free(effects.buffer);
    kest_free(resources.entries); kest_free(resources.buffer);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(effects.mutex);
    vSemaphoreDelete(resources.mutex);
#endif
}

KEST_TEST(test_pipeline_rejects_failed_driver_clone)
{
    kest_effect_desc desc = {0};
    kest_driver driver = { .type = KEST_DRIVER_SCOPE_ENTRY, .data = NULL };
    assert(kest_driver_list_append(&desc.drivers, driver) == NO_ERROR);
    assert(kest_driver_make_clone(&driver) == NULL);
    kest_pipeline pipeline = {0};
    for (int pass = 0; pass < 3; pass++)
    {
        assert(kest_pipeline_append_effect_eff(&pipeline, &desc) == NULL);
        assert(pipeline.effects == NULL);
    }
    kest_driver_list_destroy(&desc.drivers);
}

KEST_TEST(test_driver_clone_rebinds_runtime_and_borrows_key)
{
    kest_scope scope = {0};
    kest_scope_entry entry = {0};
    kest_driver_scope_entry source = { .key = "test", .scope = &scope, .entry = &entry };
    kest_driver driver = { .type = KEST_DRIVER_SCOPE_ENTRY, .data = &source };
    kest_driver *clone = kest_driver_make_clone(&driver);
    assert(clone && clone->data != &source);
    kest_driver_scope_entry *runtime = clone->data;
    assert(runtime->key == source.key && runtime->scope == NULL && runtime->entry == NULL);
    kest_free(clone->data);
    kest_free(clone);
}

KEST_TEST(test_pipeline_append_reclaims_failed_instance)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/preset-settings.eff");
    assert(desc);
    kest_allocator saved_effects = kest_effect_allocator;
    kest_allocator saved_resources = kest_dsp_resource_allocator;
    kest_allocator saved_settings = kest_setting_allocator;
    kest_effect_pool effects;
    kest_dsp_resource_pool resources;
    kest_setting_pool settings;
    assert(kest_effect_pool_init(&effects) == NO_ERROR);
    assert(kest_dsp_resource_pool_init(&resources) == NO_ERROR);
    assert(kest_setting_pool_init(&settings) == NO_ERROR);
    assert(kest_effect_pool_reserve(&effects, 1) == NO_ERROR);
    assert(kest_dsp_resource_pool_reserve(&resources, 1) == NO_ERROR);
    assert(kest_setting_pool_reserve(&settings, 1) == NO_ERROR);
    kest_effect_pool_init_allocator(&effects, &kest_effect_allocator);
    kest_dsp_resource_pool_init_allocator(&resources, &kest_dsp_resource_allocator);
    kest_setting_pool_init_allocator(&settings, &kest_setting_allocator);
    for (int pass = 0; pass < 3; pass++)
    {
        kest_pipeline pipeline = {0};
        kest_dsp_resource *held = kest_dsp_resource_pool_obtain(&resources);
        assert(held);
        assert(kest_pipeline_append_effect_eff(&pipeline, desc) == NULL);
        assert(pipeline.effects == NULL && effects.free_count == 1);
        assert(resources.free_count == 0 && settings.free_count == 1);
        assert(kest_dsp_resource_pool_return(&resources, held) == NO_ERROR);
        assert(kest_pipeline_append_effect_eff(&pipeline, desc) == NULL);
        assert(pipeline.effects == NULL && effects.free_count == 1);
        assert(resources.free_count == 1 && settings.free_count == 1);
    }
    kest_effect_allocator = saved_effects;
    kest_dsp_resource_allocator = saved_resources;
    kest_setting_allocator = saved_settings;
    kest_free(effects.entries); kest_free(effects.buffer);
    kest_free(resources.entries); kest_free(resources.buffer);
    kest_free(settings.entries); kest_free(settings.buffer);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(effects.mutex);
    vSemaphoreDelete(resources.mutex);
    vSemaphoreDelete(settings.mutex);
#endif
}

KEST_TEST(test_pipeline_stage_reload_preserves_identity_and_rejects_incompatible_values)
{
    kest_parameter old_parameter = {
        .value = 0.25f, .min = 0.0f, .max = 1.0f,
        .name = "Amount", .name_internal = "amount", .units = "ratio"
    };
    kest_parameter new_parameter = {
        .value = 0.75f, .min = 0.0f, .max = 1.0f,
        .name = "Depth", .name_internal = "amount", .units = "ratio"
    };
    kest_parameter narrow_parameter = {
        .value = 0.2f, .min = 0.0f, .max = 0.5f,
        .name = "Depth", .name_internal = "amount", .units = "ratio"
    };
    kest_parameter_pll old_node = { .data = &old_parameter };
    kest_parameter_pll new_node = { .data = &new_parameter };
    kest_parameter_pll narrow_node = { .data = &narrow_parameter };
    kest_effect_desc old_desc = { .cname = "reload", .parameters = &old_node };
    kest_effect_desc new_desc = { .cname = "reload", .parameters = &new_node };
    kest_effect_desc narrow_desc = { .cname = "reload", .parameters = &narrow_node };
    kest_effect_desc other_desc = { .cname = "other" };
    kest_pipeline source = {0}, staged = {0}, rejected = {0};

    kest_effect *old = kest_pipeline_append_effect_eff(&source, &old_desc);
    kest_effect *other = kest_pipeline_append_effect_eff(&source, &other_desc);
    assert(old && other);
    effect_set_id(old, 7, 11);
    effect_set_id(other, 7, 19);
    old->parameters->data->value = 0.75f;

    assert(kest_pipeline_stage_reload(&staged, &source, &old_desc, &new_desc) == NO_ERROR);
    assert(source.effects->data == old && source.effects->next->data == other);
    assert(staged.effects->data != old && staged.effects->data->eff == &new_desc);
    assert(staged.effects->data->id == 11 && staged.effects->data->parameters->data->value == 0.75f);
    assert(staged.effects->next->data->id == 19 && staged.effects->next->data->eff == &other_desc);

    assert(kest_pipeline_stage_reload(&rejected, &source, &old_desc, &narrow_desc) == ERR_BAD_ARGS);
    assert(rejected.effects == NULL && source.effects->data == old);

    kest_pipeline_discard_staged(&staged);
    kest_pipeline_discard_staged(&source);
    assert(old_desc.instance_refs == 0 && new_desc.instance_refs == 0 &&
           narrow_desc.instance_refs == 0 && other_desc.instance_refs == 0);
}

KEST_TEST(test_reload_does_not_transfer_a_renamed_enum_choice)
{
    kest_setting_option old_options[] = {{ .value = 0, .name = "Quarter" }, { .value = 2, .name = "Dotted eighth" }};
    kest_setting_option new_options[] = {{ .value = 0, .name = "Quarter" }, { .value = 2, .name = "Triplet" }};
    kest_setting old_setting = { .type = EFFECT_SETTING_ENUM, .value = 2,
        .min = 0, .max = 2, .name = "Subdivision", .name_internal = "division",
        .n_options = 2, .options = old_options, .widget_type = SETTING_WIDGET_DROPDOWN };
    kest_setting new_setting = old_setting;
    new_setting.value = 0;
    new_setting.options = new_options;
    kest_setting_pll old_node = { .data = &old_setting }, new_node = { .data = &new_setting };
    kest_effect_desc old_desc = { .cname = "rhythm", .settings = &old_node };
    kest_effect_desc new_desc = { .cname = "rhythm", .settings = &new_node };
    kest_pipeline source = {0}, staged = {0};
    kest_effect *old = kest_pipeline_append_effect_eff(&source, &old_desc);
    assert(old);
    assert(kest_pipeline_stage_reload(&staged, &source, &old_desc, &new_desc) == NO_ERROR);
    assert(staged.effects->data->settings->data->value == 0);
    assert(source.effects->data->settings->data->value == 2);
    kest_pipeline_discard_staged(&staged);
    new_options[1].name = "Dotted eighth";
    assert(kest_pipeline_stage_reload(&staged, &source, &old_desc, &new_desc) == NO_ERROR);
    assert(staged.effects->data->settings->data->value == 2);
    kest_pipeline_discard_staged(&staged);
    kest_pipeline_discard_staged(&source);
}

KEST_TEST(test_reload_enum_reordering_removal_and_range_rejection)
{
    kest_setting_option old_options[] = {{ .value = 0, .name = "Quarter" }, { .value = 2, .name = "Dotted eighth" }};
    kest_setting_option reordered[] = {{ .value = 2, .name = "Dotted eighth" },
        { .value = 4, .name = "Whole" }, { .value = 0, .name = "Quarter" }};
    kest_setting_option removed[] = {{ .value = 0, .name = "Quarter" }, { .value = 4, .name = "Whole" }};
    kest_setting old_setting = { .type = EFFECT_SETTING_ENUM, .value = 2,
        .min = 0, .max = 2, .name = "Subdivision", .name_internal = "division",
        .n_options = 2, .options = old_options, .widget_type = SETTING_WIDGET_DROPDOWN };
    kest_setting replacement = old_setting;
    replacement.value = 4;
    replacement.max = 4;
    replacement.n_options = 3;
    replacement.options = reordered;
    kest_setting_pll old_node = { .data = &old_setting }, new_node = { .data = &replacement };
    kest_effect_desc old_desc = { .cname = "rhythm", .settings = &old_node };
    kest_effect_desc new_desc = { .cname = "rhythm", .settings = &new_node };
    kest_pipeline source = {0}, staged = {0};
    kest_effect *old = kest_pipeline_append_effect_eff(&source, &old_desc);
    assert(old);
    assert(kest_pipeline_stage_reload(&staged, &source, &old_desc, &new_desc) == NO_ERROR);
    assert(staged.effects->data->settings->data->value == 2);
    kest_pipeline_discard_staged(&staged);

    replacement.n_options = 2;
    replacement.options = removed;
    assert(kest_pipeline_stage_reload(&staged, &source, &old_desc, &new_desc) == NO_ERROR);
    assert(staged.effects->data->settings->data->value == 4);
    kest_pipeline_discard_staged(&staged);

    replacement.value = 0;
    replacement.max = 1;
    replacement.n_options = 1;
    assert(kest_pipeline_stage_reload(&staged, &source, &old_desc, &new_desc) == ERR_BAD_ARGS);
    assert(staged.effects == NULL && source.effects->data == old);
    assert(old->settings->data->value == 2 && old->eff == &old_desc);
    kest_pipeline_discard_staged(&source);
    assert(old_desc.instance_refs == 0 && new_desc.instance_refs == 0);
}

KEST_TEST(test_reload_partial_staging_pool_failure_preserves_source)
{
    kest_allocator saved_effects = kest_effect_allocator;
    kest_effect_pool pool;
    assert(kest_effect_pool_init(&pool) == NO_ERROR);
    assert(kest_effect_pool_reserve(&pool, 4) == NO_ERROR);
    kest_effect_pool_init_allocator(&pool, &kest_effect_allocator);
    kest_effect_desc old_desc = { .cname = "reload" };
    kest_effect_desc replacement = { .cname = "reload" };
    kest_pipeline source = {0}, staged = {0};
    kest_effect *first = kest_pipeline_append_effect_eff(&source, &old_desc);
    kest_effect *second = kest_pipeline_append_effect_eff(&source, &old_desc);
    assert(first && second);
    effect_set_id(first, 7, 11);
    effect_set_id(second, 7, 19);
    first->wet_mix.value = 0.2f;
    second->wet_mix.value = 0.8f;
    kest_effect *held = kest_effect_pool_obtain(&pool);
    assert(held && pool.free_count == 1);
    for (int pass = 0; pass < 3; pass++)
    {
        assert(kest_pipeline_stage_reload(&staged, &source, &old_desc, &replacement) == ERR_BAD_ARGS);
        assert(staged.effects == NULL && pool.free_count == 1);
        assert(source.effects->data == first && source.effects->next->data == second);
        assert(first->id == 11 && second->id == 19);
        assert(first->wet_mix.value == 0.2f && second->wet_mix.value == 0.8f);
        assert(old_desc.instance_refs == 2 && replacement.instance_refs == 0);
    }
    assert(kest_effect_pool_return(&pool, held) == NO_ERROR);
    assert(kest_pipeline_stage_reload(&staged, &source, &old_desc, &replacement) == NO_ERROR);
    assert(pool.free_count == 0 && replacement.instance_refs == 2);
    assert(staged.effects->data->id == 11 && staged.effects->next->data->id == 19);
    kest_pipeline_discard_staged(&staged);
    kest_pipeline_discard_staged(&source);
    assert(pool.free_count == 4 && old_desc.instance_refs == 0 && replacement.instance_refs == 0);
    kest_effect_allocator = saved_effects;
    kest_free(pool.entries);
    kest_free(pool.buffer);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(pool.mutex);
#endif
}

KEST_TEST(test_sequence_delete_removes_context_and_saved_file)
{
    kest_context saved_context = global_cxt;
    kest_allocator saved_allocator = kest_preset_allocator;
    kest_preset_pool pool;
    assert(kest_preset_pool_init(&pool) == NO_ERROR);
    assert(kest_preset_pool_reserve(&pool, 2) == NO_ERROR);
    kest_preset_pool_init_allocator(&pool, &kest_preset_allocator);
    memset(&global_cxt, 0, sizeof(global_cxt));
    kest_preset *keep = kest_context_add_preset_rp(&global_cxt);
    kest_preset *remove = kest_context_add_preset_rp(&global_cxt);
    assert(keep && remove);
    kest_sequence sequence = {0};
    assert(sequence_append_preset(&sequence, remove) == NO_ERROR);
    char filename[] = "/tmp/kest-delete-XXXXXX";
    int fd = mkstemp(filename);
    assert(fd >= 0 && close(fd) == 0);
    snprintf(remove->fname, sizeof(remove->fname), "%s", filename);
    remove->has_fname = 1;
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(remove->mutex);
    vSemaphoreDelete(remove->pipeline.mutex);
#endif
    assert(kest_sequence_delete_preset(&sequence, remove) == NO_ERROR);
    assert(sequence.presets == NULL);
    assert(global_cxt.presets->data == keep && global_cxt.presets->next == NULL);
    assert(access(filename, F_OK) != 0);
    assert(pool.free_count == 1);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(keep->mutex);
    vSemaphoreDelete(keep->pipeline.mutex);
#endif
    assert(cxt_remove_preset(&global_cxt, keep) == NO_ERROR);
    assert(pool.free_count == 2 && global_cxt.presets == NULL);
    global_cxt = saved_context;
    kest_preset_allocator = saved_allocator;
    kest_free(pool.entries);
    kest_free(pool.buffer);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(pool.mutex);
#endif
}

KEST_TEST(test_context_creation_uses_typed_allocators)
{
    kest_allocator saved_presets = kest_preset_allocator;
    kest_allocator saved_sequences = kest_sequence_allocator;
    kest_preset_pool presets;
    kest_sequence_pool sequences;
    assert(kest_preset_pool_init(&presets) == NO_ERROR);
    assert(kest_sequence_pool_init(&sequences) == NO_ERROR);
    assert(kest_preset_pool_reserve(&presets, 2) == NO_ERROR);
    assert(kest_sequence_pool_reserve(&sequences, 1) == NO_ERROR);
    kest_preset_pool_init_allocator(&presets, &kest_preset_allocator);
    kest_sequence_pool_init_allocator(&sequences, &kest_sequence_allocator);

    for (int pass = 0; pass < 3; pass++)
    {
        kest_context cxt = {0};
        assert(kest_context_add_preset(&cxt) == NO_ERROR);
        kest_preset *second = kest_context_add_preset_rp(&cxt);
        assert(second != NULL);
        assert(cxt.n_presets == 2 && presets.free_count == 0);
        assert(cxt.presets->data == presets.entries);
        assert(second == presets.entries + 1);
        assert(kest_context_add_preset(&cxt) == ERR_ALLOC_FAIL);
        assert(kest_context_add_preset_rp(&cxt) == NULL);
        assert(cxt.n_presets == 2);
        kest_sequence *sequence = kest_context_add_sequence_rp(&cxt);
        assert(sequence == sequences.entries);
        assert(sequence->presets == NULL && sequence->name == NULL);
        assert(kest_context_add_sequence_rp(&cxt) == NULL);
        free_sequence(sequence);
        assert(sequences.free_count == 1);
        kest_free(cxt.sequences);
        for (kest_preset_pll *node = cxt.presets, *next; node; node = next)
        {
#ifdef KEST_USE_FREERTOS
            vSemaphoreDelete(node->data->mutex);
            vSemaphoreDelete(node->data->pipeline.mutex);
#endif
            next = node->next;
            free_preset(node->data);
            kest_free(node);
        }
        assert(presets.free_count == 2);
    }
    kest_preset_allocator = saved_presets;
    kest_sequence_allocator = saved_sequences;
    kest_free(presets.entries);
    kest_free(presets.buffer);
    kest_free(sequences.entries);
    kest_free(sequences.buffer);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(presets.mutex);
    vSemaphoreDelete(sequences.mutex);
#endif
}

KEST_TEST(test_effect_add_members_uses_typed_allocators)
{
    kest_allocator saved_parameters = kest_parameter_allocator;
    kest_allocator saved_settings = kest_setting_allocator;
    kest_parameter_pool parameters;
    kest_setting_pool settings;
    assert(kest_parameter_pool_init(&parameters) == NO_ERROR);
    assert(kest_setting_pool_init(&settings) == NO_ERROR);
    assert(kest_parameter_pool_reserve(&parameters, 1) == NO_ERROR);
    assert(kest_setting_pool_reserve(&settings, 1) == NO_ERROR);
    kest_parameter_pool_init_allocator(&parameters, &kest_parameter_allocator);
    kest_setting_pool_init_allocator(&settings, &kest_setting_allocator);
    for (int pass = 0; pass < 3; pass++)
    {
        kest_effect effect = {0};
        kest_parameter *param = effect_add_parameter(&effect);
        kest_setting *setting = effect_add_setting(&effect);
        assert(param == parameters.entries && setting == settings.entries);
        assert(param->pw == NULL && param->driver_index == KEST_PARAMETER_UNDRIVEN);
        assert(setting->options == NULL);
        assert(effect_add_parameter(&effect) == NULL);
        assert(effect_add_setting(&effect) == NULL);
        assert(effect.parameters->next == NULL && effect.settings->next == NULL);
        kest_parameter_pll_destroy(effect.parameters, kest_parameter_free);
        kest_setting_pll_destroy(effect.settings, kest_setting_free);
        assert(parameters.free_count == 1 && settings.free_count == 1);
    }
    kest_parameter_allocator = saved_parameters;
    kest_setting_allocator = saved_settings;
    kest_free(parameters.entries);
    kest_free(parameters.buffer);
    kest_free(settings.entries);
    kest_free(settings.buffer);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(parameters.mutex);
    vSemaphoreDelete(settings.mutex);
#endif
}
