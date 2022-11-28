#include <akit/engine.h>
#include <akit/macros.h>
#include <akit/plugin_reverb.h>
#include <mif/utils.h>
#include <stdlib.h>

static int akit_plugin_reverb_process_block(AkitEngine *engine,
                                            AkitPlugin *plugin, float *buffer,
                                            int64_t length, int64_t frame,
                                            double time) {

  if (!plugin->user_ptr) {
    return 0;
  }

  PluginReverbState *state = (PluginReverbState *)plugin->user_ptr;

  float rate = akit_engine_get_sample_rate(engine);

  int64_t max_delay_seconds = 5;
  float delay_len_seconds = 1.0f;

  int64_t delay_len_samples = (int64_t)delay_len_seconds * rate;

  float delay_wet_mix = 0.4f;
  float delay_dry_mix = fmaxf(0.0f, 1.0f - delay_wet_mix);

  float delay_feedback = 0.45f;

  int64_t delay_line_size = rate * max_delay_seconds;

  if (!plugin->buffer) {
    plugin->buffer = (float *)calloc(delay_line_size, sizeof(float));
  }

  for (int64_t i = 0; i < length; i++) {
    float delayed_left = plugin->buffer[state->delay_line_index * 2];
    float delayed_right = plugin->buffer[1 + state->delay_line_index * 2];

    buffer[i * 2] =
        (buffer[i * 2] * delay_dry_mix) + (delayed_left * delay_wet_mix);
    buffer[1 + i * 2] =
        (buffer[1 + i * 2] * delay_dry_mix) + (delayed_left * delay_wet_mix);

    plugin->buffer[state->delay_line_index * 2] =
        delay_feedback * (delayed_left + buffer[i * 2]);
    plugin->buffer[1 + state->delay_line_index * 2] =
        delay_feedback * (delayed_right + buffer[1 + i * 2]);

    if (state->delay_line_index++ >= delay_len_samples) {
      state->delay_line_index = 0;
    }
  }

  return 1;
}

static void akit_plugin_reverb_destroy(AkitPlugin *plugin) {
  if (plugin->user_ptr != 0) {
    free(plugin->user_ptr);
  }
}

int akit_plugin_reverb_init(AkitPlugin *plugin) {
  if (!akit_plugin_init(
          plugin, (AkitPluginConfig){
                      .process_callback = akit_plugin_reverb_process_block,
                      .destroy_callback = akit_plugin_reverb_destroy}))
    return 0;
  plugin->user_ptr = NEW(PluginReverbState);
  return plugin->user_ptr != 0;
}
