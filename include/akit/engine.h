#ifndef AKIT_ENGINE_H
#define AKIT_ENGINE_H
#include <akit/driver.h>
#include <akit/sound.h>
#include <akit/array.h>
#include <akit/listener.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <akit/constants.h>
#include <akit/track.h>
#include <mif/fft.h>

typedef struct {
  AkitDriverConfig driver_config;
  int64_t max_sounds;
  bool normalize_stereo;
} AkitEngineConfig;

typedef struct AKIT_ENGINE_STRUCT {
  bool initialized;

  volatile bool running;

  volatile AkitListener listener;

  AkitDriver driver;
  AkitEngineConfig config;
  AkitTrack* tracks;
  volatile int64_t tracks_length;
  //AkitArray clips;

  float* tape;
  float* tape_fx;


  pthread_t thread_id;
  pthread_mutex_t push_lock;
  pthread_mutex_t process_lock;

  double time;
  int64_t frame;
} AkitEngine;


int akit_engine_init(AkitEngine* engine, AkitEngineConfig config);

int akit_engine_start(AkitEngine* engine);

int akit_engine_stop(AkitEngine* engine);

int akit_engine_push_sound(AkitEngine* engine, AkitSound sound);

void* akit_engine_thread(void* ptr);

float akit_engine_get_sample_rate(AkitEngine* engine);

int64_t akit_engine_get_frame_length(AkitEngine* engine);

int64_t akit_engine_get_channels(AkitEngine* engine);

int64_t akit_engine_get_tape_length(AkitEngine* engine);

int akit_engine_clear_tape(AkitEngine* engine);

int akit_engine_set_listener(AkitEngine* engine, AkitListener listener);

int akit_engine_clear_sounds(AkitEngine* engine);

AkitListener akit_engine_get_listener(AkitEngine engine);


bool akit_engine_is_playing(AkitEngine* engine);

bool akit_engine_is_running(AkitEngine* engine);


AkitTrack* akit_engine_get_available_track(AkitEngine* engine);

#endif
