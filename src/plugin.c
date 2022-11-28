#include <akit/plugin.h>
#include <akit/macros.h>
#include <akit/engine.h>

MAC_IMPLEMENT_BUFFER(AkitPlugin);

int akit_plugin_init(AkitPlugin* plugin, AkitPluginConfig cfg) {
  if (!plugin) return 0;
  if (plugin->initialized) return 1;
  plugin->initialized = true;
  plugin->buffer = 0;
  plugin->buffer_length = 0;
  plugin->config = cfg;
  plugin->last_process = 0;

  return 1;
}

int akit_plugin_process_block(AkitEngine* engine, AkitPlugin* plugin, float* buffer, int64_t length, int64_t frame, double time) {
  if (!plugin->config.process_callback) return 0;
  return plugin->config.process_callback(engine, plugin, buffer, length, frame, time);
}

void akit_plugin_destroy(AkitPlugin* plugin) {
  if (!plugin) return;
  plugin->config.process_callback = 0;

  if (plugin->config.destroy_callback != 0) {
    plugin->config.destroy_callback(plugin);
  }

  if (plugin->buffer != 0) {
    free(plugin->buffer);
  }

  plugin->buffer = 0;
}
