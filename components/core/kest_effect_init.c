#include "kest_int.h"

#ifndef PRINTLINES_ALLOWED
#define PRINTLINES_ALLOWED 0
#endif

static const char *unit_string_    = "";
static const char *unit_string_hz  = " Hz";
static const char *unit_string_ms  = " ms";
static const char *unit_string_db  = " dB";
static const char *unit_string_bpm = " bpm";

static const char *FNAME = "kest_effect_init.c";
int init_3_band_eq(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_3_BAND_EQ;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 0.0;
	param->max   = 18.0;
	param->min   = -18.0;
	param->name  = "Low";
	param->units = unit_string_db;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 0;
	param->widget_type = PARAM_WIDGET_VSLIDER_TALL;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 1;

	param->value   = 0.0;
	param->max   = 18.0;
	param->min   = -18.0;
	param->name  = "Mid";
	param->units = unit_string_db;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 0;
	param->widget_type = PARAM_WIDGET_VSLIDER_TALL;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 2;

	param->value   = 0.0;
	param->max   = 18.0;
	param->min   = -18.0;
	param->name  = "High";
	param->units = unit_string_db;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 0;
	param->widget_type = PARAM_WIDGET_VSLIDER_TALL;

	return NO_ERROR;
}


int init_amplifier(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_AMPLIFIER;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 0;
	param->max   = 12;
	param->min   = -12;
	param->name  = "Gain";
	param->units = unit_string_db;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;


	kest_setting *setting;

	setting = effect_add_setting(effect);

	if (!setting)
		return ERR_ALLOC_FAIL;

	setting->id.setting_id = 0;

	setting->value   = 1;
	setting->max   = 255;
	setting->min   = 0;
	setting->name  = "Mode";
	setting->units = unit_string_;
	setting->group = -1;
	setting->widget_type = SETTING_WIDGET_DROPDOWN;
	setting->page = EFFECT_SETTING_PAGE_SETTINGS;
	setting->n_options = 2;
	setting->options = kest_alloc(sizeof(kest_setting) * 2);
	if (!setting->options) return ERR_ALLOC_FAIL;

	setting->options[0].value = 0;
	setting->options[0].name = "Linear";

	setting->options[1].value = 1;
	setting->options[1].name = "dB";

	return NO_ERROR;
}


int init_band_pass_filter(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_BAND_PASS_FILTER;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 1000.0;
	param->max   = 10000.0;
	param->min   = 1.0;
	param->name  = "Center";
	param->units = unit_string_hz;
	param->scale = PARAMETER_SCALE_LOGARITHMIC;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 1;

	param->value   = 100.0;
	param->max   = 10000.0;
	param->min   = 1.0;
	param->name  = "Bandwidth";
	param->units = unit_string_hz;
	param->scale = PARAMETER_SCALE_LOGARITHMIC;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	return NO_ERROR;
}


int init_compressor(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_COMPRESSOR;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 2.0;
	param->max   = 10.0;
	param->min   = 1.0;
	param->name  = "Ratio";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 1;

	param->value   = -5.0;
	param->max   = -30.0;
	param->min   = 0.0;
	param->name  = "Threshold";
	param->units = unit_string_db;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 2;

	param->value   = 30.0;
	param->max   = 250.0;
	param->min   = 0.01;
	param->name  = "Attack";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 3;

	param->value   = 30.0;
	param->max   = 250.0;
	param->min   = 0.01;
	param->name  = "Release";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	return NO_ERROR;
}


