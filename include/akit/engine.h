#ifndef AKIT_ENGINE_H
#define AKIT_ENGINE_H
#include <akit/driver.h>
#include <akit/sound.h>
#include <akit/array.h>
#include <akit/listener.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <hashy/hashy.h>

typedef struct {
  AkitDriverConfig driver_config;
  int64_t max_sounds;
  bool normalize_stereo;
} AkitEngineConfig;

typedef struct {
  bool initialized;

  volatile bool running;

  volatile AkitListener listener;

  AkitDriver driver;
  AkitEngineConfig config;
  AkitArray clips;

  float* tape;
  float* tape_fx;

  HashyMap sounds_playing;

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

int64_t akit_engine_get_sound_limit(AkitEngine engine);

bool akit_engine_sound_is_playing(AkitEngine* engine, const char* name);

int akit_engine_stop_sound(AkitEngine* engine, const char* name);

bool akit_engine_is_playing(AkitEngine* engine);

bool akit_engine_is_running(AkitEngine* engine);

int akit_engine_update_sound(AkitEngine* engine, const char* name, AkitSound update);

#endif
