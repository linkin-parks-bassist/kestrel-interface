#include "kest_test.h"
#include <limits.h>
extern void sw_field_save_cb(lv_event_t *e);

KEST_TEST(test_numeric_setting_validates_whole_integer_without_mutating_on_rejection)
{
    lv_display_t *display = lv_display_get_default();
    int32_t width = lv_display_get_horizontal_resolution(display);
    int32_t height = lv_display_get_vertical_resolution(display);
    lv_display_set_resolution(display, 720, 1280);
    kest_setting setting = { .type = EFFECT_SETTING_INT, .widget_type = SETTING_WIDGET_FIELD,
        .value = 120, .min = INT_MIN, .max = INT_MAX, .name = "Tempo", .units = "BPM" };
    kest_preset preset = {0};
    kest_setting_widget widget;
    lv_obj_t *screen = lv_obj_create(NULL);
    nullify_setting_widget(&widget);
    configure_setting_widget(&widget, &setting, &preset, NULL);
    assert(setting_widget_create_ui(&widget, screen) == NO_ERROR);
    lv_obj_add_event_cb(widget.obj, sw_field_save_cb, LV_EVENT_READY, &widget);
    const char *invalid[] = {"", "-", "120.5", "1a20", "120BPM", " 120", "2147483648",
        "-2147483649", "999999999999999999999999999999999999999999"};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++)
    {
        lv_textarea_set_text(widget.obj, invalid[i]);
        lv_obj_send_event(widget.obj, LV_EVENT_READY, NULL);
        assert(setting.value == 120 && !setting.updated && !preset.unsaved_changes);
        assert(strcmp(lv_textarea_get_text(widget.obj), "120") == 0);
    }
    const char *valid[] = {"-2147483648", "2147483647", "+90", "-12"};
    const int expected[] = {INT_MIN, INT_MAX, 90, -12};
    for (size_t i = 0; i < sizeof(valid) / sizeof(valid[0]); i++)
    {
        lv_textarea_set_text(widget.obj, valid[i]);
        lv_obj_send_event(widget.obj, LV_EVENT_READY, NULL);
        assert(setting.value == expected[i] && setting.updated && preset.unsaved_changes);
    }
    setting.min = 30;
    setting.max = 300;
    lv_textarea_set_text(widget.obj, "900");
    lv_obj_send_event(widget.obj, LV_EVENT_READY, NULL);
    assert(setting.value == 300);
    lv_obj_delete(screen);
    lv_display_set_resolution(display, width, height);
}

KEST_TEST(test_dropdown_changes_request_active_rebuild_only_when_changed)
{
    extern QueueHandle_t event_queue;
    QueueHandle_t saved_queue = event_queue;
    event_queue = xQueueCreate(4, sizeof(kest_event));
    assert(event_queue);
    kest_setting_option options[] = {{ .value = 4, .name = "Quarter" }, { .value = 9, .name = "Dotted eighth" }};
    kest_setting setting = { .type = EFFECT_SETTING_ENUM, .widget_type = SETTING_WIDGET_DROPDOWN,
        .value = 4, .min = 0, .max = 9, .name = "Subdivision", .n_options = 2, .options = options };
    kest_preset preset = { .active = 1 };
    kest_setting_widget widget;
    lv_obj_t *screen = lv_obj_create(NULL);
    assert(nullify_setting_widget(&widget) == NO_ERROR);
    assert(configure_setting_widget(&widget, &setting, &preset, NULL) == NO_ERROR);
    assert(setting_widget_create_ui(&widget, screen) == NO_ERROR);
    lv_dropdown_set_selected(widget.obj, 1);
    lv_obj_send_event(widget.obj, LV_EVENT_VALUE_CHANGED, NULL);
    assert(setting.value == 9 && setting.old_value == 4 && setting.updated && preset.unsaved_changes);
    kest_event event;
    assert(xQueueReceive(event_queue, &event, 0) == pdTRUE);
    assert(event.type == KEST_EVENT_SETTING_CHANGE && event.val_ptr == &preset);
    lv_obj_send_event(widget.obj, LV_EVENT_VALUE_CHANGED, NULL);
    assert(xQueueReceive(event_queue, &event, 0) == pdFALSE);
    preset.active = 0;
    preset.unsaved_changes = 0;
    lv_dropdown_set_selected(widget.obj, 0);
    lv_obj_send_event(widget.obj, LV_EVENT_VALUE_CHANGED, NULL);
    assert(setting.value == 4 && setting.old_value == 9 && preset.unsaved_changes);
    assert(xQueueReceive(event_queue, &event, 0) == pdFALSE);
    lv_obj_delete(screen);
    vQueueDelete(event_queue);
    event_queue = saved_queue;
}

