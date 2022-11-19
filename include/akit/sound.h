#ifndef AKIT_SOUND_H
#define AKIT_SOUND_H
#include <stdint.h>
#include <vec3/vec3.h>
#include <akit/listener.h>
#include <stdbool.h>

typedef struct {
  Vector3 size;
  float roughness;
} AkitWorldInfo;

typedef struct {
  float* data;
  int64_t length;
  float sample_rate;
  Vector3 position;
  Vector3 velocity;
  AkitWorldInfo world_info;
  double duration;
  int64_t channels;
  int64_t block_align;
  int64_t cursor_start;
  int64_t cursor_end;
  double start_time;
  bool ignore_full;
  float gain;
  bool no_processing;
  const char* name;
  float fade_time;
} AkitSound;


typedef struct {
  AkitSound sound;
  double time;
  double time_left_of_ambience;
  int64_t frame;
  bool finished;
  int64_t cursor;
  double time_pushed;
  double last_process;


  float fade_in;
  float fade_out;

  char* name;


} AkitSoundClip;


void akit_sound_compute_gain(AkitSoundClip* clip, AkitListener listener, float* left_gain, float* right_gain);

float akit_sound_compute_fader(AkitSoundClip* clip);


void akit_sound_clip_destroy(AkitSoundClip* clip);
#endif
