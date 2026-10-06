#include "kest_test.h"

KEST_TEST(test_info_string_lists_accept_final_item_without_trailing_comma)
{
    for (int pass = 0; pass < 3; pass++)
    {
        kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/string-list-info.eff");
        assert(desc && strcmp(desc->cname, "string_list_syntax") == 0);
        kest_effect_desc_retire(desc);
    }
    assert(kest_eff_parser_reset_mempool() == NO_ERROR);
    char source[] = "v1.0\n.INFO\nname: \"Bad string list\"\nkeywords: {\"one\" \"two\"}\n.CODE\nmov c0 c0\n";
    kest_eff_parsing_state ps = { .content = source, .file_size = strlen(source) };
    assert(kest_tokenize_content(&ps) == NO_ERROR);
    assert(kest_parse_tokens(&ps) != NO_ERROR);
    if (ps.scope)
        kest_scope_entry_dict_destroy(&ps.scope->dict, NULL);
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/string-list-info.eff");
    assert(desc);
    kest_effect_desc_retire(desc);
}

KEST_TEST(test_discovery_metadata_survives_arena_reuse_and_retirement)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/string-list-info.eff");
    assert(desc);
    kest_effect_desc_retain(desc);
    kest_effect_desc_retire(desc);
    assert(kest_eff_parser_reset_mempool() == NO_ERROR);
    kest_effect_desc *other = kest_read_eff_desc_from_file("tests/fixtures/blocks-only.eff");
    assert(other && !other->description && other->keywords.count == 0 &&
        other->instruments.count == 0 && other->types.count == 0 && other->genres.count == 0);
    assert(strcmp(desc->description, "Optional metadata syntax probe.") == 0);
    assert(desc->keywords.count == 3 && strcmp(desc->keywords.entries[2], "three") == 0);
    assert(desc->instruments.count == 1 && strcmp(desc->instruments.entries[0], "bass") == 0);
    assert(desc->types.count == 2 && strcmp(desc->types.entries[1], "modulation") == 0);
    assert(desc->genres.count == 2 && strcmp(desc->genres.entries[1], "noise") == 0);
    kest_effect_desc_retire(other);
    kest_effect_desc_release(desc);
}

KEST_TEST(test_discovery_metadata_preserves_full_text_order_duplicates_and_empty_lists)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/discovery-edge-info.eff");
    assert(desc && strlen(desc->description) == 265);
    assert(strcmp(desc->description + 212, "A long description keeps the authored detail intact. ") == 0);
    assert(desc->keywords.count == 3);
    assert(strcmp(desc->keywords.entries[0], "Bass") == 0);
    assert(strcmp(desc->keywords.entries[1], "bass") == 0);
    assert(strcmp(desc->keywords.entries[2], "Bass") == 0);
    assert(desc->instruments.count == 0 && desc->types.count == 0 && desc->genres.count == 0);
    kest_effect_desc_retire(desc);
}

KEST_TEST(test_discovery_metadata_rejects_wrong_types_and_recovers)
{
    const char *fields[] = { "description", "keywords", "instruments", "types", "genres" };
    const char *bad[] = { "1", "(nested: \"wrong\")", "{\"valid\", 1}", "{\"valid\", {\"nested\"}}" };
    for (size_t i = 0; i < sizeof(fields) / sizeof(*fields); i++)
    for (size_t j = 0; j < sizeof(bad) / sizeof(*bad); j++)
    {
        assert(kest_eff_parser_reset_mempool() == NO_ERROR);
        char source[512];
        snprintf(source, sizeof(source), "v1.0\n.INFO\nname: \"Bad metadata\"\n%s: %s\n.CODE\nmov c0 c0\n", fields[i], bad[j]);
        kest_eff_parsing_state ps = { .content = source, .file_size = strlen(source) };
        assert(kest_tokenize_content(&ps) == NO_ERROR);
        assert(kest_parse_tokens(&ps) == ERR_BAD_ARGS && ps.errors);
        for (kest_ast_node *node = ps.ast->child; node; node = node->next)
            assert(((kest_eff_desc_file_section *)node->data)->dict_.count == 0);
    }
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/string-list-info.eff");
    assert(desc && desc->genres.count == 2);
    kest_effect_desc_retire(desc);
}

KEST_TEST(test_descriptor_resource_templates_reclaim_typed_pool)
{
    kest_allocator saved = kest_dsp_resource_allocator;
    kest_dsp_resource_pool pool;
    assert(kest_dsp_resource_pool_init(&pool) == NO_ERROR);
    assert(kest_dsp_resource_pool_reserve(&pool, 1) == NO_ERROR);
    kest_dsp_resource_pool_init_allocator(&pool, &kest_dsp_resource_allocator);
    for (int pass = 0; pass < 3; pass++)
    {
        kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/readback.eff");
        assert(desc && desc->resources->data == pool.entries && pool.free_count == 0);
        assert(kest_read_eff_desc_from_file("tests/fixtures/readback.eff") == NULL);
        assert(pool.free_count == 0);
        kest_effect_desc_retain(desc);
        kest_effect_desc_retire(desc);
        assert(pool.free_count == 0);
        kest_effect_desc_release(desc);
        assert(pool.free_count == 1);
        assert(kest_read_eff_desc_from_file("tests/fixtures/rejected-resource-code.eff") == NULL);
        assert(pool.free_count == 1);
        assert(kest_read_eff_desc_from_file("tests/fixtures/polynomial-state.eff") == NULL);
        assert(pool.free_count == 1);
    }
    kest_dsp_resource_allocator = saved;
    kest_free(pool.entries);
    kest_free(pool.buffer);
}

