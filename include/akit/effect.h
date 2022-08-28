#ifndef AKIT_EFFECT_H
#define AKIT_EFFECT_H
#include <akit/sound.h>
#include <stdbool.h>


typedef enum {
AKIT_EFFECT_REVERB,
AKIT_EFFECT_DELAY
} AkitEffectType;


typedef struct {
  AkitEffectType type;
  float* buffer;
  int64_t length;
  int64_t channels;
  bool initialized;
} AkitEffect;


void akit_effect_init(AkitEffect* effect, int64_t length, int64_t channels);

void akit_effect_process(
  AkitEffect* effect,
  AkitSoundClip* source_clip,
  float* buffer,
  int64_t length
);
#endif
