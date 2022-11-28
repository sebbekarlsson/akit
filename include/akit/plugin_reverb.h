#ifndef AKIT_PLUGIN_REVERB_H
#define AKIT_PLUGIN_REVERB_H
#include <akit/plugin.h>
#include <stdint.h>

typedef struct {
  int64_t delay_line_index;
} PluginReverbState;

int akit_plugin_reverb_init(AkitPlugin* plugin);

#endif