KEST_TEST(test_descriptor_setting_templates_reclaim_typed_pool)
{
    kest_allocator saved = kest_setting_allocator;
    kest_setting_pool pool;
    assert(kest_setting_pool_init(&pool) == NO_ERROR);
    assert(kest_setting_pool_reserve(&pool, 1) == NO_ERROR);
    kest_setting_pool_init_allocator(&pool, &kest_setting_allocator);
    for (int pass = 0; pass < 3; pass++)
    {
        kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/enum-settings.eff");
        assert(desc && desc->settings->data == pool.entries && pool.free_count == 0);
        assert(kest_read_eff_desc_from_file("tests/fixtures/enum-settings.eff") == NULL);
        assert(pool.free_count == 0);
        kest_effect_desc_retain(desc);
        kest_effect_desc_retire(desc);
        assert(kest_eff_parser_reset_mempool() == NO_ERROR);
        assert(strcmp(desc->settings->data->options[0].name, "Dotted eighth") == 0);
        assert(pool.free_count == 0);
        kest_effect_desc_release(desc);
        assert(pool.free_count == 1);
        assert(kest_read_eff_desc_from_file("tests/fixtures/rejected-settings-code.eff") == NULL);
        assert(pool.free_count == 1);
    }
    kest_setting_allocator = saved;
    kest_free(pool.entries);
    kest_free(pool.buffer);
}

KEST_TEST(test_descriptor_parameter_templates_reclaim_typed_pool)
{
    kest_allocator saved = kest_parameter_allocator;
    kest_parameter_pool pool;
    assert(kest_parameter_pool_init(&pool) == NO_ERROR);
    assert(kest_parameter_pool_reserve(&pool, 1) == NO_ERROR);
    kest_parameter_pool_init_allocator(&pool, &kest_parameter_allocator);
    for (int pass = 0; pass < 3; pass++)
    {
        kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/readback.eff");
        assert(desc && desc->parameters->data == pool.entries && pool.free_count == 0);
        assert(kest_read_eff_desc_from_file("tests/fixtures/readback.eff") == NULL);
        assert(pool.free_count == 0);
        kest_effect_desc_retain(desc);
        kest_effect_desc_retire(desc);
        assert(pool.free_count == 0);
        kest_effect_desc_release(desc);
        assert(pool.free_count == 1);
        assert(kest_read_eff_desc_from_file("tests/fixtures/rejected-parameter-code.eff") == NULL);
        assert(pool.free_count == 1);
    }
    kest_parameter_allocator = saved;
    kest_free(pool.entries);
    kest_free(pool.buffer);
}

KEST_TEST(test_descriptor_enum_labels_survive_parser_reset_and_retirement)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/enum-settings.eff");
    assert(desc && desc->settings);
    kest_setting *source = desc->settings->data;
    assert(source->type == EFFECT_SETTING_ENUM && source->n_options == 2);
    assert(source->options[0].value == 9 && source->options[1].value == 4);
    kest_pipeline pipeline = {0};
    kest_effect *effect = kest_pipeline_append_effect_eff(&pipeline, desc);
    assert(effect && effect->settings);
    kest_setting *setting = effect->settings->data;
    assert(setting->type == EFFECT_SETTING_ENUM && setting->options != source->options);
    kest_effect_desc_retire(desc);
    assert(kest_eff_parser_reset_mempool() == NO_ERROR);
    assert(strcmp(setting->options[0].name, "Dotted eighth") == 0);
    assert(strcmp(setting->options[1].name, "Quarter") == 0);
    lv_obj_t *screen = lv_obj_create(NULL);
    kest_setting_widget widget;
    assert(nullify_setting_widget(&widget) == NO_ERROR);
    assert(configure_setting_widget(&widget, setting, NULL, NULL) == NO_ERROR);
    assert(setting_widget_create_ui_no_callback(&widget, screen) == NO_ERROR);
    assert(lv_dropdown_get_selected(widget.obj) == 1);
    lv_dropdown_set_selected(widget.obj, 0);
    int16_t selected = -1;
    assert(setting_widget_calc_value(&widget, &selected) == NO_ERROR && selected == 9);
    lv_obj_delete(screen);
    kest_pipeline_discard_staged(&pipeline);
}

KEST_TEST(test_descriptor_enum_rejects_invalid_choices_and_recovers)
{
    const char *choices[] = {
        "", "a: 1", "a: (name: \"A\", value: 0.5)",
        "a: (name: \"A\", value: 3)", "a: (name: \"A\", value: 1)",
        "a: (name: \"A\", value: 0), b: (name: \"B\", value: 0)",
        "a: (name: \"A\", value: 0), b: (name: \"A\", value: 1)",
        "a: (name: \"\", value: 0)", "a: (name: \"A\", value: 0, typo: 1)"
    };
    for (int pass = 0; pass < 3; pass++)
    for (size_t i = 0; i < sizeof(choices)/sizeof(choices[0]); i++)
    {
        assert(kest_eff_parser_reset_mempool() == NO_ERROR);
        char source[1024];
        snprintf(source, sizeof(source), "v1.0\n.INFO\nname: \"Invalid enum\"\n.SETTINGS\nmode: (name: \"Mode\", type: \"enum\", default: 0, min: 0, max: 2, options: (%s))\n.CODE\n", choices[i]);
        kest_eff_parsing_state ps = { .content = source, .file_size = strlen(source) };
        assert(kest_tokenize_content(&ps) == NO_ERROR);
        assert(kest_parse_tokens(&ps) != NO_ERROR && ps.settings == NULL);
        kest_scope_entry_dict_destroy(&ps.scope->dict, NULL);
        kest_effect_desc *valid = kest_read_eff_desc_from_file("tests/fixtures/enum-settings.eff");
        assert(valid);
        kest_effect_desc_retire(valid);
    }
}

