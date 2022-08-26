#include <akit/engine.h>
#include <akit/constants.h>
#include <akit/macros.h>
#include <akit/sleep.h>
#include <akit/utils.h>
#include <akit/dsp.h>
#include <stdio.h>


void akit_engine_process_clip(AkitEngine *engine, AkitSoundClip *clip,
                              float *buffer, int64_t length, double time,
                              int64_t frame) {
  int64_t clip_length = clip->sound.length;
  int64_t clip_channels = OR(clip->sound.channels, 1);

  if (clip->time >= clip->sound.duration ||
      (clip->cursor + length) >= (clip_length)) {
    clip->finished = true;
    return;
  }

  float sample_rate =
      OR(clip->sound.sample_rate, akit_engine_get_sample_rate(engine));

  float *clip_buffer = &clip->sound.data[clip->cursor];

  AkitListener listener = akit_engine_get_listener(*engine);

  float left_gain = 0.0f;
  float right_gain = 0.0f;

  akit_sound_compute_gain(clip->sound, listener, &left_gain, &right_gain);


//  int64_t samples_avail = MAX(0, (clip_length / sizeof(float)) - clip->cursor);

  if (clip_channels >= 2) {
    for (int64_t i = 0; i < length; i++) {
      if (clip->cursor + (i * 2) >= (clip_length / sizeof(float))) {
        clip->finished = true;
        break;
      }

      float *out_left = &buffer[i * 2];
      float *out_right = &buffer[1 + i * 2];

      float in_left = clip_buffer[i * 2];
      float in_right = clip_buffer[1 + i * 2];

      *out_left += (in_left * left_gain);
      *out_right += (in_right * right_gain);

      *out_left = akit_dsp_get_corrected_sample(*out_left);
      *out_right = akit_dsp_get_corrected_sample(*out_right);
    }

  } else {

    for (int64_t i = 0; i < length; i++) {
      if (clip->cursor + i >= (clip_length / sizeof(float))) {
        clip->finished = true;
        break;
      }

      float *out_left = &buffer[i * 2];
      float *out_right = &buffer[1 + i * 2];

      float sample = clip_buffer[i];

      *out_left += (sample * left_gain);
      *out_right += (sample * right_gain);

      *out_left = akit_dsp_get_corrected_sample(*out_left);
      *out_right = akit_dsp_get_corrected_sample(*out_right);

    }
  }

  clip->cursor += length * clip_channels;

  int64_t divisor = clip_channels <= 1 ? 1 : 2;

  clip->time = (1.0f / sample_rate) * (clip->cursor / divisor);

  if (clip->time >= clip->sound.duration || (clip->cursor) >= (clip_length)) {
    clip->finished = true;
  }
}

void akit_engine_process(AkitEngine *engine, float *buffer, int64_t length,
                         double time, int64_t frame) {

  for (int64_t i = 0; i < engine->clips.length; i++) {
    AkitSoundClip *clip = (AkitSoundClip *)engine->clips.items[i];

    if (clip->time_pushed <= 0.0f) {
      clip->time_pushed = time;
    }

    if (clip->finished && clip->cursor > 0) {
      akit_sound_clip_destroy(clip);
      akit_array_remove(&engine->clips, clip, 0);
      free(clip);
      clip = 0;
    } else {
      akit_engine_process_clip(engine, clip, buffer, length, time, frame);
    }
  }
}

void *akit_engine_thread(void *ptr) {
  if (!ptr) {
    fprintf(stderr, "(Akit): Invalid thread pointer.\n");
    pthread_exit(0);
    return 0;
  }

  AkitEngine *engine = (AkitEngine *)ptr;

  if (engine->tape != 0) {
    fprintf(stderr, "Engine tape was not zero when starting thread.\n");
    pthread_exit(0);
    return 0;
  }

  if (!akit_driver_init(&engine->driver, engine->config.driver_config)) {
    fprintf(stderr, "Failed to initialize driver.\n");
    pthread_exit(0);
    return 0;
  }

  engine->stopped = false;
  engine->running = true;

  printf("(Akit): Engine started.\n");

  float sample_rate = akit_engine_get_sample_rate(engine);
  int64_t channels = akit_engine_get_channels(engine);
  int64_t frame_length = akit_engine_get_frame_length(engine);
  int64_t tape_length =
      sample_rate * 4; // frame_length * channels *
                       // 2; // sample_rate * 4;//rame_length * channels * 4;
  float time_unit = (float)(frame_length) / (float)(sample_rate);

  printf("Engine info:\n");
  printf("sample_rate: %12.6f\n", sample_rate);
  printf("channels: %ld\n", channels);
  printf("frame_length: %ld\n", frame_length);
  printf("tape_length: %ld\n", tape_length);

  engine->tape = (float *)calloc(tape_length * AKIT_TAPE_LENGTH_MULTIPLIER, sizeof(float));

  akit_msleep(2000);

  while (true) {

    if (engine->frame + (frame_length * channels) >= tape_length) {
      engine->frame = 0;
      akit_engine_clear_tape(engine);
     // akit_msleep(ceilf(time_unit * 60));
    }

    pthread_mutex_lock(&engine->push_lock);
    akit_engine_clear_sounds(engine);
    pthread_mutex_unlock(&engine->push_lock);

    if (akit_array_is_empty(&engine->clips)) {
      akit_engine_clear_tape(engine);
      akit_driver_prepare(&engine->driver);
      akit_driver_flush(&engine->driver);
    //  akit_msleep(ceilf(time_unit * 60));
    }

   // if (!engine->tape)
   //   continue;

    akit_driver_flush(&engine->driver);

    pthread_mutex_lock(&engine->push_lock);
    akit_engine_process(engine, &engine->tape[engine->frame], frame_length,
                        engine->time, engine->frame);
    pthread_mutex_unlock(&engine->push_lock);

    akit_driver_buffer_data(&engine->driver, &engine->tape[engine->frame],
                            frame_length);

    engine->frame += frame_length * channels;
    engine->time += time_unit;
    akit_msleep(time_unit * 1000);

    if (engine->stopped || !engine->running) {
      akit_driver_destroy(&engine->driver);
      break;
    }
  }

  printf("(Akit): Engine stopped.\n");
  pthread_exit(0);
  return 0;
}