KEST_TEST(kest_test_format_float_negative)
{
    char buf[32];

    format_float(buf, -2.5f, 32);

    assert(strncmp(buf, "-2.50", 5) == 0);
}


KEST_TEST(kest_test_format_float_small)
{
    char buf[32];

    format_float(buf, 0.01f, 32);

    assert(strcmp(buf, "0.01") == 0);
}


KEST_TEST(kest_test_nullify_parameter_widget_null)
{
    int rc = nullify_parameter_widget(NULL);

    assert(rc == ERR_NULL_PTR);
}


KEST_TEST(kest_test_nullify_parameter_widget_basic)
{
    kest_parameter_widget pw;

    int rc = nullify_parameter_widget(&pw);

    assert(rc == NO_ERROR);

    assert(pw.param == NULL);
    assert(pw.obj == NULL);
    assert(pw.name_label == NULL);
    assert(pw.val_label == NULL);
    assert(pw.container == NULL);
    assert(pw.parent == NULL);

    assert(pw.val_label_text[0] == 0);

}


KEST_TEST(kest_test_configure_parameter_widget_null)
{
    kest_parameter param;

    int rc = configure_parameter_widget(NULL, &param, NULL, NULL);

    assert(rc == ERR_NULL_PTR);
}


KEST_TEST(kest_test_configure_parameter_widget_basic)
{
    kest_parameter_widget pw;
    kest_parameter param;

    memset(&pw, 0, sizeof(pw));
    memset(&param, 0, sizeof(param));

    param.value = 2.25f;

    int rc = configure_parameter_widget(&pw, &param, NULL, NULL);

    assert(rc == NO_ERROR);

    assert(pw.param == &param);
}


KEST_TEST(kest_test_parameter_widget_update_value_label_v_null)
{
    parameter_widget_update_value_label_v(NULL, 1.0f);
}


KEST_TEST(kest_test_parameter_widget_update_value_label_v_basic)
{
    kest_parameter_widget pw;
    kest_parameter param;

    memset(&pw, 0, sizeof(pw));
    memset(&param, 0, sizeof(param));

    pw.param = &param;

    parameter_widget_update_value_label_v(&pw, 5.5f);

    assert(strlen(pw.val_label_text) > 0);
}


KEST_TEST(kest_test_parameter_widget_update_value_label_basic)
{
    kest_parameter_widget pw;
    kest_parameter param;

    memset(&pw, 0, sizeof(pw));
    memset(&param, 0, sizeof(param));

    pw.param = &param;
    param.value = 1.75f;

    parameter_widget_update_value_label(&pw);

    assert(strlen(pw.val_label_text) > 0);
}


KEST_TEST(kest_test_nullify_setting_widget_null)
{
    int rc = nullify_setting_widget(NULL);

    assert(rc == ERR_NULL_PTR);
}


KEST_TEST(kest_test_nullify_setting_widget_basic)
{
    kest_setting_widget sw;
    memset(&sw, 0xa5, sizeof(sw));

    int rc = nullify_setting_widget(&sw);

    assert(rc == NO_ERROR);

    assert(sw.setting == NULL);
    assert(sw.obj == NULL);
    assert(sw.type == SETTING_WIDGET_DROPDOWN);
    assert(sw.saved_field_text == NULL);
    assert(sw.parent == NULL);
    assert(sw.container == NULL && sw.label == NULL && sw.pad == NULL);

}


KEST_TEST(kest_test_configure_setting_widget_null)
{
    kest_setting setting;

    int rc = configure_setting_widget(NULL, &setting, NULL, NULL);

    assert(rc == ERR_NULL_PTR);
}


KEST_TEST(kest_test_configure_setting_widget_basic)
{
    kest_setting_widget sw;
    kest_setting setting;

    memset(&sw, 0, sizeof(sw));
    memset(&setting, 0, sizeof(setting));

    setting.widget_type = SETTING_WIDGET_DROPDOWN;

    int rc = configure_setting_widget(&sw, &setting, NULL, NULL);

    assert(rc == NO_ERROR);

    assert(sw.setting == &setting);
    assert(sw.type == SETTING_WIDGET_DROPDOWN);
}

