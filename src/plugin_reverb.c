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

  float avg = 0.0f;

  for (int64_t i = 0; i < length; i++) {
    float delayed_left = plugin->buffer[state->delay_line_index * 2];
    float delayed_right = plugin->buffer[1 + state->delay_line_index * 2];


    buffer[i * 2] =
        (buffer[i * 2] * delay_dry_mix) + (delayed_left * delay_wet_mix);
    buffer[1 + i * 2] =
        (buffer[1 + i * 2] * delay_dry_mix) + (delayed_left * delay_wet_mix);


    float next_left = delay_feedback * (delayed_left + buffer[i * 2]);
    float next_right = delay_feedback * (delayed_right + buffer[1 + i * 2]);

    plugin->buffer[state->delay_line_index * 2] = next_left;
    plugin->buffer[1 + state->delay_line_index * 2] = next_right;

    avg += (fabsf(next_left) + fabsf(next_right)) / 2.0f;

    if (state->delay_line_index++ >= delay_len_samples) {
      state->delay_line_index = 0;
    }
  }

  avg /= (float)length;

  return avg >= 0.00001f || (time <= 1.0f);
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
