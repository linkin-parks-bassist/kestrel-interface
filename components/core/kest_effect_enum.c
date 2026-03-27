// Code generated from config/effect/*.yaml by codegen.py
#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "kest_effect_enum.h"

static const char *FNAME = "kest_effect_enum.c";

const char *effect_type_to_string(uint16_t type)
{
	 switch(type)
	{
		case EFFECT_3_BAND_EQ:          return "EFFECT_3_BAND_EQ";
		case EFFECT_AMPLIFIER:          return "EFFECT_AMPLIFIER";
		case EFFECT_BAND_PASS_FILTER:   return "EFFECT_BAND_PASS_FILTER";
		case EFFECT_COMPRESSOR:         return "EFFECT_COMPRESSOR";
		case EFFECT_DELAY:              return "EFFECT_DELAY";
		case EFFECT_DIRTY_OCTAVE:       return "EFFECT_DIRTY_OCTAVE";
		case EFFECT_DISTORTION:         return "EFFECT_DISTORTION";
		case EFFECT_ENVELOPE:           return "EFFECT_ENVELOPE";
		case EFFECT_FLANGER:            return "EFFECT_FLANGER";
		case EFFECT_HIGH_PASS_FILTER:   return "EFFECT_HIGH_PASS_FILTER";
		case EFFECT_LOW_END_COMPRESSOR: return "EFFECT_LOW_END_COMPRESSOR";
		case EFFECT_LOW_PASS_FILTER:    return "EFFECT_LOW_PASS_FILTER";
		case EFFECT_NOISE_SUPPRESSOR:   return "EFFECT_NOISE_SUPPRESSOR";
		case EFFECT_PERCUSSIFIER:       return "EFFECT_PERCUSSIFIER";
		case EFFECT_WARBLER:            return "EFFECT_WARBLER";
		default: return "UNKNOWN";
	}
}

int effect_type_valid(uint16_t type)
{
	 switch(type)
	{
		case EFFECT_3_BAND_EQ: 
		case EFFECT_AMPLIFIER: 
		case EFFECT_BAND_PASS_FILTER: 
		case EFFECT_COMPRESSOR: 
		case EFFECT_DELAY: 
		case EFFECT_DIRTY_OCTAVE: 
		case EFFECT_DISTORTION: 
		case EFFECT_ENVELOPE: 
		case EFFECT_FLANGER: 
		case EFFECT_HIGH_PASS_FILTER: 
		case EFFECT_LOW_END_COMPRESSOR: 
		case EFFECT_LOW_PASS_FILTER: 
		case EFFECT_NOISE_SUPPRESSOR: 
		case EFFECT_PERCUSSIFIER: 
		case EFFECT_WARBLER: 
			return 1;
		default:
			return 0;
	}
}
