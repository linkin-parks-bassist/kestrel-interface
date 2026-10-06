#include "kest_test.h"

KEST_TEST(test_tempo_settings_rebuild_delay_allocations_from_current_instance_values)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/tempo-delay.eff");
    assert(desc);
    kest_preset preset = {0};
    kest_effect *effect = kest_pipeline_append_effect_eff(&preset.pipeline, desc);
    assert(effect);
    kest_setting *tempo = NULL, *division = NULL;
    for (kest_setting_pll *s = effect->settings; s; s = s->next)
    {
        if (strcmp(s->data->name_internal, "tempo") == 0) tempo = s->data;
        if (strcmp(s->data->name_internal, "division") == 0) division = s->data;
    }
    assert(tempo && division);
    kest_updater_state state;
    assert(kest_updater_state_init(&state) == NO_ERROR);
    const int values[][4] = {{120, 24, 22050, 22056}, {90, 24, 29400, 29408},
        {60, 18, 33075, 33080}, {30, 24, 88200, 88208}, {120, 24, 22050, 22056}};
    for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); i++)
    {
        tempo->value = values[i][0];
        division->value = values[i][1];
        assert(kest_scope_fetch(effect->scope, "tempo")->val.setting == tempo);
        kest_update update = { .type = KEST_UPDATE_PRESET, .data.preset = &preset };
        assert(kest_updater_handle_update(&state, update) == NO_ERROR);
        assert(state.state == KEST_UPDATER_STATE_REPROGRAM && state.allocs.count == 1);
        assert(state.allocs.entries[0].size_2 == values[i][2]);
        assert(state.allocs.entries[0].size_1 == values[i][3]);
        assert(kest_updater_generate_command_list(&state) == NO_ERROR);
        /* The real updater leaves REPROGRAM after submitting the batch. */
        state.state = KEST_UPDATER_STATE_READY;
    }
    assert(desc->settings->data->value == 120 && desc->settings->next->data->value == 24);
    kest_updater_state_destroy(&state);
    kest_pipeline_discard_staged(&preset.pipeline);
    kest_effect_desc_retire(desc);
}

KEST_TEST(test_live_polynomial_coefficients_share_one_commit)
{
    kest_effect_desc *desc = kest_read_eff_desc_from_file("tests/fixtures/polynomial-live.eff");
    assert(desc);
    kest_pipeline pipeline = {0};
    kest_effect *effect = kest_pipeline_append_effect_eff(&pipeline, desc);
    assert(effect && effect->parameters);
    kest_parameter *param = effect->parameters->data;
    const float values[] = {0.125f, -0.25f, 0.5f};
    for (int pass = 0; pass < 3; pass++)
    {
        kest_updater_state state;
        assert(kest_updater_state_init(&state) == NO_ERROR);
        assert(kest_parameter_set(param, values[pass]) == NO_ERROR);
        kest_update update = { .type = KEST_UPDATE_PARAM, .data.param = param };
        assert(kest_updater_handle_update(&state, update) == NO_ERROR);
        assert(state.filter_writes.count == 3);
        assert(state.reg_writes.count == 0 && state.instr_writes.count == 0 && state.allocs.count == 0);
        assert(kest_updater_generate_command_list(&state) == NO_ERROR);
        assert(state.cmds.count == 4);
        int seen = 0;
        for (int i = 0; i < 3; i++)
        {
            kest_fpga_command *cmd = &state.cmds.entries[i];
            assert(cmd->type == COMMAND_UPDATE_FILTER_COEF && cmd->data_1.handle == 0);
            assert(cmd->data_2.coef >= 0 && cmd->data_2.coef <= 2);
            seen |= 1 << cmd->data_2.coef;
            assert(cmd->val == (cmd->data_2.coef == 0 ? values[pass] : cmd->data_2.coef == 1 ? 0.25f : -values[pass]));
        }
        assert(seen == 7);
        assert(state.cmds.entries[3].type == COMMAND_COMMIT_FILTER_COEF);
        assert(state.cmds.entries[3].data_1.handle == 0);
        kest_updater_state_destroy(&state);
    }
    kest_effect_free_retired(effect);
    kest_free(pipeline.effects);
}