KEST_TEST(test_setting_numeric_fields_reject_fractional_and_out_of_range_values)
{
    const char *fields[] = {
        "default: 0.5, min: 0, max: 2", "default: 0, min: 0.5, max: 2",
        "default: 0, min: 0, max: 2.5", "default: 2147483648, min: 0, max: 2",
        "default: 0, min: -4294967296, max: 2", "default: 0, min: 0, max: 4294967296",
        "default: 3, min: 0, max: 2", "default: 0, min: 3, max: 2"
    };
    for (size_t i = 0; i < sizeof(fields)/sizeof(fields[0]); i++)
    {
        assert(kest_eff_parser_reset_mempool() == NO_ERROR);
        char source[512];
        snprintf(source, sizeof(source), "v1.0\n.INFO\nname: \"Invalid setting\"\n.SETTINGS\nmode: (name: \"Mode\", type: \"int\", %s)\n.CODE\n", fields[i]);
        kest_eff_parsing_state ps = { .content = source, .file_size = strlen(source) };
        assert(kest_tokenize_content(&ps) == NO_ERROR);
        int result = kest_parse_tokens(&ps);
        if (result == NO_ERROR || ps.settings != NULL)
            fprintf(stderr, "Accepted invalid setting fields: %s\n", fields[i]);
        assert(result != NO_ERROR && ps.settings == NULL);
        kest_scope_entry_dict_destroy(&ps.scope->dict, NULL);
    }
}

KEST_TEST(kest_test_setting_section_consumes_fresh_values)
{
	const char *tails[] = { "", ", bogus: named + 2", ", group: 0.5" };
	kest_allocator saved = kest_expression_allocator;
	kest_expression_pool pool;
	assert(kest_expression_pool_init(&pool) == NO_ERROR);
	assert(kest_expression_pool_reserve(&pool, 32) == NO_ERROR);
	kest_expression_pool_init_allocator(&pool, &kest_expression_allocator);
	kest_expression *held = kest_expression_pool_obtain(&pool);
	assert(held && kest_expr_init_const(held, 42) == NO_ERROR);
	assert(kest_eff_parser_init_mempool() == NO_ERROR);
	for (int pass = 0; pass < 3; pass++)
	for (int c = 0; c < 3; c++)
	{
		assert(kest_eff_parser_reset_mempool() == NO_ERROR);
		char source[512];
		snprintf(source, sizeof(source), "v1.0\n.INFO\nname: \"probe\"\n.SETTINGS\nmode: (name: \"Mode\", default: 1, min: 0, max: 2, type: \"int\", units: \"steps\"%s)\n.CODE\n", tails[c]);
		kest_eff_parsing_state ps = { .content = source, .file_size = strlen(source) };
		assert(kest_tokenize_content(&ps) == NO_ERROR);
		assert(kest_parse_tokens(&ps) == (c ? ERR_BAD_ARGS : NO_ERROR));
		assert(pool.free_count == 31 && kest_expression_evaluate(held, NULL) == 42);
		kest_scope_entry_dict_destroy(&ps.scope->dict, NULL);
		assert(kest_eff_parser_reset_mempool() == NO_ERROR);
		if (!c)
		{
			assert(ps.settings && ps.settings->data);
			kest_setting *setting = ps.settings->data;
			assert(setting->value == 1 && setting->min == 0 && setting->max == 2);
			assert(strcmp(setting->name, "Mode") == 0 && strcmp(setting->name_internal, "mode") == 0);
			assert(strcmp(setting->units, "steps") == 0);
			kest_free(setting->name); kest_free(setting->name_internal); kest_free(setting->units);
			kest_setting_pll_free(ps.settings);
		}
		else assert(ps.settings == NULL);
	}
	assert(kest_expression_pool_return(&pool, held) == NO_ERROR);
	kest_expression_allocator = saved;
	kest_free(pool.entries); kest_free(pool.buffer);
#ifdef KEST_USE_FREERTOS
	vSemaphoreDelete(pool.mutex);
#endif
}


KEST_TEST(kest_test_rejected_parameter_retires_new_driver_borrows)
{
	const char *tails[] = { "bogus: named + 2", "scale: \"invalid\"", "group: 0.5" };
	kest_allocator saved = kest_expression_allocator;
	kest_expression_pool pool;
	assert(kest_expression_pool_init(&pool) == NO_ERROR);
	assert(kest_expression_pool_reserve(&pool, 64) == NO_ERROR);
	kest_expression_pool_init_allocator(&pool, &kest_expression_allocator);
	kest_expression *held = kest_expression_pool_obtain(&pool);
	assert(held && kest_expr_init_const(held, 42) == NO_ERROR);
	assert(kest_eff_parser_init_mempool() == NO_ERROR);
	for (int pass = 0; pass < 3; pass++)
	for (int c = 0; c < 3; c++)
	for (int seed = 0; seed < 2; seed++)
	{
		assert(kest_eff_parser_reset_mempool() == NO_ERROR);
		char source[512];
		snprintf(source, sizeof(source), "v1.0\n.INFO\nname: \"probe\"\n.PARAMETERS\nlevel: (name: \"Level\", default: 0, min: 0, max: 1, driver: named, %s)\nother: (name: \"Other\", default: 0, min: 0, max: 1, driver: other)\n.CODE\n", tails[c]);
		kest_eff_parsing_state ps = { .content = source, .file_size = strlen(source) };
		kest_driver prior = {0};
		if (seed)
		{
			assert(kest_driver_init_scope_entry(&prior, "prior") == NO_ERROR);
			assert(kest_driver_list_append(&ps.drivers, prior) == NO_ERROR);
		}
		assert(kest_tokenize_content(&ps) == NO_ERROR);
		assert(kest_parse_tokens(&ps) == ERR_BAD_ARGS);
		assert(ps.drivers.count == (size_t)seed);
		if (seed)
		{
			assert(ps.drivers.entries[0].data == prior.data);
			assert(strcmp(((kest_driver_scope_entry *)prior.data)->key, "prior") == 0);
		}
		else assert(ps.drivers.entries == NULL && ps.drivers.capacity == 0);
		assert(pool.free_count == 63 && kest_expression_evaluate(held, NULL) == 42);
		assert(ps.parameters == NULL);
		kest_scope_entry_dict_destroy(&ps.scope->dict, NULL);
		kest_free(prior.data);
		kest_driver_list_destroy(&ps.drivers);
	}
	assert(kest_expression_pool_return(&pool, held) == NO_ERROR);
	kest_expression_allocator = saved;
	kest_free(pool.entries); kest_free(pool.buffer);
#ifdef KEST_USE_FREERTOS
	vSemaphoreDelete(pool.mutex);
#endif
}