KEST_TEST(test_widget_free_cancels_queued_refresh_and_animation)
{
    kest_parameter param = {0};
    kest_parameter_widget *pw = kest_alloc(sizeof(*pw));
    assert(pw);
    memset(pw, 0, sizeof(*pw));
    pw->param = &param;
    param.pw = pw;
    pw->timer = lv_timer_create(NULL, 100, pw);
    assert(pw->timer);
    assert(kest_ui_async_call(kest_parameter_widget_refresh_async_wrapper, pw) == NO_ERROR);
    free_parameter_widget(pw);
    assert(param.pw == NULL);
    assert(lv_async_call_cancel(kest_parameter_widget_refresh_async_wrapper, pw) == LV_RESULT_INVALID);
    for (lv_timer_t *timer = lv_timer_get_next(NULL); timer; timer = lv_timer_get_next(timer))
        assert(lv_timer_get_user_data(timer) != pw);
}

KEST_TEST(test_effect_settings_teardown_releases_backstage_widgets)
{
    kest_effect effect;
    assert(init_effect(&effect) == NO_ERROR);
    kest_ui_page *page = kest_alloc(sizeof(*page));
    assert(page && init_effect_settings_page(page) == NO_ERROR);
    assert(configure_effect_settings_page(page, &effect) == NO_ERROR);
    effect_settings_page_str *str = page->data_struct;
    lv_obj_t *backstage = lv_obj_create(NULL);
    page->screen = lv_obj_create(NULL);
    str->band_lp_cutoff.container = lv_obj_create(backstage);
    str->band_hp_cutoff.container = lv_obj_create(backstage);
    str->band_mode.container = lv_obj_create(page->screen);
    str->band_lp_cutoff.timer = lv_timer_create(NULL, 100, &str->band_lp_cutoff);
    assert(kest_ui_async_call(kest_parameter_widget_refresh_async_wrapper, &str->band_lp_cutoff) == NO_ERROR);
    assert(free_effect_settings_page_ui(page) == NO_ERROR);
    assert(lv_obj_get_child_count(backstage) == 0);
    assert(page->screen == NULL && page->ui_created == 0);
    assert(effect.band_lp_cutoff.pw == NULL && effect.band_hp_cutoff.pw == NULL);
    assert(str->band_lp_cutoff.timer == NULL);
    assert(lv_async_call_cancel(kest_parameter_widget_refresh_async_wrapper, &str->band_lp_cutoff) == LV_RESULT_INVALID);
    assert(free_effect_settings_page_ui(page) == NO_ERROR); // Safe before full release too.
    assert(effect_settings_page_free_all(page) == NO_ERROR);
    lv_obj_del(backstage);
    kest_free(effect.band_mode.options);
    vSemaphoreDelete(effect.mutex);
    kest_block_list_destroy(&effect.blocks);
    kest_driver_list_destroy(&effect.drivers);
    kest_dsp_resource_ptr_list_destroy(&effect.resources);
}

KEST_TEST(test_preset_settings_initializes_gain_widget)
{
    kest_ui_page page;
    memset(&page, 0xa5, sizeof(page));
    assert(init_preset_settings_page(&page) == NO_ERROR);
    kest_preset_settings_str *str = page.data_struct;
    assert(str && str->volume_widget.timer == NULL);
    assert(str->volume_widget.container == NULL);
    assert(str->volume_widget.nominal_value == 0.0f);
    assert(str->save_button == NULL && str->default_button == NULL);
    kest_preset preset = {0};
    init_parameter(&preset.volume, "Gain", -2.5f, -12.0f, 12.0f);
    preset.name = "Fixture";
    assert(configure_preset_settings_page(&page, &preset) == NO_ERROR);
    assert(str->volume_widget.parent == &page);
    assert(str->volume_widget.nominal_value == -2.5f);
    assert(preset.volume.pw == &str->volume_widget);
    assert(str->volume_widget.driven == 0);
    kest_interval range = kest_parameter_get_range(&preset.volume);
    assert(range.a == -12.0f && range.b == 12.0f);
    parameter_widget_update_value_label(&str->volume_widget);
    assert(strcmp(str->volume_widget.val_label_text, "-2.50") == 0);
    gut_parameter_widget(&str->volume_widget);
    kest_free((void*)page.panel->text);
    kest_free(page.panel);
    kest_free(str);
}

KEST_TEST(test_parameter_range_mixes_literal_and_expression_bounds)
{
    kest_parameter param;
    init_parameter(&param, "Gain", 0.0f, -12.0f, 12.0f);
    param.min_expr = &kest_expression_standard_gain_min;
    kest_interval range = kest_parameter_get_range(&param);
    assert(range.a == KEST_STANDARD_GAIN_MIN && range.b == 12.0f);
    param.min_expr = NULL;
    param.max_expr = &kest_expression_standard_gain_max;
    range = kest_parameter_get_range(&param);
    assert(range.a == -12.0f && range.b == KEST_STANDARD_GAIN_MAX);
}