KEST_TEST(test_resource_clone_for_effect_failure)
{
    kest_effect effect = {0};
    assert(kest_dsp_resource_make_clone_for_effect(NULL, &effect) == NULL);
    kest_dsp_resource filter = { .type = KEST_DSP_RESOURCE_FILTER };
    assert(kest_dsp_resource_make_clone_for_effect(&filter, &effect) == NULL);
}
#include <pthread.h>

static void *sample_stream(void *data)
{
    for (int i = 0; i < 50000; i++)
        kest_mem_slot_read_cb(data, (i & 1) ? 32767 : 32768);
    return NULL;
}

KEST_TEST(kest_test_readback_signed_samples_and_errors)
{
    kest_mem_slot mem = {0};
    assert(atomic_is_lock_free(&mem.value));
    const uint16_t samples[] = {0, 32767, 32768, 65535};
    for (int i = 0; i < 4; i++)
    {
        mem.updated = 0;
        kest_mem_slot_read_cb(&mem, samples[i]);
        assert(mem.value == (int16_t)samples[i]);
        assert(mem.updated == 1);
    }
    mem.updated = 0;
    for (int64_t error = -1; error >= -6; error--)
        kest_mem_slot_read_cb(&mem, error);
    kest_mem_slot_read_cb(NULL, 42);
    assert(mem.value == -1 && mem.updated == 0);

    mem.value = 0;
    pthread_t writer;
    assert(pthread_create(&writer, NULL, sample_stream, &mem) == 0);
    for (int i = 0; i < 50000; i++)
    {
        int sample = mem.value;
        assert(sample == 0 || sample == 32767 || sample == -32768);
    }
    assert(pthread_join(writer, NULL) == 0);
    assert(mem.value == 32767);
}

KEST_TEST(kest_test_readback_latest_sample_coalesces_arrivals)
{
    kest_updater_state state;
    assert(kest_updater_state_init(&state) == NO_ERROR);
    kest_scope scope;
    assert(kest_scope_init(&scope) == NO_ERROR);
    kest_effect effect = { .scope = &scope, .alive = 1 };
    kest_mem_slot mem = { .read_enable = 1, .read_period_ms = 10 };
    kest_dsp_resource res = {
        .type = KEST_DSP_RESOURCE_MEM, .name = "meter", .data = &mem, .effect = &effect
    };
    assert(kest_scope_add_mem(&scope, res.name, &mem) == NO_ERROR);
    assert(kest_dsp_resource_ptr_list_append(&state.resources, &res) == NO_ERROR);
    kest_mem_slot_read_cb(&mem, 42);
    kest_mem_slot_read_cb(&mem, 123);
    assert(mem.updated == 1 && mem.value == 123);
    // A failed next request must not erase or suppress an arrival already on hand.
    assert(kest_updater_handle_resource_updates(&state) == ERR_QUEUE_SEND_FAILED);
    assert(mem.updated == 0 && mem.value == 123);
    assert(state.updates.count == 0);
    kest_scope_entry_dict_destroy(&scope.dict, NULL);
    kest_updater_state_destroy(&state);
}

