#include <akit/constants.h>
#include <akit/engine.h>
#include <akit/macros.h>
#include <akit/utils.h>
#include <akit/plugin_limiter.h>
#include <akit/plugin_reverb.h>
#include <string.h>

int akit_engine_init(AkitEngine *engine, AkitEngineConfig config) {
  if (engine->initialized)
    return 0;
  engine->initialized = true;
  engine->config = config;
  mac_AkitTrack_buffer_init(&engine->tracks);
  //akit_array_init(&engine->clips, sizeof(AkitSoundClip *));
  engine->running = false;


  hashy_map_init(&engine->sounds_playing, OR(config.max_sounds, 128));

  return 1;
}

int akit_engine_start(AkitEngine *engine) {
  if (!engine->initialized)
    return 0;
  if (engine->running)
    return 0;

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

int akit_engine_stop(AkitEngine *engine) {
  if (!engine->initialized)
    return 0;
  if (!engine->running)
    return 0;

  engine->running = false;

  pthread_join(engine->thread_id, 0);

  return 1;
}

bool akit_engine_is_running(AkitEngine* engine) {
  if (!engine) return false;
  return engine->running;
}

int akit_engine_push_sound(AkitEngine *engine, AkitSound sound) {
  if (!engine->running)
    return 0;
  if (sound.data == 0)
    return 0;
  if (sound.length == 0)
    return 0;
  if (sound.gain <= 0.0f) {
    fprintf(stderr,
            "(Akit): No use in pushing a sound with no gain. Will not push.\n");
    return 0;
  }

  #if 0
  pthread_mutex_trylock(&engine->process_lock);

  int64_t max_sounds = akit_engine_get_sound_limit(*engine);

  if (!sound.ignore_full) {
    if (engine->clips.length >= max_sounds) {
      fprintf(stderr, "(Akit): Sound buffer is full, please try again later.\n");
      pthread_mutex_unlock(&engine->process_lock);
      return 0;
    }
  }
  pthread_mutex_unlock(&engine->process_lock);
  #endif

  sound.sample_rate =
      OR(sound.sample_rate, akit_engine_get_sample_rate(engine));

  AkitSoundClip clip = {0};//NEW(AkitSoundClip);
  clip.sound = sound;
  clip.finished = false;
  clip.cursor = clip.sound.cursor_start;
  clip.time_pushed = 0;
  clip.sound.gain = akit_clamp(sound.gain, 0.0f, M_PI);
  clip.fade_in = 0.0f;
  clip.fade_out = 0.0f;

  pthread_mutex_trylock(&engine->process_lock);


  AkitTrack* track = 0;

  if (sound.reverb.mix > 0.00001f && sound.reverb.delay > 0.01f && sound.no_processing == false) {
    AkitTrack next_track = {0};
    akit_track_init(&next_track);
    track = mac_AkitTrack_buffer_push(&engine->tracks, next_track);


    if (track->plugins.length <= 0) {
      AkitPlugin reverb = {0};
      akit_plugin_reverb_init(&reverb, sound.reverb);
      reverb.config.position = sound.position;
      akit_track_push_plugin(track, reverb);
    }

  }

  track = OR(track, akit_engine_get_available_track(engine));

  if (!track) {
      fprintf(stderr, "(Akit): Sound buffer is full, please try again later.\n");
      pthread_mutex_unlock(&engine->process_lock);
      return 0;
  }



  akit_track_push(track, clip);

 // if (clip->sound.name != 0) {
  //  clip->name = strdup(clip->sound.name);
   // hashy_map_set(&engine->sounds_playing, clip->sound.name, clip);
 // }
  //akit_array_push(&engine->clips, clip);
  pthread_mutex_unlock(&engine->process_lock);

  return 1;
}

float akit_engine_get_sample_rate(AkitEngine *engine) {
  return OR(engine->driver.info.sample_rate, AKIT_SAMPLE_RATE);
}

int64_t akit_engine_get_frame_length(AkitEngine *engine) {
  return OR(engine->driver.info.frame_length,
            OR(engine->config.driver_config.frame_length, AKIT_FRAME_LENGTH));
}

int64_t akit_engine_get_tape_length(AkitEngine* engine) {
  return akit_engine_get_sample_rate(engine) * 6;
}

int64_t akit_engine_get_channels(AkitEngine *engine) {
  return OR(engine->driver.info.channels, AKIT_CHANNELS);
}

int akit_engine_clear_tape(AkitEngine *engine) {
  if (!engine->initialized)
    return 0;
  if (!engine->tape)
    return 0;

  int64_t length = akit_engine_get_tape_length(engine) * AKIT_TAPE_LENGTH_MULTIPLIER;

  memset(&engine->tape[0], 0, length * sizeof(float));

  return 1;
}

int akit_engine_set_listener(AkitEngine *engine, AkitListener listener) {
  if (!engine->initialized)
    return 0;
  engine->listener = listener;

  return 1;
}

AkitListener akit_engine_get_listener(AkitEngine engine) {
  return engine.listener;
}

int akit_engine_clear_sounds(AkitEngine *engine) {
  #if 0
  if (!engine->initialized)
    return 0;
  if (!engine->clips.length)
    return 0;

  double now = engine->time;

  for (int64_t i = 0; i < engine->clips.length; i++) {
    AkitSoundClip *clip = (AkitSoundClip *)engine->clips.items[i];

    double time_pushed = clip->time_pushed;
    double diff = now - time_pushed;

    if (clip->cursor > 0 && (clip->finished || diff >= AKIT_MAX_SOUND_LENGTH)) {
      akit_sound_clip_destroy(clip);
      akit_array_remove(&engine->clips, clip, 0);
      free(clip);
      clip = 0;
    }
  }
  #endif

  return 1;
}

int64_t akit_engine_get_sound_limit(AkitEngine engine) {
  return OR(engine.config.max_sounds, AKIT_MAX_SOUNDS);
}

bool akit_engine_sound_is_playing(AkitEngine* engine, const char* name) {
  if (!engine) return false;
  if (!name) return false;
  if (engine->tracks.length <= 0) return false;
  if (!engine->sounds_playing.initialized) return false;
  if (!akit_engine_is_running(engine)) return false;

  return true;
  //return hashy_map_get(&engine->sounds_playing, name) != 0;
}

int akit_engine_stop_sound(AkitEngine* engine, const char* name) {
  if (!engine) return 0;
  if (!name) return 0;
  if (engine->tracks.length <= 0) return 0;
  if (!engine->sounds_playing.initialized) return 0;
  if (!akit_engine_is_running(engine)) return 0;
  AkitSoundClip* clip = hashy_map_get(&engine->sounds_playing, name);
  if (!clip) return 0;



  pthread_mutex_trylock(&engine->process_lock);
  clip->finished = true;
  pthread_mutex_unlock(&engine->process_lock);

  return 1;
}

bool akit_engine_is_playing(AkitEngine* engine) {
  if (!engine) return false;
  if (!engine->initialized) return false;
  if (engine->tracks.length <= 0) return false;

  return akit_engine_is_running(engine);
}

int akit_engine_update_sound(AkitEngine* engine, const char* name, AkitSound update) {
  if (!engine) return 0;
  if (!engine->initialized) return 0;
  if (engine->tracks.length <= 0) return 0;
  if (!name) return 0;



  AkitSoundClip* clip = hashy_map_get(&engine->sounds_playing, name);
  if (!clip) return 0;

  pthread_mutex_trylock(&engine->process_lock);

  clip->sound.gain = update.gain;
  clip->sound.position = update.position;

  pthread_mutex_unlock(&engine->process_lock);

  return 1;
}

AkitTrack* akit_engine_get_available_track(AkitEngine* engine) {
  if (!engine || !engine->initialized) return 0;
  if (engine->tracks.length <= 0) {
    if (!engine->tracks.initialized) {
      mac_AkitTrack_buffer_init(&engine->tracks);
    }
    AkitTrack track = {0};
    akit_track_init(&track);
    mac_AkitTrack_buffer_push(&engine->tracks, track);
  }

  int64_t max_sounds = akit_engine_get_sound_limit(*engine);

  for (int64_t i = 0; i < engine->tracks.length; i++) {
    AkitTrack* track = &engine->tracks.items[i];
    if (track->clips.length >= max_sounds) continue;

    return track;
  }

  return 0;
}
