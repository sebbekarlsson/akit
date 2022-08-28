#ifndef AKIT_DSP_H
#define AKIT_DSP_H
#include <stdint.h>
#include <stdbool.h>
#include <akit/engine.h>
#include <akit/sound.h>

float akit_dsp_get_sample_error(float sample);

float akit_dsp_get_corrected_sample(float sample);

bool akit_dsp_sample_is_bad(float sample, float* error);


void akit_dsp_process(AkitEngine* engine, AkitSoundClip* clip, float* left, float* right);
void akit_dsp_process_fx(AkitEngine* engine, AkitSoundClip* clip);

#endif