KEST_TEST(kest_test_readback_best_effort_cadence_and_disabled_reads)
{
    assert(kest_fpga_queue_mem_read(0, NULL, NULL) == ERR_QUEUE_SEND_FAILED);
    kest_updater_state state;
    assert(kest_updater_state_init(&state) == NO_ERROR);
    kest_effect effect = { .alive = 1 };
    kest_mem_slot mem = { .read_enable = 1, .read_period_ms = 25 };
    kest_dsp_resource res = { .type = KEST_DSP_RESOURCE_MEM, .effect = &effect, .data = &mem };
    assert(kest_dsp_resource_ptr_list_append(&state.resources, &res) == NO_ERROR);
    state.tick_ctr = 1;
    assert(kest_updater_handle_resource_updates(&state) == NO_ERROR);
    state.tick_ctr = 2;
    assert(kest_updater_handle_resource_updates(&state) == NO_ERROR);
    state.tick_ctr = 3;
    assert(kest_updater_handle_resource_updates(&state) == ERR_QUEUE_SEND_FAILED);
    mem.read_enable = 0;
    assert(kest_updater_handle_resource_updates(&state) == NO_ERROR);
    mem.read_enable = 1;
    mem.read_period_ms = 0;
    assert(kest_updater_handle_resource_updates(&state) == NO_ERROR);
    mem.read_period_ms = 7;
    state.tick_ctr = 1;
    assert(kest_updater_handle_resource_updates(&state) == ERR_QUEUE_SEND_FAILED);
    state.state = KEST_UPDATER_STATE_REPROGRAM;
    assert(kest_updater_handle_resource_updates(&state) == NO_ERROR);
    assert(kest_fpga_batch_append(&state.batch, COMMAND_CLEAR_CMD_ERR_FLAG) == NO_ERROR);
    uint8_t *retained = state.batch.buf;
    int epoch = global_cxt.epoch;
    int64_t epoch_start = global_cxt.epoch_start_ms;
    for (int i = 0; i < 3; i++)
    {
        assert(kest_updater_send(&state) == ERR_QUEUE_SEND_FAILED);
        assert(state.state == KEST_UPDATER_STATE_REPROGRAM);
        assert(state.batch.buf == retained && state.batch.len == 1);
        assert(global_cxt.epoch == epoch && global_cxt.epoch_start_ms == epoch_start);
        assert(state.batch.buf[0] == COMMAND_CLEAR_CMD_ERR_FLAG);
    }
    kest_updater_state_destroy(&state);
}

KEST_TEST(kest_test_readback_clone_has_independent_sample_and_arrival)
{
    kest_mem_slot mem = { .value = -32768, .updated = 1, .addr = 3,
        .effective_addr = 11, .read_enable = 1, .read_period_ms = 25 };
    kest_dsp_resource res = { .type = KEST_DSP_RESOURCE_MEM, .data = &mem };
    kest_dsp_resource clone;
    assert(kest_dsp_resource_clone(&clone, &res) == NO_ERROR);
    kest_mem_slot *copy = clone.data;
    assert(copy && copy->value == -32768 && copy->updated == 0);
    assert(copy->addr == 3 && copy->effective_addr == 11);
    assert(copy->read_enable == 1 && copy->read_period_ms == 25);
    kest_mem_slot_read_cb(&mem, 42);
    assert(copy->value == -32768 && copy->updated == 0);
    kest_free(copy);
}

KEST_TEST(test_resource_deletion_ignores_arrivals_and_control_dependencies)
{
    kest_updater_state state;
    assert(kest_updater_state_init(&state) == NO_ERROR);
    kest_mem_slot mem = { .read_enable = 1, .read_period_ms = 10, .value = 42 };
    // A marked payload must not dereference its former owner or scope.
    kest_dsp_resource res = { .type = KEST_DSP_RESOURCE_MEM, .data = &mem };
    assert(kest_dsp_resource_ptr_list_append(&state.resources, &res) == NO_ERROR);
    kest_dsp_resource_mark_for_deletion(&res);
    kest_mem_slot_read_cb(&mem, 123);
    assert(mem.value == 42 && mem.updated == 0);
    assert(kest_updater_handle_resource_updates(&state) == NO_ERROR);
    kest_dsp_resource *clone = kest_dsp_resource_make_clone(&res);
    assert(clone && !kest_dsp_resource_delete_requested(clone));
    kest_dsp_resource_free(clone);
    kest_updater_state_destroy(&state);
}

