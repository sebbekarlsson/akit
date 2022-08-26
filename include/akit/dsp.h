#ifndef AKIT_DSP_H
#define AKIT_DSP_H
#include <stdint.h>
#include <stdbool.h>

float akit_dsp_get_sample_error(float sample);

float akit_dsp_get_corrected_sample(float sample);

bool akit_dsp_sample_is_bad(float sample, float* error);

#endif