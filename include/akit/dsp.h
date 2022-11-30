#ifndef AKIT_DSP_H
#define AKIT_DSP_H
#include <stdint.h>
#include <stdbool.h>
#include <akit/engine.h>
#include <akit/sound.h>

void akit_dsp_process(AkitEngine* engine, AkitSoundClip* clip, float* left, float* right);

float akit_dsp_get_corrected_sample(float sample);
#endif
