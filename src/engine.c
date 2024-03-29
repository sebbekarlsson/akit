#include <akit/constants.h>
#include <akit/engine.h>
#include <akit/macros.h>
#include <akit/plugin_limiter.h>
#include <akit/plugin_reverb.h>
#include <akit/utils.h>
#include <string.h>

int akit_engine_init(AkitEngine *engine, AkitEngineConfig config) {
  if (engine->initialized)
    return 0;
  engine->initialized = true;
  engine->config = config;
  engine->tracks = (AkitTrack *)calloc(AKIT_TRACK_CAP, sizeof(AkitTrack));
  engine->tracks_length = AKIT_TRACK_CAP;

  for (int64_t i = 0; i < engine->tracks_length; i++) {
    akit_track_init(&engine->tracks[i]);
  }
  // akit_array_init(&engine->clips, sizeof(AkitSoundClip *));
  engine->running = false;

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

bool akit_engine_is_running(AkitEngine *engine) {
  if (!engine)
    return false;
  if (engine->tape == 0) return false;
  return engine->running;
}

int akit_engine_push_sound(AkitEngine *engine, AkitSound sound) {
 if (!akit_engine_is_running(engine)) AKIT_WARNING_RETURN(0, stderr, "Akit Engine not running.\n");
  if (sound.data == 0)
    return 0;
  if (sound.length == 0)
    return 0;
  if (sound.gain <= 0.0f) {
    fprintf(stderr,
            "(Akit): No use in pushing a sound with no gain. Will not push.\n");
    return 0;
  }


  sound.sample_rate =
      OR(sound.sample_rate, akit_engine_get_sample_rate(engine));

  AkitSoundClip clip = {0}; // NEW(AkitSoundClip);
  clip.sound = sound;
  clip.finished = false;
  clip.cursor = 0;//clip.sound.cursor_start;
  clip.time_pushed = 0;
  clip.sound.gain = akit_clamp(sound.gain, 0.0f, M_PI);
  clip.fade_in = 0.0f;
  clip.fade_out = 0.0f;

  if (pthread_mutex_trylock(&engine->process_lock) != 0) {
    AKIT_WARNING_RETURN(0, stderr, "Failed to push sound (Mutex error).\n");
  }

  AkitTrack *track = akit_engine_get_available_track(engine);

  if (!track) {
    fprintf(stderr, "(Akit): Sound buffer is full, please try again later.\n");
    pthread_mutex_unlock(&engine->process_lock);
    return 0;
  }

  #if 0
  if (sound.reverb.mix > 0.00001f && sound.reverb.delay > 0.001f &&
      sound.no_processing == false) {
    if (track->plugins.length <= 0) {
      AkitPlugin reverb = {0};
      akit_plugin_reverb_init(&reverb, sound.reverb);
      reverb.config.position = sound.position;
      akit_track_push_plugin(track, reverb);
    }
  }
  #endif

  akit_track_push(track, clip);


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

int64_t akit_engine_get_tape_length(AkitEngine *engine) {
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

  int64_t length =
      akit_engine_get_tape_length(engine) * AKIT_TAPE_LENGTH_MULTIPLIER;

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

bool akit_engine_is_playing(AkitEngine *engine) {
  if (!engine)
    return false;
  if (!engine->initialized)
    return false;
  if (engine->tracks_length <= 0)
    return false;
  if (!engine->tracks)
    return false;
  if (!akit_engine_is_running(engine))
    return false;

  for (int64_t i = 0; i < engine->tracks_length; i++) {
    if (engine->tracks[i].clips.length > 0)
      return true;
  }

  return false;
}

AkitTrack *akit_engine_get_available_track(AkitEngine *engine) {
  if (!engine)
    return 0;
  if (!akit_engine_is_running(engine))
    return 0;
  if (!engine->initialized)
    AKIT_WARNING_RETURN(0, stderr, "Engine not initialized.\n");
  if (engine->tracks_length <= 0)
    return 0;
  if (engine->tracks == 0)
    return 0;

  for (int64_t i = 0; i < engine->tracks_length; i++) {
    AkitTrack *track = &engine->tracks[i];
    if (!track->initialized)
      continue;
    if (track->clips.length >= AKIT_CLIPS_PER_TRACK)
      continue;

    return track;
  }

  return 0;
}

int akit_engine_destroy(AkitEngine* engine) {
  if (!engine) return 0;
  if (akit_engine_is_running(engine) || akit_engine_is_playing(engine)) AKIT_WARNING_RETURN(0, stderr, "Please stop the Akit engine first.\n");

  if (engine->tracks != 0) {
    for (int64_t i = 0; i < engine->tracks_length; i++) {
      AkitTrack* track = &engine->tracks[i];
      akit_track_destroy(track);
    }

    free(engine->tracks);
    engine->tracks = 0;
  }

  engine->tracks_length = 0;

  if (engine->tape != 0) {
    free(engine->tape);
    engine->tape = 0;
  }

  engine->frame = 0;
  engine->time = 0.0f;
  pthread_mutex_destroy(&engine->push_lock);
  pthread_mutex_destroy(&engine->process_lock);
  engine->initialized = false;

  return 1;
}