KEST_TEST(kest_test_parameter_attribute_guards_stop_extraction)
{
	kest_expression zero = {0}, one = {0}, half = {0}, ref = {0};
	assert(kest_expr_init_const(&zero, 0) == NO_ERROR);
	assert(kest_expr_init_const(&one, 1) == NO_ERROR);
	assert(kest_expr_init_const(&half, 0.5) == NO_ERROR);
	ref.type = KEST_EXPR_REF;
	ref.val.ref_name = "borrowed";
	kest_eff_entry entries[] = {
		{ .name = "name", .type = KEST_EFF_ENTRY_TYPE_STR, .value.val_string = "probe" },
		{ .name = "default", .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &zero },
		{ .name = "min", .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &zero },
		{ .name = "max", .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &one },
		{ .name = "group", .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &zero },
		{ .name = "driver", .type = KEST_EFF_ENTRY_TYPE_EXPR, .value.val_expr = &ref }
	};
	kest_eff_entry_dict dict = { .entries = entries, .count = 6, .capacity = 6 };
	for (int pass = 0; pass < 3; pass++)
	for (int c = 0; c < 4; c++)
	{
		entries[0].type = c == 1 ? KEST_EFF_ENTRY_TYPE_EXPR : KEST_EFF_ENTRY_TYPE_STR;
		entries[0].value.val_string = "probe";
		if (c == 1) entries[0].value.val_expr = &ref;
		entries[1].value.val_expr = c == 2 ? &ref : &zero;
		entries[4].value.val_expr = c == 3 ? &half : &zero;
		entries[5].type = c == 0 ? KEST_EFF_ENTRY_TYPE_STR : KEST_EFF_ENTRY_TYPE_EXPR;
		kest_eff_parsing_state ps = {0};
		assert(kest_extract_parameter(&ps, &dict, "probe") == NULL);
		assert(ps.errors == 1);
		assert(ps.drivers.count == 0);
		assert(ref.type == KEST_EXPR_REF && strcmp(ref.val.ref_name, "borrowed") == 0);
	}
}


KEST_TEST(kest_test_pre_extraction_failure_reclaims_all_sections)
{
	const char *sources[] = {
		"v1.0\n.INFO\nname: \"probe\", extra: {named, (child: 1 + 2)}\n.DEFS\nfirst: named + 3\n.PARAMETERS\nbad: max(1,)\n.CODE\n",
		"v1.0\n.INFO\nname: named + 1\n.DEFS\nfirst: {named, 2 + 3}\n.CODE\n",
		"v1.0\n.INFO\nname: \"probe\", cname: named + 1\n.DEFS\nfirst: {named, 2 + 3}\n.CODE\n",
		"v1.0\n.DEFS\nfirst: {named, 2 + 3}\n.CODE\n"
	};
	kest_allocator saved = kest_expression_allocator;
	kest_expression_pool pool;
	assert(kest_expression_pool_init(&pool) == NO_ERROR);
	assert(kest_expression_pool_reserve(&pool, 32) == NO_ERROR);
	kest_expression_pool_init_allocator(&pool, &kest_expression_allocator);
	kest_expression *held = kest_expression_pool_obtain(&pool);
	assert(held && kest_expr_init_const(held, 42) == NO_ERROR);
	assert(kest_eff_parser_init_mempool() == NO_ERROR);
	for (int pass = 0; pass < 3; pass++)
		for (int c = 0; c < 4; c++)
		{
			assert(kest_eff_parser_reset_mempool() == NO_ERROR);
			kest_eff_parsing_state ps = { .content = (char *)sources[c], .file_size = strlen(sources[c]) };
			assert(kest_tokenize_content(&ps) == NO_ERROR);
			assert(kest_parse_tokens(&ps) == ERR_BAD_ARGS);
			assert(ps.ast);
			for (kest_ast_node *node = ps.ast->child; node; node = node->next)
				assert(((kest_eff_desc_file_section *)node->data)->dict_.count == 0);
			assert(pool.free_count == 31);
			assert(kest_expression_evaluate(held, NULL) == 42);
		}
	assert(kest_expression_pool_return(&pool, held) == NO_ERROR);
	kest_expression_allocator = saved;
	kest_free(pool.entries); kest_free(pool.buffer);
#ifdef KEST_USE_FREERTOS
	vSemaphoreDelete(pool.mutex);
#endif
}


KEST_TEST(kest_test_failed_section_reclaims_inserted_values)
{
	const char *tails[][12] = {
		{ "bad", ":", "max", "(", "1", ",", ")", NULL },
		{ "first", ":", "2", NULL },
		{ "bad", "wrong", NULL }
	};
	kest_allocator saved = kest_expression_allocator;
	kest_expression_pool pool;
	assert(kest_expression_pool_init(&pool) == NO_ERROR);
	assert(kest_expression_pool_reserve(&pool, 32) == NO_ERROR);
	kest_expression_pool_init_allocator(&pool, &kest_expression_allocator);
	kest_expression *held = kest_expression_pool_obtain(&pool);
	assert(held && kest_expr_init_const(held, 42) == NO_ERROR);
	assert(kest_eff_parser_init_mempool() == NO_ERROR);
	for (int pass = 0; pass < 3; pass++)
		for (int c = 0; c < 3; c++)
		{
			assert(kest_eff_parser_reset_mempool() == NO_ERROR);
			const char *words[48] = { "\n", "first", ":", "{", "named", ",", "(", "child", ":", "1", "+", "2", ")", "}", "," };
			int n = 15;
			for (int i = 0; tails[c][i]; i++) words[n++] = tails[c][i];
			kest_token_ll tokens[48] = {0};
			for (int i = 0; i < n; i++)
			{
				tokens[i].data = (char *)words[i];
				tokens[i].next = i + 1 < n ? &tokens[i + 1] : NULL;
			}
			kest_eff_desc_file_section sec = { .tokens = tokens };
			kest_ast_node node = { .type = KEST_AST_NODE_SECTION, .data = &sec };
			kest_eff_parsing_state ps = {0};
			assert(kest_parse_entry_section(&ps, &node) == (c == 1 ? ERR_DUPLICATE_KEY : ERR_BAD_ARGS));
			assert(sec.dict_.count == 0);
			assert(pool.free_count == 31);
			assert(kest_expression_evaluate(held, NULL) == 42);
		}
	assert(kest_expression_pool_return(&pool, held) == NO_ERROR);
	kest_expression_allocator = saved;
	kest_free(pool.entries); kest_free(pool.buffer);
#ifdef KEST_USE_FREERTOS
	vSemaphoreDelete(pool.mutex);
#endif
}


