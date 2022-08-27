#include <akit/dsp.h>
#include <akit/utils.h>
#include <akit/macros.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define AKIT_DSP_SAMPLE_TARGET 1.0f
#define AKIT_DSP_SAMPLE_ERROR_TOLERANCE 0.1f
#define AKIT_DSP_SILENCE 0.000001f


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

  float error = 0.0f;
  if (!akit_dsp_sample_is_bad(sample, &error)) return sample;

  return akit_clamp(sample, -AKIT_DSP_SAMPLE_TARGET, AKIT_DSP_SAMPLE_TARGET);
}


void akit_dsp_process_channels(AkitEngine* engine, AkitSoundClip* clip, float* left, float* right) {
    float L = *left;
  float R = *right;
  if (engine->config.normalize_stereo) {

    float v = akit_clamp((L+R), -1.0f, 1.0f);

    L = v;
    R = v;
  }

  *left = L;
  *right = L;
}

void akit_dsp_process(AkitEngine* engine, AkitSoundClip* clip, float* left, float* right) {
  akit_dsp_process_channels(engine, clip, left, right);
/*
  int64_t frame_length = akit_engine_get_frame_length(engine);
  int64_t tape_length = akit_engine_get_tape_length(engine);
  float sample_rate = akit_engine_get_sample_rate(engine);
  int64_t channels = 2;
  float second = frame_length*4;



  float rough = clip->sound.world_info.roughness;
  float refl = fmaxf(0.0f, 1.0f - rough);

  if (refl <= 0.0f) return;

  Vector3 space = clip->sound.world_info.size;
  float spacemag = fabsf(vector3_mag(space));

  float spacerough = (spacemag * rough);
  float wait = (spacerough / 2.0f) / 60.0f;

  if (fabs(clip->time - clip->last_process) > wait) {
    AkitSound sound = clip->sound;


    float timeleft = (float)clip->cursor / ((float)clip->sound.length + 0.001f);

    sound.cursor_start = clip->cursor;

    float dur = spacemag;

    sound.cursor_end = ((int64_t)(sound.cursor_start + (frame_length*dur))) % clip->sound.length;
    sound.duration = dur;

    float max_gain = akit_clamp(((fabsf(*left) + fabsf(*right)) * 0.5f), 0.0f, 0.5f);

    sound.gain = clip->sound.gain * 0.5f;
    sound.start_time = engine->time + wait;
    sound.ignore_full = true;
    akit_engine_push_sound(engine, sound);
    clip->last_process = clip->time;
  }*/

 // int64_t frames_to_capture = 4;
  //int64_t total_size = (frame_length * frames_to_capture);

//  float* buffer = &engine->tape[(engine->frame + (frame_length*channels)) % tape_length];

}
