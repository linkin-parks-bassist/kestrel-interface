#include "m_test.h"

M_TEST(m_test_format_float_negative)
{
    char buf[32];

    format_float(buf, -2.5f, 32);

    assert(strncmp(buf, "-2.50", 5) == 0);
}


M_TEST(m_test_format_float_small)
{
    char buf[32];

    format_float(buf, 0.01f, 32);

    assert(strcmp(buf, "0.01") == 0);
}


M_TEST(m_test_nullify_parameter_widget_null)
{
    int rc = nullify_parameter_widget(NULL);

    assert(rc == ERR_NULL_PTR);
}


M_TEST(m_test_nullify_parameter_widget_basic)
{
    m_parameter_widget pw;

    int rc = nullify_parameter_widget(&pw);

    assert(rc == NO_ERROR);

    assert(pw.param == NULL);
    assert(pw.obj == NULL);
    assert(pw.name_label == NULL);
    assert(pw.val_label == NULL);
    assert(pw.container == NULL);
    assert(pw.parent == NULL);

    assert(pw.val_label_text[0] == 0);

    assert(pw.rep.representer == &pw);
    assert(pw.rep.representee == NULL);
    assert(pw.rep.update == param_widget_rep_update);
}


M_TEST(m_test_configure_parameter_widget_null)
{
    m_parameter param;

    int rc = configure_parameter_widget(NULL, &param, NULL, NULL);

    assert(rc == ERR_NULL_PTR);
}


M_TEST(m_test_configure_parameter_widget_basic)
{
    m_parameter_widget pw;
    m_parameter param;

    memset(&pw, 0, sizeof(pw));
    memset(&param, 0, sizeof(param));

    param.value = 2.25f;

    int rc = configure_parameter_widget(&pw, &param, NULL, NULL);

    assert(rc == NO_ERROR);

    assert(pw.param == &param);
}


M_TEST(m_test_parameter_widget_update_value_label_v_null)
{
    parameter_widget_update_value_label_v(NULL, 1.0f);
}


M_TEST(m_test_parameter_widget_update_value_label_v_basic)
{
    m_parameter_widget pw;
    m_parameter param;

    memset(&pw, 0, sizeof(pw));
    memset(&param, 0, sizeof(param));

    pw.param = &param;

    parameter_widget_update_value_label_v(&pw, 5.5f);

    assert(strlen(pw.val_label_text) > 0);
}


M_TEST(m_test_parameter_widget_update_value_label_basic)
{
    m_parameter_widget pw;
    m_parameter param;

    memset(&pw, 0, sizeof(pw));
    memset(&param, 0, sizeof(param));

    pw.param = &param;
    param.value = 1.75f;

    parameter_widget_update_value_label(&pw);

    assert(strlen(pw.val_label_text) > 0);
}


M_TEST(m_test_nullify_setting_widget_null)
{
    int rc = nullify_setting_widget(NULL);

    assert(rc == ERR_NULL_PTR);
}


M_TEST(m_test_nullify_setting_widget_basic)
{
    m_setting_widget sw;

    int rc = nullify_setting_widget(&sw);

    assert(rc == NO_ERROR);

    assert(sw.setting == NULL);
    assert(sw.obj == NULL);
    assert(sw.type == SETTING_WIDGET_DROPDOWN);
    assert(sw.saved_field_text == NULL);
    assert(sw.parent == NULL);

    assert(sw.rep.representer == &sw);
    assert(sw.rep.representee == NULL);
    assert(sw.rep.update == setting_widget_rep_update);
}


M_TEST(m_test_configure_setting_widget_null)
{
    m_setting setting;

    int rc = configure_setting_widget(NULL, &setting, NULL, NULL);

    assert(rc == ERR_NULL_PTR);
}


M_TEST(m_test_configure_setting_widget_basic)
{
    m_setting_widget sw;
    m_setting setting;

    memset(&sw, 0, sizeof(sw));
    memset(&setting, 0, sizeof(setting));

    setting.widget_type = SETTING_WIDGET_DROPDOWN;

    int rc = configure_setting_widget(&sw, &setting, NULL, NULL);

    assert(rc == NO_ERROR);

    assert(sw.setting == &setting);
    assert(sw.type == SETTING_WIDGET_DROPDOWN);
}
