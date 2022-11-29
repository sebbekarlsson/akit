#include <akit/dsp.h>
#include <akit/utils.h>
#include <akit/macros.h>
#include <akit/constants.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <mif/utils.h>
#include <mif/fft.h>
#include <vec3/vec3.h>



float akit_dsp_get_sample_error(float sample) {
  return akit_clamp(fmaxf(0.0, fabsf(sample) - AKIT_DSP_SAMPLE_TARGET), 0.0f, 1.0f);
}


bool akit_dsp_sample_is_bad(float sample, float* error) {

  *error = akit_dsp_get_sample_error(sample);
  return (*error >= AKIT_DSP_SAMPLE_ERROR_TOLERANCE) || (isinf(sample) || isnan(sample));
}

float akit_dsp_get_corrected_sample(
  float sample
) {
  if (isinf(sample) || isnan(sample)) return akit_random_range(-0.001f, 0.001f);

  sample = akit_clamp(sample, AKIT_MIN_SAMPLE, AKIT_MAX_SAMPLE);

  return sample;
//  float error = 0.0f;
 // if (!akit_dsp_sample_is_bad(sample, &error)) return sample;

  //return akit_clamp(sample, -AKIT_DSP_SAMPLE_TARGET, AKIT_DSP_SAMPLE_TARGET);
}


void akit_dsp_process_channels(AkitEngine* engine, AkitSoundClip* clip, float* left, float* right) {



    float L = *left;
  float R = *right;
  if (engine->config.normalize_stereo) {

    float v = akit_clamp((L+R), -1.001f, 1.001f);



    L = v;
    R = v;
  }


  L = akit_clamp(L, AKIT_MIN_SAMPLE, AKIT_MAX_SAMPLE);
  R = akit_clamp(R, AKIT_MIN_SAMPLE, AKIT_MAX_SAMPLE);



  *left = L;
  *right = L;
}

void akit_dsp_process(AkitEngine* engine, AkitSoundClip* clip, float* left, float* right) {
  akit_dsp_process_channels(engine, clip, left, right);
}
