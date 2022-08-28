#include <akit/macros.h>
#include <akit/sound.h>
#include <akit/utils.h>
#include <string.h>

static float safediv(float a, float b) {
  if (ceilf(fabsf(b)) == 0.0f)
    b = 1.0f;
  return a / b;
}

void akit_sound_compute_gain(AkitSound sound, AkitListener listener,
                             float *left_gain, float *right_gain) {
  float left = 0.0f;
  float right = 0.0f;

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
}


void akit_sound_clip_free(AkitSoundClip* clip) {
  if (clip->sound.data) {
    free(clip->sound.data);
    clip->sound.data = 0;
  }

  akit_sound_clip_destroy(clip);

  free(clip);
  clip = 0;
}

int akit_sound_clip_copy(
  AkitSoundClip src,
  AkitSoundClip* dest,
  int64_t start,
  int64_t length
) {
  if (!dest) return 0;
  if ((start + length) <= 0) return 0;
  if (!src.sound.data) return 0;
  if (!src.sound.length) return 0;


  dest->sound = src.sound;
  int64_t pos_start = start;
  if (length <= 0) return 0;


  float* buff = (float*)calloc(length*2, sizeof(float));
  memcpy(&buff[0], &src.sound.data[pos_start], length*2*sizeof(float));


  dest->sound.data = buff;
  dest->sound.length = length;
  dest->sound.duration = 4.0f;//(double)length / (double)src.sound.sample_rate;
  dest->sound.channels = src.sound.channels;
  dest->sound.position = src.sound.position;
  dest->sound.gain = src.sound.gain;
  dest->sound.sample_rate = src.sound.sample_rate;
  dest->sound.block_align = src.sound.block_align;
  dest->sound.world_info = src.sound.world_info;

  return 1;
}
int akit_sound_copy(
  AkitSound src,
  AkitSound* dest,
  int64_t start,
  int64_t length
) {
  if (!dest) return 0;
  if ((start + length) <= 0) return 0;
  if (!src.data) return 0;
  if (!src.length) return 0;
  if (length <= 0) return 0;

  *dest = src;


  int64_t max_length = src.length/2;

  int64_t clip_length = src.length;
  dest->data = (float*)calloc(length*2, sizeof(float));

  for (int64_t i = 0; i < length; i++) {
    if (src.channels <= 1) {
      if ((start + i >= (clip_length / sizeof(float)))) {
        break;
      }
    } else {
      if ((start + (i * 2) >= (clip_length / sizeof(float)))) {
        break;
      }
    }

    float *out_left = &dest->data[(i * 2)];
    float *out_right = &dest->data[(1 + i * 2)];

    float in_left = 0.0f;
    float in_right = 0.0f;

    if (src.channels <= 1) {
      in_left = src.data[start + i];
      in_right = src.data[start + i];
    } else {
      in_left = src.data[start + (i * 2)];
      in_right = src.data[start + (1 + i * 2)];
    }

    *out_left = in_left;
    *out_right = in_right;

  }

  dest->length = length * src.channels;
  dest->duration = 100.0f;//(double)length / (double)src.sound.sample_rate;
  dest->channels = src.channels;
  dest->position = src.position;
  dest->gain = src.gain;
  dest->sample_rate = src.sample_rate;
  dest->block_align = src.block_align;
  dest->world_info = src.world_info;

  return 1;
}