int init_delay(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_DELAY;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = -6.0;
	param->max   = 0.0;
	param->min   = -12;
	param->name  = "Delay Gain";
	param->units = unit_string_db;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 0;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;


	kest_setting *setting;

	setting = effect_add_setting(effect);

	if (!setting)
		return ERR_ALLOC_FAIL;

	setting->id.setting_id = 0;

	setting->value   = 120;
	setting->max   = 1000;
	setting->min   = 1;
	setting->name  = "Tempo";
	setting->units = unit_string_;
	setting->group = 1;
	setting->widget_type = SETTING_WIDGET_FIELD;
	setting->page = EFFECT_SETTING_PAGE_MAIN;
	setting->n_options = 0;
	setting->options = NULL;
	setting = effect_add_setting(effect);

	if (!setting)
		return ERR_ALLOC_FAIL;

	setting->id.setting_id = 1;

	setting->value   = 4;
	setting->max   = 255;
	setting->min   = 0;
	setting->name  = "Note";
	setting->units = unit_string_;
	setting->group = 2;
	setting->widget_type = SETTING_WIDGET_DROPDOWN;
	setting->page = EFFECT_SETTING_PAGE_MAIN;
	setting->n_options = 5;
	setting->options = kest_alloc(sizeof(kest_setting) * 5);
	if (!setting->options) return ERR_ALLOC_FAIL;

	setting->options[0].value = 1;
	setting->options[0].name = "Whole";

	setting->options[1].value = 2;
	setting->options[1].name = "Half";

	setting->options[2].value = 4;
	setting->options[2].name = "Quarter";

	setting->options[3].value = 8;
	setting->options[3].name = "Eighth";

	setting->options[4].value = 16;
	setting->options[4].name = "Sixteenth";

	return NO_ERROR;
}


int init_dirty_octave(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_DIRTY_OCTAVE;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 5;
	param->max   = 10;
	param->min   = 0;
	param->name  = "Fuzz";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	return NO_ERROR;
}


int init_distortion(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_DISTORTION;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 4.0;
	param->max   = 15;
	param->min   = 0.0;
	param->name  = "Gain";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 1;

	param->value   = 0.7;
	param->max   = 1.0;
	param->min   = 0.0;
	param->name  = "Wet Mix";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 2;

	param->value   = 0.4;
	param->max   = 1.0;
	param->min   = 0.0;
	param->name  = "Bass Mix";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 3;

	param->value   = 125;
	param->max   = 250;
	param->min   = 20;
	param->name  = "Bass Cutoff";
	param->units = unit_string_hz;
	param->scale = PARAMETER_SCALE_LOGARITHMIC;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;


	kest_setting *setting;

	setting = effect_add_setting(effect);

	if (!setting)
		return ERR_ALLOC_FAIL;

	setting->id.setting_id = 0;

	setting->value   = 1;
	setting->max   = 255;
	setting->min   = 0;
	setting->name  = "Function";
	setting->units = unit_string_;
	setting->group = -1;
	setting->widget_type = SETTING_WIDGET_DROPDOWN;
	setting->page = EFFECT_SETTING_PAGE_MAIN;
	setting->n_options = 4;
	setting->options = kest_alloc(sizeof(kest_setting) * 4);
	if (!setting->options) return ERR_ALLOC_FAIL;

	setting->options[0].value = 0;
	setting->options[0].name = "Clip";

	setting->options[1].value = 1;
	setting->options[1].name = "Tanh";

	setting->options[2].value = 2;
	setting->options[2].name = "Arctan";

	setting->options[3].value = 3;
	setting->options[3].name = "Fold";

	return NO_ERROR;
}


int init_envelope(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_ENVELOPE;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 200.0;
	param->max   = 500.0;
	param->min   = 50.0;
	param->name  = "Min Center";
	param->units = unit_string_hz;
	param->scale = PARAMETER_SCALE_LOGARITHMIC;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 1;

	param->value   = 2000.0;
	param->max   = 5000.0;
	param->min   = 200.0;
	param->name  = "Max Center";
	param->units = unit_string_hz;
	param->scale = PARAMETER_SCALE_LOGARITHMIC;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 2;

	param->value   = 0.25;
	param->max   = 1.0;
	param->min   = 0.1;
	param->name  = "Width";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 3;

	param->value   = 2.9;
	param->max   = 0.5;
	param->min   = 200.0;
	param->name  = "Speed";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 4;

	param->value   = 2.0;
	param->max   = 10.0;
	param->min   = 0.1;
	param->name  = "Sensitivity";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 5;

	param->value   = 0.5;
	param->max   = 1.0;
	param->min   = 0.0;
	param->name  = "Smoothness";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	return NO_ERROR;
}


