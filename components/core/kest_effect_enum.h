// Code generated from config/effect/*.yaml by codegen.py
#ifndef KEST_EFFECT_ENUM_H_
#define KEST_EFFECT_ENUM_H_

#define EFFECT_3_BAND_EQ          0
#define EFFECT_AMPLIFIER          1
#define EFFECT_BAND_PASS_FILTER   2
#define EFFECT_COMPRESSOR         3
#define EFFECT_DELAY              4
#define EFFECT_DIRTY_OCTAVE       5
#define EFFECT_DISTORTION         6
#define EFFECT_ENVELOPE           7
#define EFFECT_FLANGER            8
#define EFFECT_HIGH_PASS_FILTER   9
#define EFFECT_LOW_END_COMPRESSOR 10
#define EFFECT_LOW_PASS_FILTER    11
#define EFFECT_NOISE_SUPPRESSOR   12
#define EFFECT_PERCUSSIFIER       13
#define EFFECT_WARBLER            14

typedef enum
{
	low_pass		= 0,
	high_pass		= 1,
	band_pass 		= 2,
	notch 			= 3,
	peaking_band_eq = 4,
	low_shelf 		= 5,
	high_shelf 		= 6
} biquad_type;

#define DISTORTION_SOFT_FOLD 	0
#define DISTORTION_ARCTAN		1
#define DISTORTION_TANH			2
#define DISTORTION_CLIP			3

const char *effect_type_to_string(uint16_t type);
int effect_type_valid(uint16_t type);

#endif
