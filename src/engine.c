#include <akit/engine.h>
#include <akit/macros.h>
#include <akit/constants.h>
#include <string.h>


int akit_engine_init(AkitEngine* engine, AkitEngineConfig config) {
  if (engine->initialized) return 0;
  engine->initialized = true;
  engine->config = config;
  akit_array_init(&engine->clips, sizeof(AkitSoundClip*));
  engine->stopped = true;

  return 1;
}

int akit_engine_start(AkitEngine* engine) {
  if (!engine->initialized) return 0;
  if (engine->running) return 0;

  if (pthread_mutex_init(&engine->push_lock, 0)) {
    fprintf(stderr, "(Akit): Failed to create mutex.\n");
    return 0;
  }

  if (pthread_mutex_init(&engine->process_lock, 0)) {
    fprintf(stderr, "(Akit): Failed to create mutex.\n");
    return 0;
  }

  if (pthread_create(&engine->thread_id, 0, akit_engine_thread, engine)) {
    fprintf(stderr, "(Akit): Failed to create thread.\n");
    return 0;
  }

  return 1;
}

int akit_engine_stop(AkitEngine* engine) {
  if (!engine->initialized) return 0;
  if (!engine->running) return 0;

  engine->stopped = true;
  engine->running = false;

  pthread_join(engine->thread_id, 0);

  return 1;
}

int akit_engine_push_sound(AkitEngine* engine, AkitSound sound) {
  if (!engine->running) return 0;
  if (engine->stopped) return 0;
  if (sound.data == 0) return 0;
  if (sound.length == 0) return 0;


  pthread_mutex_trylock(&engine->process_lock);
  if (engine->clips.length >= AKIT_MAX_SOUNDS) {
    fprintf(stderr, "(Akit): Sound buffer is full, please try again later.\n");
    pthread_mutex_unlock(&engine->process_lock);
    return 0;
  }
  pthread_mutex_unlock(&engine->process_lock);

  sound.sample_rate = OR(sound.sample_rate, akit_engine_get_sample_rate(engine));

  AkitSoundClip* clip = NEW(AkitSoundClip);
  clip->finished = false;
  clip->cursor = 0;

  AkitSound snd = sound;
  snd.data = (float*)calloc(sound.length, 1);
  memcpy(&snd.data[0], &sound.data[0], sound.length);

  clip->sound = snd;


  pthread_mutex_trylock(&engine->process_lock);
  akit_array_push(&engine->clips, clip);
  pthread_mutex_unlock(&engine->process_lock);

  return 1;
}


float akit_engine_get_sample_rate(AkitEngine* engine) {
  return OR(engine->driver.info.sample_rate, AKIT_SAMPLE_RATE);
}

int64_t akit_engine_get_frame_length(AkitEngine* engine) {
  return 512;
  return OR(engine->driver.info.frame_length, AKIT_FRAME_LENGTH);
}

int64_t akit_engine_get_channels(AkitEngine* engine) {
  return OR(engine->driver.info.channels, AKIT_CHANNELS);
}

int akit_engine_clear_tape(AkitEngine* engine) {
  if (!engine->initialized) return 0;
  if (!engine->tape) return 0;

  free(engine->tape);
  engine->tape = 0;

  return 1;
}


int akit_engine_set_listener(AkitEngine* engine, AkitListener listener) {
  if (!engine->initialized) return 0;
  engine->listener = listener;

  return 1;
}

AkitListener akit_engine_get_listener(AkitEngine engine) {
  return engine.listener;
}
