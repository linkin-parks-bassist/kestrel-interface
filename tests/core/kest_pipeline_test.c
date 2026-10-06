#include "kest_test.h"

extern void add_effect_from_menu_eff(lv_event_t *event);

KEST_TEST(kest_test_sequence_preset_page_parent)
{
    kest_ui_page sequence_page = {0};
    kest_sequence sequence = {.view_page = &sequence_page};
    kest_ui_page page;
    assert(init_ui_page(&page) == NO_ERROR);
    kest_preset preset = {.name = "Parent test", .sequence = &sequence, .view_page = &page};
    kest_preset_view_str view = {.preset = &preset};
    kest_ui_page_panel panel = {0};
    page.data_struct = &view;
    page.panel = &panel;
    page.parent = &sequence_page;
    assert(create_preset_view_ui(&page) == NO_ERROR);
    assert(page.parent == &sequence_page && page.parent != &page);
    lv_obj_delete(page.screen);
}

KEST_TEST(kest_test_activation_rejects_oversized_preset)
{
    kest_effect_desc desc = {.res_rpt = {.blocks = KEST_FPGA_N_BLOCKS + 1}};
    kest_effect effect = {.eff = &desc, .blocks = {.count = KEST_FPGA_N_BLOCKS + 1}};
    kest_effect_pll effect_node = {.data = &effect};
    kest_preset rejected = {.pipeline = {.effects = &effect_node}};
    kest_preset running = {.active = 1};
    kest_preset *saved_preset = global_cxt.active_preset;
    kest_sequence *saved_sequence = global_cxt.sequence;
    kest_sequence previous = {.active = 1};
    global_cxt.active_preset = &running;
    global_cxt.sequence = &previous;
    assert(kest_preset_set_active(&rejected) == ERR_PIPELINE_FULL);
    assert(set_active_preset(&rejected) == ERR_PIPELINE_FULL);
    assert(set_active_preset_from_sequence(&rejected) == ERR_PIPELINE_FULL);
    assert(!rejected.active && !rejected.pending && running.active);
    assert(global_cxt.active_preset == &running && global_cxt.sequence == &previous);

    seq_kest_preset_pll nodes[2] = {{.data = &rejected}, {.data = &running}};
    nodes[0].next = &nodes[1];
    nodes[1].prev = &nodes[0];
    kest_sequence sequence = {.presets = nodes};
    assert(kest_sequence_begin(&sequence) == ERR_PIPELINE_FULL);
    assert(kest_sequence_begin_at(&sequence, &rejected) == ERR_PIPELINE_FULL);
    assert(!sequence.active && !sequence.position);
    sequence.active = 1;
    sequence.position = &nodes[1];
    assert(kest_sequence_regress(&sequence) == ERR_PIPELINE_FULL);
    assert(sequence.position == &nodes[1]);
    nodes[0].data = &running;
    nodes[1].data = &rejected;
    sequence.position = &nodes[0];
    assert(kest_sequence_advance(&sequence) == ERR_PIPELINE_FULL);
    assert(sequence.position == &nodes[0]);
    assert(global_cxt.active_preset == &running && global_cxt.sequence == &previous);
    assert(!rejected.active && !rejected.pending && running.active);
    global_cxt.active_preset = saved_preset;
    global_cxt.sequence = saved_sequence;
}

KEST_TEST(kest_test_selector_rejects_full_preset)
{
    kest_effect_desc existing = {.res_rpt = {.blocks = KEST_FPGA_N_BLOCKS}};
    kest_effect effect = {.eff = &existing, .blocks = {.count = KEST_FPGA_N_BLOCKS}};
    kest_effect_pll node = {.data = &effect};
    kest_preset preset = {.pipeline = {.effects = &node}};
    kest_effect_desc extra = {.res_rpt = {.blocks = 1}};
    kest_effect_selector_button button = {.eff = &extra};
    kest_preset *previous = global_cxt.working_preset;
    global_cxt.working_preset = &preset;
    lv_obj_t *target = lv_button_create(lv_screen_active());
    lv_obj_add_event_cb(target, add_effect_from_menu_eff, LV_EVENT_CLICKED, &button);
    uint32_t overlays = lv_obj_get_child_count(lv_layer_top());
    lv_obj_t *screen = lv_screen_active();
    lv_obj_send_event(target, LV_EVENT_CLICKED, NULL);
    assert(preset.pipeline.effects == &node && !node.next);
    assert(lv_screen_active() == screen);
    assert(lv_obj_get_child_count(lv_layer_top()) == overlays + 1);
    lv_obj_delete(lv_obj_get_child(lv_layer_top(), -1));
    lv_obj_delete(target);
    global_cxt.working_preset = previous;
}