KEST_TEST(kest_test_partial_container_failures_reclaim_fresh_expressions)
{
	const char *cases[][24] = {
		{ "(", "first", ":", "named", ",", "bad", ":", "max", "(", "1", ",", ")", ")", NULL },
		{ "{", "named", ",", "max", "(", "1", ",", ")", "}", NULL },
		{ "(", "first", ":", "named", NULL },
		{ "{", "named", NULL }
	};
	kest_allocator saved = kest_expression_allocator;
	kest_expression_pool pool;
	assert(kest_expression_pool_init(&pool) == NO_ERROR);
	assert(kest_expression_pool_reserve(&pool, 16) == NO_ERROR);
	kest_expression_pool_init_allocator(&pool, &kest_expression_allocator);
	kest_expression *held = kest_expression_pool_obtain(&pool);
	assert(held && kest_expr_init_const(held, 42) == NO_ERROR);
	assert(kest_eff_parser_init_mempool() == NO_ERROR);
	for (int pass = 0; pass < 3; pass++)
		for (int c = 0; c < 4; c++)
		{
			assert(kest_eff_parser_reset_mempool() == NO_ERROR);
			kest_token_ll tokens[24] = {0};
			for (int i = 0; cases[c][i]; i++)
			{
				tokens[i].data = (char *)cases[c][i];
				tokens[i].next = cases[c][i + 1] ? &tokens[i + 1] : NULL;
			}
			kest_eff_parsing_state ps = { .current_token = tokens };
			kest_eff_entry entry;
			memset(&entry, 0xa5, sizeof(entry));
			assert(kest_parse_eff_entry(&ps, &entry) == ERR_BAD_ARGS);
			assert(entry.type == KEST_EFF_ENTRY_TYPE_NOTHING);
			assert(pool.free_count == 15);
			assert(kest_expression_evaluate(held, NULL) == 42);
		}
	assert(kest_expression_pool_return(&pool, held) == NO_ERROR);
	kest_expression_allocator = saved;
	kest_free(pool.entries); kest_free(pool.buffer);
#ifdef KEST_USE_FREERTOS
	vSemaphoreDelete(pool.mutex);
#endif
}

KEST_TEST(kest_test_rejected_dictionary_values_reclaim_fresh_expressions)
{
	const char *values[][24] = {
		{ "max", "(", "named", ",", "2", ")", "+", "3", NULL },
		{ "(", "child", ":", "max", "(", "named", ",", "2", ")", ")", NULL },
		{ "{", "named", ",", "1", "+", "2", ",", "(", "child", ":", "3", ")", "}", NULL }
	};
	kest_allocator saved = kest_expression_allocator;
	kest_expression_pool pool;
	assert(kest_expression_pool_init(&pool) == NO_ERROR);
	assert(kest_expression_pool_reserve(&pool, 16) == NO_ERROR);
	kest_expression_pool_init_allocator(&pool, &kest_expression_allocator);
	kest_expression *held = kest_expression_pool_obtain(&pool);
	assert(held && kest_expr_init_const(held, 42) == NO_ERROR);
	assert(kest_eff_parser_init_mempool() == NO_ERROR);
	for (int pass = 0; pass < 3; pass++)
		for (int c = 0; c < 3; c++)
		{
			assert(kest_eff_parser_reset_mempool() == NO_ERROR);
			const char *words[32] = { "key", ":", "\"first\"", ",", "key", ":" };
			int n = 6;
			for (int i = 0; values[c][i]; i++) words[n++] = values[c][i];
			words[n++] = ")";
			kest_token_ll tokens[32] = {0};
			for (int i = 0; i < n; i++)
			{
				tokens[i].data = (char *)words[i];
				tokens[i].next = i + 1 < n ? &tokens[i + 1] : NULL;
			}
			kest_eff_entry_dict dict;
			assert(kest_eff_entry_dict_init(&dict) == NO_ERROR);
			kest_eff_parsing_state ps = { .current_token = tokens };
			assert(kest_parse_eff_entries(&ps, &dict) == ERR_DUPLICATE_KEY);
			assert(dict.count == 1 && ps.errors == 1);
			assert(pool.free_count == 15);
			assert(kest_expression_evaluate(held, NULL) == 42);
		}
	assert(kest_expression_pool_return(&pool, held) == NO_ERROR);
	kest_expression_allocator = saved;
	kest_free(pool.entries); kest_free(pool.buffer);
#ifdef KEST_USE_FREERTOS
	vSemaphoreDelete(pool.mutex);
#endif
}