int init_flanger(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_FLANGER;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 0.5;
	param->max   = 1.0;
	param->min   = 0.0;
	param->name  = "Range";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 1;

	param->value   = 4.0;
	param->max   = 10.0;
	param->min   = 0.1;
	param->name  = "Depth";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 2;

	param->value   = 0.5;
	param->max   = 1.0;
	param->min   = 0.0;
	param->name  = "Mix";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 3;

	param->value   = 120;
	param->max   = 300;
	param->min   = 30;
	param->name  = "Tempo";
	param->units = unit_string_bpm;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;


	kest_setting *setting;

	setting = effect_add_setting(effect);

	if (!setting)
		return ERR_ALLOC_FAIL;

	setting->id.setting_id = 0;

	setting->value   = 4;
	setting->max   = 255;
	setting->min   = 0;
	setting->name  = "Note";
	setting->units = unit_string_;
	setting->group = -1;
	setting->widget_type = SETTING_WIDGET_DROPDOWN;
	setting->page = EFFECT_SETTING_PAGE_MAIN;
	setting->n_options = 5;
	setting->options = kest_alloc(sizeof(kest_setting) * 5);
	if (!setting->options) return ERR_ALLOC_FAIL;

	setting->options[0].value = 1;
	setting->options[0].name = "Whole";

	setting->options[1].value = 2;
	setting->options[1].name = "Half";

	setting->options[2].value = 4;
	setting->options[2].name = "Quarter";

	setting->options[3].value = 8;
	setting->options[3].name = "Eighth";

	setting->options[4].value = 16;
	setting->options[4].name = "Sixteenth";

	return NO_ERROR;
}


int init_high_pass_filter(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_HIGH_PASS_FILTER;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 1000.0;
	param->max   = 10000.0;
	param->min   = 1.0;
	param->name  = "Cutoff Frequency";
	param->units = unit_string_hz;
	param->scale = PARAMETER_SCALE_LOGARITHMIC;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	return NO_ERROR;
}


int init_low_end_compressor(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_LOW_END_COMPRESSOR;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 2.0;
	param->max   = 10.0;
	param->min   = 1.0;
	param->name  = "Bass Ratio";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 1;

	param->value   = -12.0;
	param->max   = -30.0;
	param->min   = 0.0;
	param->name  = "Bass Threshold";
	param->units = unit_string_db;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 2;

	param->value   = 10.0;
	param->max   = 250.0;
	param->min   = 0.01;
	param->name  = "Bass Attack";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 2;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 3;

	param->value   = 200.0;
	param->max   = 250.0;
	param->min   = 0.01;
	param->name  = "Bass Release";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 2;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 4;

	param->value   = 2.0;
	param->max   = 10.0;
	param->min   = 1.0;
	param->name  = "Mid Ratio";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 3;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 5;

	param->value   = -8.0;
	param->max   = -30.0;
	param->min   = 0.0;
	param->name  = "Mid Threshold";
	param->units = unit_string_db;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 3;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 6;

	param->value   = 20.0;
	param->max   = 250.0;
	param->min   = 0.01;
	param->name  = "Mid Attack";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 4;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 7;

	param->value   = 200.0;
	param->max   = 250.0;
	param->min   = 0.01;
	param->name  = "Mid Release";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 4;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	return NO_ERROR;
}


int init_low_pass_filter(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_LOW_PASS_FILTER;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 100.0;
	param->max   = 10000.0;
	param->min   = 1.0;
	param->name  = "Cutoff Frequency";
	param->units = unit_string_hz;
	param->scale = PARAMETER_SCALE_LOGARITHMIC;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	return NO_ERROR;
}


int init_noise_suppressor(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_NOISE_SUPPRESSOR;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = -50;
	param->max   = -10;
	param->min   = -100;
	param->name  = "Threshold";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 1;

	param->value   = -30;
	param->max   = -100;
	param->min   = 0;
	param->name  = "Max Reduction";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 2;

	param->value   = 1.0;
	param->max   = 20.0;
	param->min   = 0.0;
	param->name  = "Ratio";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	return NO_ERROR;
}


