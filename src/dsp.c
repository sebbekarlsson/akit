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




float akit_dsp_get_corrected_sample(
  float sample
) {
  if (akit_number_is_unsafe(sample)) return 0.0f;//akit_random_range(-0.001f, 0.001f);

  sample = akit_clamp(sample, AKIT_MIN_SAMPLE, AKIT_MAX_SAMPLE);

  if (akit_number_is_unsafe(sample)) return 0.0f;//akit_random_range(-0.001f, 0.001f);

  return sample;
}


void akit_dsp_process(AkitEngine* engine, AkitSoundClip* clip, float* left, float* right) {
  if (!left || !right) return;

  float L = *left;
  float R = *right;

  if (engine->config.normalize_stereo) {

    float v = akit_clamp((L+R), -1.001f, 1.001f);

    L = v;
    R = v;
  }


  L = akit_clamp(L, AKIT_MIN_SAMPLE, AKIT_MAX_SAMPLE);
  R = akit_clamp(R, AKIT_MIN_SAMPLE, AKIT_MAX_SAMPLE);


  if (akit_number_is_unsafe(L) || akit_number_is_unsafe(R)) return;


  *left = L;
  *right = L;

}
