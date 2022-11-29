#include <akit/constants.h>
#include <akit/dsp.h>
#include <akit/engine.h>
#include <akit/macros.h>
#include <akit/sleep.h>
#include <akit/utils.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <mif/utils.h>
#include <mif/fft.h>
#include <string.h>

void akit_engine_process(AkitEngine *engine, float *buffer, int64_t length,
                         double time, int64_t frame) {
  for (int64_t i = 0; i < engine->tracks_length; i++) {
    AkitTrack* track = &engine->tracks[i];
    if (!akit_track_process_block(engine, track, buffer, length, frame, time)) {
      continue;
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

  engine->running = true;

  printf("(Akit): Engine started.\n");

  float sample_rate = akit_engine_get_sample_rate(engine);
  int64_t channels = akit_engine_get_channels(engine);
  int64_t frame_length = akit_engine_get_frame_length(engine);
  int64_t tape_length = akit_engine_get_tape_length(
      engine); // frame_length * channels *
               // 2; // sample_rate * 4;//rame_length * channels * 4;
  float time_unit = (float)(frame_length) / (float)(sample_rate);

  printf("Engine info:\n");
  printf("sample_rate: %12.6f\n", sample_rate);
  printf("channels: %ld\n", channels);
  printf("frame_length: %ld\n", frame_length);
  printf("tape_length: %ld\n", tape_length);

  engine->tape =
    (float *)calloc(tape_length * AKIT_TAPE_LENGTH_MULTIPLIER, sizeof(float));


   engine->tape_fx =
      (float *)calloc(tape_length * AKIT_TAPE_LENGTH_MULTIPLIER, sizeof(float));

  akit_msleep(1);
  double wanted_delay = time_unit * 1000;

  while (akit_engine_is_running(engine)) {

    //if (akit_array_is_empty(&engine->clips)) {
    if (!akit_engine_is_playing(engine)) {
      akit_msleep(wanted_delay);
      akit_engine_clear_tape(engine);
      akit_driver_flush(&engine->driver);
      akit_driver_reset(&engine->driver);
      // akit_driver_wait(&engine->driver, 10);
    } else {
      pthread_mutex_lock(&engine->push_lock);
      akit_engine_clear_sounds(engine);
      pthread_mutex_unlock(&engine->push_lock);
    }

    // if (!engine->tape)
    //   continue;

    pthread_mutex_lock(&engine->push_lock);
    akit_engine_process(engine, &engine->tape[engine->frame], frame_length,
                        engine->time, engine->frame);
    pthread_mutex_unlock(&engine->push_lock);

    // akit_driver_wait(&engine->driver, 1000);

    akit_driver_wait(&engine->driver, 60);
    akit_driver_buffer_data(&engine->driver, &engine->tape[engine->frame],
                            frame_length);

    // akit_delay(time_unit * 500);

    double delay =
        (double)akit_driver_get_delay(&engine->driver) / (double)sample_rate;
    double got_delay = ((delay * 1000.0f) / 60.0f) * 2;
    akit_msleep(fmax(got_delay, wanted_delay));

    engine->time += time_unit;

    if (engine->frame + (frame_length * channels) >= (tape_length)) {
      engine->frame = 0;

      akit_engine_clear_tape(engine);

      if (got_delay > wanted_delay) {
        akit_driver_flush(&engine->driver);
        akit_driver_reset(&engine->driver);
      }

    } else {
      engine->frame += frame_length * channels;
    }


    if (engine->time >= (FLT_MAX-60.0f)) {
      engine->time = 0.0f;
    }

    // akit_driver_wait(&engine->driver, 1);
  }

  akit_driver_destroy(&engine->driver);

  printf("(Akit): Engine stopped.\n");
  pthread_exit(0);
  return 0;
}