KEST_TEST(kest_test_pipeline_instruction_capacity)
{
    kest_block blocks[KEST_FPGA_N_BLOCKS + 1];
    for (int i = 0; i <= KEST_FPGA_N_BLOCKS; i++)
        assert(kest_init_block(&blocks[i]) == NO_ERROR);

    kest_effect_desc desc[2] = {0};
    kest_effect effects[2] = {0};
    kest_effect_pll nodes[2] = {0};
    for (int i = 0; i < 2; i++)
    {
        desc[i].res_rpt.blocks = KEST_FPGA_N_BLOCKS / 2;
        effects[i].eff = &desc[i];
        effects[i].blocks.entries = blocks;
        effects[i].blocks.count = KEST_FPGA_N_BLOCKS / 2;
        nodes[i].data = &effects[i];
    }
    nodes[0].next = &nodes[1];
    kest_pipeline pipeline = {.effects = nodes};
    assert(kest_pipeline_check_capacity(&pipeline, 0) == NO_ERROR);
    assert(kest_pipeline_check_capacity(&pipeline, 1) == ERR_PIPELINE_FULL);
    kest_preset preset = {.pipeline = pipeline};
    kest_effect_desc extra = {.res_rpt = {.blocks = 1}};
    assert(!kest_preset_append_effect_eff(&preset, &extra));
    assert(preset.pipeline.effects == nodes && nodes[0].next == &nodes[1]);
    kest_fpga_transfer_batch batch = {0};
    assert(kest_pipeline_create_fpga_transfer_batch(&pipeline, &batch) == NO_ERROR);
    assert(batch.len > 0 && effects[1].block_position == KEST_FPGA_N_BLOCKS / 2);
    kest_free_fpga_transfer_batch(batch);

    effects[0].block_position = 17;
    effects[1].block_position = 29;
    effects[0].position_.block_start = 17;
    effects[1].position_.block_start = 29;
    effects[1].blocks.count++;
    desc[1].res_rpt.blocks++;
    assert(kest_pipeline_create_fpga_transfer_batch(&pipeline, &batch) == ERR_PIPELINE_FULL);
    assert(!batch.buf && !batch.len && !batch.buf_len);
    assert(effects[0].block_position == 17 && effects[1].block_position == 29);
    assert(effects[0].position_.block_start == 17 && effects[1].position_.block_start == 29);

    /* Check emitted blocks as well as the descriptor's position increment. */
    desc[1].res_rpt.blocks--;
    assert(kest_pipeline_create_fpga_transfer_batch(&pipeline, &batch) == ERR_PIPELINE_FULL);
    effects[1].blocks.count--;
    desc[1].res_rpt.blocks++;
    assert(kest_pipeline_create_fpga_transfer_batch(&pipeline, &batch) == ERR_PIPELINE_FULL);

    nodes[0].next = NULL;
    effects[0].blocks.count = KEST_FPGA_N_BLOCKS + 1;
    desc[0].res_rpt.blocks = KEST_FPGA_N_BLOCKS + 1;
    assert(kest_pipeline_create_fpga_transfer_batch(&pipeline, &batch) == ERR_PIPELINE_FULL);

    pipeline.effects = NULL;
    assert(kest_pipeline_check_capacity(&pipeline, KEST_FPGA_N_BLOCKS) == NO_ERROR);
    assert(kest_pipeline_check_capacity(&pipeline, KEST_FPGA_N_BLOCKS + 1) == ERR_PIPELINE_FULL);
    assert(kest_pipeline_create_fpga_transfer_batch(&pipeline, &batch) == NO_ERROR);
    kest_free_fpga_transfer_batch(batch);
}
