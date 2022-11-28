#include <akit/track.h>
#include <akit/macros.h>
#include <akit/engine.h>
#include <akit/constants.h>
#include <akit/dsp.h>

MAC_IMPLEMENT_BUFFER(AkitTrack);


int akit_track_init(AkitTrack* track) {
  if (!track) return 0;
  if (track->initialized) return 1;
  track->initialized = true;
  mac_AkitSoundClip_buffer_init(&track->clips);
  mac_AkitPlugin_buffer_init(&track->plugins);
  return 1;
}

int akit_track_push(AkitTrack* track, AkitSoundClip clip) {
  if (!track) return 0;
  if (!track->initialized) AKIT_WARNING_RETURN(0, stderr, "Track Not initialized.\n");
  mac_AkitSoundClip_buffer_push(&track->clips, clip);
  return 1;
}

int akit_track_push_plugin(AkitTrack* track, AkitPlugin plugin) {
  if (!track) return 0;
  if (!track->initialized) AKIT_WARNING_RETURN(0, stderr, "Track Not initialized.\n");
  if (!plugin.initialized) AKIT_WARNING_RETURN(0, stderr, "Plugin not initialized.\n");

  mac_AkitPlugin_buffer_push(&track->plugins, plugin);

  return 1;
}


int akit_track_process_clip(AkitEngine* engine, AkitTrack* track, AkitSoundClip* clip, float* buffer, int64_t length, int64_t frame, double time) {
  int64_t clip_length = clip->sound.length;
  int64_t clip_channels = OR(clip->sound.channels, 1);
  int64_t tape_length = akit_engine_get_tape_length(engine);


  int64_t cursor_end = clip->sound.cursor_end;
  if (cursor_end <= 0) {
    cursor_end = INT64_MAX;
  }

  if (clip->time >= (clip->sound.duration * 2) ||
      (clip->cursor + length) >= (clip_length) || clip->cursor >= cursor_end) {
    clip->finished = true;
    return 0;
  }



  float sample_rate =
      OR(clip->sound.sample_rate, akit_engine_get_sample_rate(engine));

  float *clip_buffer = &clip->sound.data[clip->cursor];


  AkitListener listener = akit_engine_get_listener(*engine);

  float left_gain = 0.0f;
  float right_gain = 0.0f;

  if (clip->sound.no_processing) {
    left_gain = 1.0f;
    right_gain = 1.0f;
  } else {
    akit_sound_compute_gain(clip, listener, &left_gain, &right_gain);
  }

  if (clip->sound.fade_time > 0.0f) {
    float fader = akit_sound_compute_fader(clip);
    left_gain *= fader;
    right_gain *= fader;
  }

  left_gain *= clip->sound.gain;
  right_gain *= clip->sound.gain;

  //  int64_t samples_avail = MAX(0, (clip_length / sizeof(float)) -
  //  clip->cursor);


  //int64_t future_delay = sample_rate/2;

  if (clip_channels >= 2) {
    for (int64_t i = 0; i < length; i++) {
      if ((clip->cursor + (i * 2) >= (clip_length / sizeof(float))) || clip->cursor >= cursor_end) {
        clip->finished = true;
        break;
      }

      float *out_left = &buffer[(i * 2)%tape_length];
      float *out_right = &buffer[(1 + i * 2)%tape_length];

    //  float *out_left_future = &buffer[(future_delay+(i * 2))%tape_length];
    //  float *out_right_future = &buffer[(future_delay+(1 + i * 2))%tape_length];

      float in_left = clip_buffer[i * 2];
      float in_right = clip_buffer[1 + i * 2];

      akit_dsp_process(engine, clip, &in_left, &in_right);

      *out_left += (in_left * left_gain);
      *out_right += (in_right * right_gain);

    //  *out_left_future += (in_left * left_gain);
     // *out_right_future += (in_right * right_gain);

      *out_left = akit_dsp_get_corrected_sample(*out_left);
      *out_right = akit_dsp_get_corrected_sample(*out_right);

  //    *out_left_future = akit_dsp_get_corrected_sample(*out_left_future);
   //   *out_right_future = akit_dsp_get_corrected_sample(*out_right_future);
    }

  } else {

    for (int64_t i = 0; i < length; i++) {
      if ((clip->cursor + i >= (clip_length / sizeof(float))) || clip->cursor >= cursor_end) {
        clip->finished = true;
        break;
      }

      float *out_left = &buffer[i * 2];
      float *out_right = &buffer[1 + i * 2];

    //  float *out_left_future = &buffer[(future_delay+(i * 2))%tape_length];
     // float *out_right_future = &buffer[(future_delay+(1 + i * 2))%tape_length];

      float sample = clip_buffer[i];

      akit_dsp_process(engine, clip, &sample, &sample);

      *out_left += (sample * left_gain);
      *out_right += (sample * right_gain);

     // *out_left_future += (sample * left_gain);
      //*out_right_future += (sample * right_gain);


      *out_left = akit_dsp_get_corrected_sample(*out_left);
      *out_right = akit_dsp_get_corrected_sample(*out_right);

      //*out_left_future = akit_dsp_get_corrected_sample(*out_left_future);
      //*out_right_future = akit_dsp_get_corrected_sample(*out_right_future);
    }
  }

  clip->cursor += (length * clip_channels) + clip->timeshift;

  int64_t divisor = clip_channels <= 1 ? 1 : 2;

  clip->time = (1.0f / sample_rate) * ((double)clip->cursor / (double)divisor);

  if (clip->time >= (clip->sound.duration * 2) ||
      (clip->cursor) >= (clip_length) || clip->cursor >= cursor_end) {
    clip->finished = true;
  }

  clip->sound.position = vector3_add(clip->sound.position, clip->sound.velocity);

  return clip->finished == false;
}

int akit_track_process_block(struct AKIT_ENGINE_STRUCT* engine, AkitTrack* track, float* buffer, int64_t length, int64_t frame, double time) {
  for (int64_t i = 0; i < track->clips.length; i++) {
    AkitSoundClip* clip = &track->clips.items[i];

    if (!akit_track_process_clip(engine, track, clip, buffer, length, frame, time)) {
      akit_sound_clip_destroy(clip);
      mac_AkitSoundClip_buffer_remove(&track->clips, i);
      continue;
    }
  }

  for (int64_t i = 0; i < track->plugins.length; i++) {
    AkitPlugin* plugin = &track->plugins.items[i];

    if (!akit_plugin_process_block(engine, plugin, buffer, length, frame, time)) {
      akit_plugin_destroy(plugin);
      mac_AkitPlugin_buffer_remove(&track->plugins, i);
      continue;
    }
  }

  return track->clips.length > 0;
}

void akit_track_destroy(AkitTrack* track) {
  if (!track) return;
  mac_AkitSoundClip_buffer_clear(&track->clips);

  for (int64_t i = 0; i < track->plugins.length; i++) {
    AkitPlugin* plugin = &track->plugins.items[i];
    akit_plugin_destroy(plugin);
  }

  mac_AkitPlugin_buffer_clear(&track->plugins);
}
