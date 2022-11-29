#ifndef AKIT_PLUGIN_REVERB_CONFIG_H
#define AKIT_PLUGIN_REVERB_CONFIG_H
#include <vec3/vec3.h>
typedef struct {
  float mix;
  float feedback;
  float delay;
  Vector2 pingpong_speed;
  float pingpong_amplitude;
} AkitPluginReverbConfig;
#endif
