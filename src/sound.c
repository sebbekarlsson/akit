#include <akit/macros.h>
#include <akit/sound.h>
#include <akit/utils.h>

MAC_IMPLEMENT_BUFFER(AkitSoundClip);
MAC_IMPLEMENT_LIST(AkitSoundClip);


static float safediv(float a, float b) {
  if (ceilf(fabsf(b)) == 0.0f) {
    b = 1.0f;
  }
  return a / b;
}


float akit_sound_compute_fader(AkitSoundClip* clip) {
  if (!clip) return 0.0f;

  AkitSound sound = clip->sound;

  if (sound.fade_time <= 0.0f) return 1.0f;

  float fade_out_start = fmaxf(0.0f, sound.duration - sound.fade_time);
  float fade_out_end = sound.duration;
  float fade_in = akit_clamp(clip->time / sound.fade_time, 0.0f, 1.0f);
  float fade_out = 1.0f - akit_clamp((clip->time - fade_out_start) / sound.fade_time, 0.0f, 1.0f);
  float fader = (fade_in * fade_out);

  return fader;
}

void akit_sound_compute_gain(Vector3 position, AkitListener listener, float* left_gain, float* right_gain) {

  float left = 0.0f;
  float right = 0.0f;

  float dm = OR(listener.distance_multiplier, 1.0f);

  Vector3 forward = vector3_unit(listener.forward);

  float dist = fabsf(vector3_distance3d(listener.position, position)) * dm;

  if (dist <= 0.0001f) {
    *left_gain = 1.0f;
    *right_gain = 1.0f;
    return;
  }

  float inv_dist = safediv(1.0f, dist);

  Vector3 left_right = vector3_unit(vector3_cross(forward, listener.up));
  Vector3 left_dir = vector3_scale(left_right, -1);
  Vector3 right_dir = vector3_scale(left_right, 1);
  Vector3 sound_dir =
      vector3_unit(vector3_sub(position, listener.position));

  left_dir = vector3_unit(vector3_lerp_factor(left_dir, forward, listener.ear_forward_bend));
  right_dir = vector3_unit(vector3_lerp_factor(right_dir, forward, listener.ear_forward_bend));

  float left_dot = vector3_dot(left_dir, sound_dir);
  float right_dot = vector3_dot(right_dir, sound_dir);


  left = left_dot * inv_dist;
  right = right_dot * inv_dist;

  left += inv_dist * (listener.ear_forward_bend / 2.0f);
  right += inv_dist * (listener.ear_forward_bend  / 2.0f);

  left = akit_clamp(left, 0.0f, 1.0f);
  right = akit_clamp(right, 0.0f, 1.0f);

  if (left < 0 || isnan(left) || isinf(left))
    left = 0.0f;
  if (right < 0 || isnan(right) || isinf(right))
    right = 0.0f;



  *left_gain = left;
  *right_gain = right;
}

void akit_sound_clip_destroy(AkitSoundClip *clip) {
  /*
   * We assume who ever gave us the data will free it.
   */
  clip->sound.data = 0;
  clip->cursor = 0;
  clip->frame = 0;
  clip->time = 0;
  clip->sound.length = 0;
  clip->sound.duration = 0;
  clip->sound.sample_rate = 0;

  if (clip->name != 0) {
    free(clip->name);
  }
  clip->name = 0;
}


bool akit_sound_clip_wants_randomness(AkitSoundClip* clip) {
  return clip->sound.random_seed > VEC3_TINY_FLOAT && clip->sound.random_factor > VEC3_TINY_FLOAT && clip->sound.duration >= VEC3_TINY_FLOAT;
}