KEST_TEST(test_retired_effect_survives_reprogram_clear_and_spi_queue_rejection)
{
    kest_updater_state state;
    assert(kest_updater_state_init(&state) == NO_ERROR);
    kest_effect *effect = kest_allocator_alloc(&kest_effect_allocator, sizeof(*effect));
    assert(effect && init_effect(effect) == NO_ERROR);
    kest_mem_slot mem = { .read_enable = 1, .read_period_ms = 10 };
    kest_dsp_resource src = { .type = KEST_DSP_RESOURCE_MEM, .data = &mem };
    kest_dsp_resource *res = kest_dsp_resource_make_clone_for_effect(&src, effect);
    assert(res && kest_dsp_resource_ptr_list_append(&effect->resources, res) == NO_ERROR);
    assert(kest_dsp_resource_ptr_list_append(&state.resources, res) == NO_ERROR);
    free_effect(effect);
    free_effect(effect); // Repeated marking cannot submit the same owner twice.
    assert(!effect->alive && kest_dsp_resource_delete_requested(res));
    kest_updater_collect_retired_effects(&state);
    assert(state.retiring_effects == effect && effect->retire_next == NULL);
    state.state = KEST_UPDATER_STATE_REPROGRAM;
    kest_updater_reap_effects(&state);
    assert(state.retiring_effects == effect && effect->spi_retirement == 0);
    assert(kest_updater_clear(&state) == NO_ERROR);
    assert(state.retiring_effects == effect); // Also survives replacement of active resources.
    state.state = KEST_UPDATER_STATE_READY;
    kest_updater_reap_effects(&state);
    assert(state.retiring_effects == effect && effect->spi_retirement == 0);
    // Model the SPI task's ordered completion; only control may now reclaim it.
    atomic_store(&effect->spi_retirement, 2);
    kest_updater_reap_effects(&state);
    assert(state.retiring_effects == NULL);
    // Final release runs on the UI task, after control/SPI have finished.
    lv_timer_t *refresh = lv_display_get_refr_timer(lv_display_get_default());
    lv_timer_pause(refresh); // Unit harness has no FreeRTOS drawing scheduler.
    lv_timer_handler();
    lv_timer_resume(refresh);
    kest_updater_state_destroy(&state);
}

KEST_TEST(test_resource_clones_use_and_reclaim_typed_pool)
{
    kest_allocator saved = kest_dsp_resource_allocator;
    kest_dsp_resource_pool pool;
    assert(kest_dsp_resource_pool_init(&pool) == NO_ERROR);
    assert(kest_dsp_resource_pool_reserve(&pool, 1) == NO_ERROR);
    kest_dsp_resource_pool_init_allocator(&pool, &kest_dsp_resource_allocator);
    kest_mem_slot mem = {0};
    mem.value = -123;
    kest_dsp_resource src = { .type = KEST_DSP_RESOURCE_MEM, .data = &mem };
    for (int pass = 0; pass < 3; pass++)
    {
        assert(kest_dsp_resource_make_clone(NULL) == NULL);
        assert(pool.free_count == 1);
        kest_dsp_resource invalid = { .type = KEST_DSP_RESOURCE_FILTER };
        assert(kest_dsp_resource_make_clone(&invalid) == NULL);
        assert(pool.free_count == 1);
        kest_dsp_resource *clone = kest_dsp_resource_make_clone(&src);
        assert(clone == pool.entries && pool.free_count == 0);
        assert(clone->data != &mem);
        assert(((kest_mem_slot*)clone->data)->value == -123);
        assert(kest_dsp_resource_make_clone(&src) == NULL);
        kest_dsp_resource_free(clone);
        assert(pool.free_count == 1);
    }
    kest_dsp_resource_allocator = saved;
    kest_free(pool.entries);
    kest_free(pool.buffer);
#ifdef KEST_USE_FREERTOS
    vSemaphoreDelete(pool.mutex);
#endif
}
