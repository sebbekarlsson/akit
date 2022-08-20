#include <akit/sound.h>
#include <akit/utils.h>
#include <akit/macros.h>


void akit_sound_compute_gain(AkitSound sound, AkitListener listener, float* left_gain, float* right_gain) {
  float left = 0.0f;
  float right = 0.0f;

  float far = OR(listener.far, 400.0f);

  Vector3 left_dir = vector3_scale(vector3_unit(vector3_cross(listener.forward, listener.up)), -1);
  Vector3 right_dir = vector3_scale(left_dir, -1);


  printf("Listener:\n");
  printf("Position: ");
  VEC3_PRINT(listener.position); printf("\n");

  float distance = fabsf(vector3_distance3d(sound.position, listener.position)) / far;
  if (distance <= 0.0f) distance = 1.0f;

  float dot_forward = vector3_dot(sound.position, listener.forward);
  float dot_left = vector3_dot(sound.position, left_dir) / (M_PI*2.0f);
  float dot_right = vector3_dot(sound.position, right_dir) / (M_PI*2.0f);


  left = fmaxf(0.0f, 1.0f - dot_left);
  right = fmaxf(0.0f, 1.0f - dot_right);


  left = (left*left) / (distance*distance);
  right = (right*right) / (distance*distance);


  left = akit_clamp(left, 0.0f, 1.0f);
  right = akit_clamp(right, 0.0f, 1.0f);

  printf("Left gain: %12.6f, right gain: %12.6f\n", left, right);

  *left_gain = left;
  *right_gain = right;
}

void akit_sound_clip_destroy(AkitSoundClip* clip) {
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
