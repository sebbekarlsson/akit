#ifndef AKIT_PLUGIN_H
#define AKIT_PLUGIN_H
#include <mac/buffer.h>
#include <stdbool.h>
#include <vec3/vec3.h>

struct AKIT_ENGINE_STRUCT;
struct AKIT_PLUGIN_STRUCT;

typedef int (*AkitPluginCallback)(struct AKIT_ENGINE_STRUCT* engine, struct AKIT_PLUGIN_STRUCT* plugin, float* buffer, int64_t length, int64_t frame, double time);
typedef void (*AkitPluginDestroyCallback)(struct AKIT_PLUGIN_STRUCT* plugin);

typedef struct {
  AkitPluginCallback process_callback;
  AkitPluginDestroyCallback destroy_callback;
  Vector3 position;
} AkitPluginConfig;

typedef struct AKIT_PLUGIN_STRUCT {
  AkitPluginConfig config;
  float* buffer;
  int64_t buffer_length;
  int64_t ticks;
  void* user_ptr;
  double last_process;
  bool initialized;
} AkitPlugin;


MAC_DEFINE_BUFFER(AkitPlugin);

int akit_plugin_init(AkitPlugin* plugin, AkitPluginConfig cfg);

void akit_plugin_destroy(AkitPlugin* plugin);

int akit_plugin_process_block(struct AKIT_ENGINE_STRUCT* engine, AkitPlugin* plugin, float* buffer, int64_t length, int64_t frame, double time);

#endif