KEST_TEST(kest_test_dictionary_duplicate_error_survives_following_entry)
{
	assert(kest_eff_parser_init_mempool() == NO_ERROR);
	for (int pass = 0; pass < 3; pass++)
	{
		assert(kest_eff_parser_reset_mempool() == NO_ERROR);
		char *words[] = { "name", ":", "\"first\"", ",", "name", ":", "\"second\"", ",", "other", ":", "\"third\"", ")" };
		kest_token_ll tokens[12] = {0};
		for (int i = 0; i < 12; i++)
		{
			tokens[i].data = words[i];
			tokens[i].next = i < 11 ? &tokens[i + 1] : NULL;
		}
		kest_eff_entry_dict dict;
		assert(kest_eff_entry_dict_init(&dict) == NO_ERROR);
		kest_eff_parsing_state ps = { .current_token = tokens };
		assert(kest_parse_eff_entries(&ps, &dict) == ERR_DUPLICATE_KEY);
		assert(ps.errors == 1);
		assert(dict.count == 1);
		assert(strcmp(kest_eff_entry_dict_lookup(&dict, "name")->value.val_string, "first") == 0);
		assert(kest_eff_entry_dict_lookup(&dict, "other") == NULL);
	}
}

KEST_TEST(kest_test_resource_type_requires_string_tag_and_value)
{
	kest_eff_entry entry = { .name = "type", .value.val_string = "mem" };
	kest_eff_entry_dict dict = { .entries = &entry, .count = 1, .capacity = 1 };
	int types[] = { KEST_EFF_ENTRY_TYPE_EXPR, KEST_EFF_ENTRY_TYPE_SUBDICT, KEST_EFF_ENTRY_TYPE_LIST };
	for (int i = 0; i < 3; i++)
	{
		kest_eff_parsing_state ps = {0};
		entry.type = types[i];
		assert(kest_extract_resource(&ps, &dict, "probe") == NULL);
		assert(ps.errors == 1);
	}
	entry.type = KEST_EFF_ENTRY_TYPE_STR;
	entry.value.val_string = NULL;
	kest_eff_parsing_state ps = {0};
	assert(kest_extract_resource(&ps, &dict, "probe") == NULL);
	assert(ps.errors == 1);
}

KEST_TEST(kest_test_descriptor_rejects_unnamed_resource_entry)
{
	assert(kest_eff_parser_init_mempool() == NO_ERROR);
	assert(kest_eff_parser_reset_mempool() == NO_ERROR);
	assert(kest_read_eff_desc_from_file("tests/fixtures/unnamed-resource.eff") == NULL);
}

KEST_TEST(kest_test_section_locations_survive_dirty_arena_reuse)
{
	assert(kest_eff_parser_init_mempool() == NO_ERROR);
	for (int pass = 0; pass < 3; pass++)
	{
		assert(kest_eff_parser_reset_mempool() == NO_ERROR);
		void *dirty = kest_parser_alloc(4096);
		assert(dirty);
		memset(dirty, 0xa5, 4096);
		assert(kest_eff_parser_reset_mempool() == NO_ERROR);
		char source[] = "v1.0\n\n.INFO\n\n\n.CODE\n";
		kest_eff_parsing_state ps = {
			.content = source, .file_size = sizeof(source) - 1
		};
		assert(kest_tokenize_content(&ps) == NO_ERROR);
		assert(kest_parse_tokens(&ps) == ERR_BAD_ARGS);
		assert(ps.ast && ps.ast->line == 1);
		assert(ps.ast->child && ps.ast->child->line == 3);
		assert(ps.ast->child->next && ps.ast->child->next->line == 6);
		assert(ps.ast->child->next->next == NULL);
	}
}

KEST_TEST(kest_test_section_extractors_reject_non_dictionary_entries)
{
	kest_eff_entry entry = {
		.name = "invalid", .type = KEST_EFF_ENTRY_TYPE_STR,
		.value.val_string = "not a dictionary"
	};
	kest_eff_desc_file_section section = {0};
	section.dict_.entries = &entry;
	section.dict_.count = section.dict_.capacity = 1;
	kest_ast_node node = { .data = &section };
	kest_eff_parsing_state ps = {0};
	kest_parameter_pll *parameters = NULL;
	kest_setting_pll *settings = NULL;
	kest_dsp_resource_pll *resources = NULL;
	assert(kest_parameters_section_extract(&ps, &parameters, &node) == ERR_BAD_ARGS);
	assert(ps.errors == 1 && parameters == NULL);
	assert(kest_settings_section_extract(&ps, &settings, &node) == ERR_BAD_ARGS);
	assert(ps.errors == 2 && settings == NULL);
	assert(kest_resources_section_extract(&ps, &resources, &node) == ERR_BAD_ARGS);
	assert(ps.errors == 3 && resources == NULL);
}

