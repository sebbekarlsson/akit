#include <akit/constants.h>
#include <akit/dsp.h>
#include <akit/macros.h>
#include <akit/utils.h>
#include <math.h>
#include <mif/utils.h>
#include <spath/spath.h>
#include <stdbool.h>
#include <stdio.h>
#include <waves/wav.h>

float akit_dsp_get_sample_error(float sample) {
  return akit_clamp(fmaxf(0.0, fabsf(sample) - AKIT_DSP_SAMPLE_TARGET), 0.0f,
                    1.0f);
}

bool akit_dsp_sample_is_bad(float sample, float *error) {

  *error = akit_dsp_get_sample_error(sample);
  return (*error >= AKIT_DSP_SAMPLE_ERROR_TOLERANCE) ||
         (isinf(sample) || isnan(sample));
}

float akit_dsp_get_corrected_sample(float sample) {
  if (isinf(sample) || isnan(sample))
    return akit_random_range(-0.001f, 0.001f);

  float error = 0.0f;
  if (!akit_dsp_sample_is_bad(sample, &error))
    return sample;

  return akit_clamp(sample, -AKIT_DSP_SAMPLE_TARGET, AKIT_DSP_SAMPLE_TARGET);
}

void akit_dsp_process_channels(AkitEngine *engine, AkitSoundClip *clip,
                               float *left, float *right) {
  float L = *left;
  float R = *right;
  if (engine->config.normalize_stereo) {

    float v = akit_clamp((L + R), -1.0f, 1.0f);

    L = v;
    R = v;
  }

  *left = L;
  *right = L;
}

void akit_dsp_process(AkitEngine *engine, AkitSoundClip *clip, float *left,
                      float *right) {
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


      float timeleft = (float)clip->cursor / ((float)clip->sound.length +
    0.001f);

      sound.cursor_start = clip->cursor;

      float dur = spacemag;

      sound.cursor_end = ((int64_t)(sound.cursor_start + (frame_length*dur))) %
    clip->sound.length; sound.duration = dur;

      float max_gain = akit_clamp(((fabsf(*left) + fabsf(*right)) * 0.5f), 0.0f,
    0.5f);

      sound.gain = clip->sound.gain * 0.5f;
      sound.start_time = engine->time + wait;
      sound.ignore_full = true;
      akit_engine_push_sound(engine, sound);
      clip->last_process = clip->time;
    }*/

  // int64_t frames_to_capture = 4;
  // int64_t total_size = (frame_length * frames_to_capture);

  //  float* buffer = &engine->tape[(engine->frame + (frame_length*channels)) %
  //  tape_length];
}

typedef struct {
  uint32_t start;
  uint32_t length;
  bool active;
  float eps;
  float sum;
  float avg;
  float peak;
} AkitFloatBufferView;
int akit_dsp_extract_onsets(Wave sound, const char *outname) {
  if (!outname || sound.data == 0 || sound.length <= 0)
    return 0;

  char barename[64];
  spath_get_barename(outname, barename);
  float *buff = sound.data;
  int channels = OR(sound.header.channels, 2);
  int64_t nr_samples = (sound.length / channels) / sizeof(float);

  int64_t peaks = mif_count_peaks(buff, nr_samples);
  float global_avg = mif_avg_abs(buff, nr_samples);
  float global_peak = mif_max_abs(buff, nr_samples, 0);
  float eps = 0.01f;
  printf("Peaks: %ld\n", peaks);
  printf("Global avg: %12.6f\n", global_avg);
  printf("eps: %12.6f\n", eps);

  int64_t view_idx = 0;
  int64_t nr_views = 0;
  AkitFloatBufferView views[256];

  for (int i = 0; i < 256; i++) {
    views[i] = (AkitFloatBufferView){.start = 0,
                                     .length = 0,
                                     .active = false,
                                     .eps = eps,
                                     .sum = 0.0f,
                                     .avg = 0.0f,
                                     .peak = 0.0f};
  }

  int64_t frame = 0;
  int64_t framesize = 128;

  bool reader_open = false;

  double rate = (double)sound.header.sample_rate;

  double T = 0.0f;
  double last_open_time = 0.0f;
  float reader_power = 0.0f;

  while (frame < nr_samples && nr_views < 256 && view_idx < 256) {
    AkitFloatBufferView *view = &views[view_idx];
    float *framebuffer = &buff[frame];
    float frame_avg = mif_avg_abs(framebuffer, framesize);
    float frame_peak = mif_max_abs(framebuffer, framesize, 0);

    float value = 0.0f;
    if (channels >= 2) {
      float L = fabsf(framebuffer[0]);
      float R = fabsf(framebuffer[1]);

      value = fabsf(fmaxf(L, R));
    } else {
      value = fabsf(framebuffer[0]);
    }

    double now = T;
    double diff = now - last_open_time;

    if (value >= frame_peak) {
      reader_power += 0.03f;
    }

    if (frame_avg >= view->eps) {
      reader_power += 0.02f;
    }

    if (reader_power > 0.0f && frame_avg > 0.009f) {
      if (reader_open == false) {
        reader_open = true;
        last_open_time = T;
        view->start = frame;
      }
      //  printf("open %12.6f \n", T);
      view->length += framesize * 2;
      view->active = true;
    } else if (reader_open == true && diff > 0.5f) {
      reader_open = false;
      view->active = false;
      last_open_time = 0.0;
      reader_power = 0.0f;
      view_idx++;
      nr_views++;
      //  printf("close %12.6f\n", T);
    }

    reader_power = fmaxf(0.0f, reader_power - 0.01f);
    frame += framesize;
    T += (1.0 / rate) * framesize;
  }

  printf("result: %ld\n", nr_views);

  int64_t count = 0;
  for (int64_t i = 0; i < nr_views; i++) {
    AkitFloatBufferView view = views[i];
    // if (view.length <= 1500)
    // continue;

    // if (view.avg <= global_avg*0.5f) continue;

    // printf("PEAK: %12.6f\n", view.peak);
    char tmp[256];
    sprintf(tmp, "slice_%ld_%s", count, barename);

    printf("%s\n", tmp);
    // printf("%d %d\n", view.start, view.length);
    Wave wav = {0};
    wav.data = &buff[view.start];
    wav.header = sound.header;
    wav.header.data_size = view.length * sizeof(float);
    wav.length = view.length;
    wav.header.overall_size = (sizeof(float) * view.length) + sizeof(WavHeader);
    wav_write(wav, tmp);
    count++;
  }
  return 1;
}
