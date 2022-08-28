#include <akit/effect.h>

void akit_effect_init(AkitEffect* effect, int64_t length, int64_t channels) {
  if (effect->initialized) return;
  effect->initialized = true;


  effect->channels = channels;
  effect->length = length;
  effect->buffer = (float*)calloc(length*channels, sizeof(float));
}

void akit_effect_process(
  AkitEffect* effect,
  AkitSoundClip* source_clip,
  float* buffer,
  int64_t length
) {

}
