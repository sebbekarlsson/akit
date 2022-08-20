#include <akit/sound.h>
#include <akit/utils.h>



void akit_sound_compute_gain(AkitSound sound, AkitListener listener, float* left_gain, float* right_gain) {
  float left = 0.0f;
  float right = 0.0f;


  Vector3 left_dir = vector3_scale(vector3_unit(vector3_cross(listener.forward, listener.up)), -1);
  Vector3 right_dir = vector3_scale(left_dir, -1);


  float distance = fabsf(vector3_distance3d(sound.position, listener.position));
  if (distance <= 0.0f) distance = 1.0f;

  float dot_forward = vector3_dot(sound.position, listener.forward);
  float dot_left = vector3_dot(sound.position, left_dir);
  float dot_right = vector3_dot(sound.position, right_dir);


  left = 1.0f - dot_left;
  right = 1.0f - dot_right;


  left = (left*left) / (distance*distance);
  right = (right*right) / (distance*distance);


  *left_gain = akit_clamp(left, 0.0f, 1.0f);
  *right_gain = akit_clamp(right, 0.0f, 1.0f);
}

void akit_sound_clip_destroy(AkitSoundClip* clip) {
  if (clip->sound.data != 0) {
    free(clip->sound.data);
    clip->sound.data = 0;
  }

  clip->cursor = 0;
  clip->frame = 0;
  clip->time = 0;
  clip->sound.length = 0;
  clip->sound.duration = 0;
  clip->sound.sample_rate = 0;
}
