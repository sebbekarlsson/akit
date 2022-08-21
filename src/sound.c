#include <akit/macros.h>
#include <akit/sound.h>
#include <akit/utils.h>

static float safediv(float a, float b) {
  if (ceilf(fabsf(b)) == 0.0f)
    b = 1.0f;
  return a / b;
}

void akit_sound_compute_gain(AkitSound sound, AkitListener listener,
                             float *left_gain, float *right_gain) {
  float left = 0.0f;
  float right = 0.0f;
  float center = 0.0f;

  float dm = OR(listener.distance_multiplier, 1.0f);

  Vector3 forward = vector3_unit(listener.forward);

  float dist = fabsf(vector3_distance3d(listener.position, sound.position)) * dm;
  float inv_dist = safediv(1.0f, dist);

  Vector3 left_right = vector3_unit(vector3_cross(forward, listener.up));
  Vector3 left_dir = vector3_scale(left_right, -1);
  Vector3 right_dir = vector3_scale(left_right, 1);
  Vector3 sound_dir =
      vector3_unit(vector3_sub(sound.position, listener.position));

  left_dir = vector3_unit(vector3_lerp_factor(left_dir, forward, 0.33f));
  right_dir = vector3_unit(vector3_lerp_factor(right_dir, forward, 0.33f));

  float left_dot = vector3_dot(left_dir, sound_dir);
  float right_dot = vector3_dot(right_dir, sound_dir);


  left = left_dot * inv_dist;
  right = right_dot * inv_dist;

  left += inv_dist * 0.33f;
  right += inv_dist * 0.33f;

  left = akit_clamp(left * sound.gain, 0.0f, 1.0f);
  right = akit_clamp(right * sound.gain, 0.0f, 1.0f);

  if (left < 0 || isnan(left) || isinf(left))
    left = 0.0f;
  if (right < 0 || isnan(right) || isinf(right))
    right = 0.0f;

  *left_gain = left;
  *right_gain = right;
}

void akit_sound_clip_destroy(AkitSoundClip *clip) {
  clip->sound.data = 0;
  //  if (clip->sound.data != 0) {
  //    free(clip->sound.data);
  //    clip->sound.data = 0;
  //  }

  clip->cursor = 0;
  clip->frame = 0;
  clip->time = 0;
  clip->sound.length = 0;
  clip->sound.duration = 0;
  clip->sound.sample_rate = 0;
}