KEST_TEST(kest_test_failed_pratt_parses_reclaim_unpublished_trees)
{
    const char *cases[][12] = {
        { "min", "(", "1", ")", NULL },
        { "1", "+", NULL },
        { "max", "(", "1", "+", "2", ",", ")", NULL },
        { "(", "1", "+", "2", NULL },
        { "max", "(", "1", ",", "2", ",", "3", ")", NULL },
        { "(", "named", NULL },
        { "(", "max", "(", "named", ",", "2", ")", "+", ")", NULL }
    };
    kest_allocator saved = kest_expression_allocator;
    kest_expression_pool pool;
    assert(kest_expression_pool_init(&pool) == NO_ERROR);
    assert(kest_expression_pool_reserve(&pool, 16) == NO_ERROR);
    kest_expression_pool_init_allocator(&pool, &kest_expression_allocator);
    kest_expression *held = kest_expression_pool_obtain(&pool);
    assert(held && kest_expr_init_const(held, 42) == NO_ERROR);
    for (int pass = 0; pass < 3; pass++)
        for (size_t c = 0; c < sizeof(cases) / sizeof(cases[0]); c++)
        {
            kest_token_ll tokens[12] = {0};
            for (int i = 0; cases[c][i]; i++)
            {
                tokens[i].data = (char *)cases[c][i];
                tokens[i].next = cases[c][i + 1] ? &tokens[i + 1] : NULL;
            }
            kest_eff_parsing_state ps = { .current_token = tokens };
            assert(kest_parse_expression(&ps, tokens, NULL) == NULL);
            assert(pool.free_count == 15);
            assert(kest_expression_evaluate(held, NULL) == 42);
        }
    const char *exhausted[][12] = {
        { "-", "1", NULL },
        { "1", "+", "2", NULL },
        { "max", "(", "1", ",", "2", ")", NULL },
        { "-", "max", "(", "1", ",", "2", ")", NULL }
    };
    for (int c = 0; c < 4; c++)
    {
        int available = c == 0 ? 1 : c == 3 ? 3 : 2;
        kest_expression *reserved[14];
        for (int i = 0; i < 15 - available; i++)
        {
            reserved[i] = kest_expression_pool_obtain(&pool);
            assert(reserved[i]);
        }
        for (int pass = 0; pass < 3; pass++)
        {
            kest_token_ll tokens[12] = {0};
            const char **words = exhausted[c];
            for (int i = 0; words[i]; i++)
            {
                tokens[i].data = (char *)words[i];
                tokens[i].next = words[i + 1] ? &tokens[i + 1] : NULL;
            }
            kest_eff_parsing_state ps = { .current_token = tokens };
            assert(kest_parse_expression(&ps, tokens, NULL) == NULL);
            assert(pool.free_count == available);
            assert(kest_expression_evaluate(held, NULL) == 42);
        }
        for (int i = 0; i < 15 - available; i++)
            assert(kest_expression_pool_return(&pool, reserved[i]) == NO_ERROR);
    }
    assert(kest_expression_pool_return(&pool, held) == NO_ERROR);
    kest_expression_allocator = saved;
    kest_free(pool.entries); kest_free(pool.buffer);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(pool.mutex);
#endif
}

KEST_TEST(kest_test_expression_allocation_failure_preserves_cursor)
{
    kest_allocator saved = kest_expression_allocator;
    kest_expression_pool pool;
    assert(kest_expression_pool_init(&pool) == NO_ERROR);
    assert(kest_expression_pool_reserve(&pool, 1) == NO_ERROR);
    kest_expression_pool_init_allocator(&pool, &kest_expression_allocator);
    kest_expression *held = kest_expression_pool_obtain(&pool);
    assert(held);
    kest_token_ll token = { .data = "0" };
    for (int pass = 0; pass < 3; pass++)
    {
        kest_eff_parsing_state ps = { .current_token = &token };
        assert(kest_parse_expression(&ps, &token, NULL) == NULL);
        assert(ps.current_token == &token && pool.free_count == 0);
    }
    assert(kest_expression_pool_return(&pool, held) == NO_ERROR);
    kest_eff_parsing_state ps = { .current_token = &token };
    kest_expression *expr = kest_parse_expression(&ps, &token, NULL);
    assert(expr && ps.current_token == NULL);
    assert(kest_expression_evaluate(expr, NULL) == 0);
    assert(kest_expression_pool_return(&pool, expr) == NO_ERROR);
    kest_expression_allocator = saved;
    kest_free(pool.entries); kest_free(pool.buffer);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(pool.mutex);
#endif
}

KEST_TEST(kest_test_anonymous_coefficient_list)
{
    assert(kest_eff_parser_init_mempool() == NO_ERROR);
    kest_token_ll tokens[] = {
        { .data = "0" }, { .data = "," }, { .data = "-" },
        { .data = "1" }, { .data = "}" }
    };
    for (int i = 0; i < 4; i++) tokens[i].next = &tokens[i + 1];
    kest_eff_parsing_state ps = { .current_token = tokens };
    kest_eff_entry result = {0};
    assert(kest_parse_eff_list(&ps, &result) == NO_ERROR);
    assert(ps.current_token == &tokens[4]);
    assert(result.value.val_list && result.value.val_list->count == 2);
    assert(kest_expression_evaluate(result.value.val_list->entries[0].value.val_expr, NULL) == 0);
    assert(kest_expression_evaluate(result.value.val_list->entries[1].value.val_expr, NULL) == -1);
}

KEST_TEST(kest_test_descriptor_survives_parser_arena_reuse)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/readback.eff");
    assert(desc);
    kest_pipeline original = {0}, later = {0};
    kest_effect *first = kest_pipeline_append_effect_eff(&original, desc);
    assert(first);
    kest_fpga_transfer_batch before = {0}, after = {0};
    assert(kest_pipeline_create_fpga_transfer_batch(&original, &before) == NO_ERROR);
    assert(before.len > 0);
    assert(kest_eff_parser_reset_mempool() == NO_ERROR);
    assert(kest_read_eff_desc_from_file("tests/fixtures/preset-settings.eff"));
    assert(strcmp(desc->name, "Readback Test") == 0);
    assert(strcmp(first->parameters->data->name_internal, "level") == 0);
    assert(strcmp(first->resources.entries[0]->name, "probe") == 0);
    kest_effect *second = kest_pipeline_append_effect_eff(&later, desc);
    assert(second);
    assert(kest_pipeline_create_fpga_transfer_batch(&later, &after) == NO_ERROR);
    assert(after.len == before.len && memcmp(after.buf, before.buf, before.len) == 0);
    assert(kest_expression_evaluate(second->blocks.entries[0].reg_0.expr, second->scope) == 0.25f);
    kest_free_fpga_transfer_batch(before);
    kest_free_fpga_transfer_batch(after);
    kest_effect_free_retired(first);
    kest_effect_free_retired(second);
    kest_free(original.effects);
    kest_free(later.effects);
}

