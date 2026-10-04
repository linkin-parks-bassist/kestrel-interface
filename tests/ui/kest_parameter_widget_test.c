#include "kest_test.h"

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