int init_percussifier(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_PERCUSSIFIER;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 120.0;
	param->max   = 300;
	param->min   = 20;
	param->name  = "Tempo";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 0;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 1;

	param->value   = 16.0;
	param->max   = 32.0;
	param->min   = 4.0;
	param->name  = "Note";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = 0;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 2;

	param->value   = 3.0;
	param->max   = 4.0;
	param->min   = 0.0;
	param->name  = "Trigger Threshold";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 3;

	param->value   = 1.5;
	param->max   = 2.0;
	param->min   = 0.0;
	param->name  = "Arm Threshold";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 4;

	param->value   = 2.9;
	param->max   = 10.0;
	param->min   = 0.5;
	param->name  = "Fade In";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 5;

	param->value   = 6;
	param->max   = 10.0;
	param->min   = 0.5;
	param->name  = "Fade Out";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 6;

	param->value   = 100.0;
	param->max   = 700;
	param->min   = 0.0;
	param->name  = "Refractory Period";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	return NO_ERROR;
}


int init_warbler(kest_effect *effect)
{
	if (!effect)
		return ERR_NULL_PTR;

	effect->type = EFFECT_WARBLER;
	effect->view_page = NULL;

	kest_parameter *param;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 0;

	param->value   = 440.0;
	param->max   = 1500.0;
	param->min   = 50.0;
	param->name  = "Center";
	param->units = unit_string_hz;
	param->scale = PARAMETER_SCALE_LOGARITHMIC;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 1;

	param->value   = 0.25;
	param->max   = 1.0;
	param->min   = 0.1;
	param->name  = "Width";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 2;

	param->value   = 100;
	param->max   = 0.5;
	param->min   = 200.0;
	param->name  = "Reactivity";
	param->units = unit_string_ms;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 3;

	param->value   = 2.0;
	param->max   = 10.0;
	param->min   = 0.1;
	param->name  = "Sensitivity";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 4;

	param->value   = 0.25;
	param->max   = 1.0;
	param->min   = 0.0;
	param->name  = "Min Rate";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	param = effect_add_parameter(effect);

	if (!param)
		return ERR_ALLOC_FAIL;

	param->id.parameter_id = 5;

	param->value   = 0.5;
	param->max   = 3.0;
	param->min   = 1.0;
	param->name  = "Max Rate";
	param->units = unit_string_;
	param->scale = PARAMETER_SCALE_LINEAR;
	param->group = -1;
	param->widget_type = PARAM_WIDGET_VIRTUAL_POT;

	return NO_ERROR;
}


int init_effect_of_type(kest_effect *effect, uint16_t type)
{
	if (!effect)
		return ERR_NULL_PTR;

	int ret_val = init_effect(effect);
	
	if (ret_val != NO_ERROR)
		return ret_val;
	switch (type)
	{
		case EFFECT_3_BAND_EQ:          return init_3_band_eq(effect);
		case EFFECT_AMPLIFIER:          return init_amplifier(effect);
		case EFFECT_BAND_PASS_FILTER:   return init_band_pass_filter(effect);
		case EFFECT_COMPRESSOR:         return init_compressor(effect);
		case EFFECT_DELAY:              return init_delay(effect);
		case EFFECT_DIRTY_OCTAVE:       return init_dirty_octave(effect);
		case EFFECT_DISTORTION:         return init_distortion(effect);
		case EFFECT_ENVELOPE:           return init_envelope(effect);
		case EFFECT_FLANGER:            return init_flanger(effect);
		case EFFECT_HIGH_PASS_FILTER:   return init_high_pass_filter(effect);
		case EFFECT_LOW_END_COMPRESSOR: return init_low_end_compressor(effect);
		case EFFECT_LOW_PASS_FILTER:    return init_low_pass_filter(effect);
		case EFFECT_NOISE_SUPPRESSOR:   return init_noise_suppressor(effect);
		case EFFECT_PERCUSSIFIER:       return init_percussifier(effect);
		case EFFECT_WARBLER:            return init_warbler(effect);
		default: return ERR_BAD_ARGS;
	}

	return NO_ERROR;
}
