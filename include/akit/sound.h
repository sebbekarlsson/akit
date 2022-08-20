#ifndef AKIT_SOUND_H
#define AKIT_SOUND_H
#include <stdint.h>
#include <vec3/vec3.h>
#include <akit/listener.h>

typedef struct {
  float* data;
  int64_t length;
  float sample_rate;
  Vector3 position;
  double duration;
  int64_t channels;
  int64_t block_align;
} AkitSound;


typedef struct {
  AkitSound sound;
  double time;
  int64_t frame;
  bool finished;
  int64_t cursor;
  double time_pushed;
} AkitSoundClip;


void akit_sound_compute_gain(AkitSound sound, AkitListener listener, float* left_gain, float* right_gain);

void akit_sound_clip_destroy(AkitSoundClip* clip);
#endif