KEST_TEST(kest_test_readback_effect_descriptor)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/readback.eff");
    assert(desc != NULL);
    assert(strcmp(desc->name, "Readback Test") == 0);
    assert(desc->resources && !desc->resources->next);
    kest_dsp_resource *res = desc->resources->data;
    assert(res && res->type == KEST_DSP_RESOURCE_MEM && res->mem_size == 1);
    assert(strcmp(res->name, "probe") == 0);
    kest_mem_slot *mem = res->data;
    assert(mem && mem->read_enable && mem->read_period_ms == 10);
    assert(atomic_load(&mem->value) == 0);
    assert(desc->blocks && desc->blocks->next && !desc->blocks->next->next);
    kest_block *load = desc->blocks->data;
    assert(load->instr == BLOCK_INSTR_MADD && load->dest == 1);
    assert(load->arg_a.type == BLOCK_OPERAND_TYPE_R && load->arg_a.addr == 0);
    assert(load->reg_0.active && load->reg_0.expr);
    kest_scope *scope = kest_eff_desc_create_scope(desc);
    assert(scope != NULL);
    assert(kest_expression_evaluate(load->reg_0.expr, scope) == 0.25f);
    kest_block *write = desc->blocks->next->data;
    assert(write->instr == BLOCK_INSTR_MEM_WRITE);
    assert(write->res == res);
    assert(write->arg_a.type == BLOCK_OPERAND_TYPE_C && write->arg_a.addr == 1);
}

KEST_TEST(kest_test_parsed_svf_uses_descriptor_send_policy)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/svf.eff");
    assert(desc && desc->blocks && desc->blocks->next);
    kest_scope *scope = kest_eff_desc_create_scope(desc);
    assert(scope);
    kest_block *svf = desc->blocks->data;
    assert(svf->desc == kest_instr_name_to_desc("svf"));
    assert(svf->reg_0.expr->type == KEST_EXPR_MAX);
    assert(svf->reg_0.format.fractional_bits == 15);
    assert(kest_expression_compute_min(svf->reg_0.expr, scope) == 0);
    assert(kest_expression_evaluate(svf->reg_0.expr, scope) == 0.25f);
    assert(svf->reg_1.format.fractional_bits == 13 && svf->shift == 2);
    kest_block *channel_svf = desc->blocks->next->next->data;
    assert(channel_svf->arg_b.type == BLOCK_OPERAND_TYPE_C);
    assert(channel_svf->arg_b.addr == 1 && channel_svf->shift == 2);
}

KEST_TEST(kest_test_three_expression_arguments_are_rejected)
{
    assert(kest_read_eff_desc_from_file("tests/fixtures/too-many-registers.eff") == NULL);
}

KEST_TEST(kest_test_parsed_explicit_shift_fields)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/shifts.eff");
    assert(desc);
    kest_block_pll *node = desc->blocks;
    const char *names[] = {"arsh", "lsh", "rsh"};
    for (int i = 0; i < 3; i++, node = node->next)
    {
        assert(node && node->data->desc == kest_instr_name_to_desc(names[i]));
        assert(node->data->shift == i + 1);
        assert(node->data->reg_0.format.fractional_bits == 15);
    }
    assert(!node);
}

KEST_TEST(kest_test_polynomial_resources_have_programmable_filter_payloads)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/polynomial-state.eff");
    assert(desc && desc->resources && desc->resources->next);
    const int counts[] = {3, 1};
    kest_dsp_resource_pll *node = desc->resources;
    for (int i = 0; i < 2; i++, node = node->next)
    {
        assert(node && node->data->type == KEST_DSP_RESOURCE_FILTER);
        kest_filter *poly = node->data->data;
        assert(poly && poly->feed_forward == counts[i] && poly->feed_back == 0);
        assert(poly->coefs.count == counts[i]);
    }
    assert(!node);
    assert(desc->blocks->data->instr == BLOCK_INSTR_POLY);
}

KEST_TEST(kest_test_malformed_polynomial_reports_error_without_crashing)
{
    assert(kest_read_eff_desc_from_file("tests/fixtures/polynomial-invalid.eff") == NULL);
}

KEST_TEST(kest_test_invalid_channel_reports_error_without_crashing)
{
    for (int attempt = 0; attempt < 3; attempt++)
        assert(kest_read_eff_desc_from_file("tests/fixtures/invalid-channel.eff") == NULL);
}

static void *reject_asm_expression_alloc(void *data, size_t size)
{
    if (data) (*(int *)data)++;
    return NULL;
}

KEST_TEST(kest_test_asm_operand_propagates_expression_allocation_failure)
{
    kest_allocator saved = kest_expression_allocator;
    kest_expression_allocator = (kest_allocator){ .alloc = reject_asm_expression_alloc };
    const char *words[] = { "c0", "12", "$" };
    for (int pass = 0; pass < 3; pass++)
    for (int c = 0; c < 3; c++)
    {
        kest_token_ll name = { .data = "probe" };
        kest_token_ll token = { .data = (char *)words[c], .next = c == 2 ? &name : NULL };
        kest_eff_parsing_state ps = { .current_token = &token };
        kest_asm_arg arg = {0};
        assert(kest_parse_asm_arg_2(&ps, &arg) == ERR_ALLOC_FAIL);
        assert(arg.expr == NULL && ps.errors == 1 && ps.current_token == NULL);
    }
    kest_expression_allocator = saved;
}

KEST_TEST(kest_test_discarded_asm_operand_does_not_allocate)
{
    kest_allocator saved = kest_expression_allocator;
    int allocations = 0;
    kest_expression_allocator = (kest_allocator){ .alloc = reject_asm_expression_alloc, .data = &allocations };
    const char *words[] = { "c0", "12", "$" };
    for (int pass = 0; pass < 3; pass++)
    for (int c = 0; c < 3; c++)
    {
        kest_token_ll name = { .data = "probe" };
        kest_token_ll token = { .data = (char *)words[c], .next = c == 2 ? &name : NULL };
        kest_eff_parsing_state ps = { .current_token = &token };
        assert(kest_parse_asm_arg_2(&ps, NULL) == NO_ERROR);
        assert(ps.current_token == NULL && ps.errors == 0 && allocations == 0);
    }
    kest_expression_allocator = saved;
}
