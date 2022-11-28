#include <akit/engine.h>
#include <akit/macros.h>
#include <akit/plugin_reverb.h>
#include <akit/sound.h>
#include <mif/utils.h>
#include <stdlib.h>

static int akit_plugin_reverb_process_block(AkitEngine *engine,
                                            AkitPlugin *plugin, float *buffer,
                                            int64_t length, int64_t frame,
                                            double time) {

  if (!plugin->user_ptr) {
    return 0;
  }

  AkitListener listener = akit_engine_get_listener(*engine);
  Vector3 position = plugin->config.position;

  float left_gain = 1.0f;
  float right_gain = 1.0f;

  akit_sound_compute_gain(position, listener, &left_gain, &right_gain);

  AkitPluginReverbState *state = (AkitPluginReverbState *)plugin->user_ptr;
  AkitPluginReverbConfig cfg = state->config;

  float ping_left =
      mif_cos_n(time * cfg.pingpong_speed.x) * cfg.pingpong_amplitude;
  float ping_right =
      mif_sin_n(time * cfg.pingpong_speed.y) * cfg.pingpong_amplitude;

  left_gain *= fmaxf(0.0f, 1.0f - ping_left);
  right_gain *= fmaxf(0.0f, 1.0f - ping_right);

  float rate = akit_engine_get_sample_rate(engine);

  int64_t max_delay_seconds = 10;
  float delay_len_seconds = cfg.delay;

  int64_t delay_len_samples = delay_len_seconds * rate;

  float delay_wet_mix_left = cfg.mix * left_gain;
  float delay_wet_mix_right = cfg.mix * right_gain;
  float delay_dry_mix_left = fmaxf(0.0f, 1.0f - delay_wet_mix_left);
  float delay_dry_mix_right = fmaxf(0.0f, 1.0f - delay_wet_mix_right);

  float delay_feedback = OR(cfg.feedback, 0.45f);

  int64_t delay_line_size = rate * max_delay_seconds;

  if (!plugin->buffer) {
    plugin->buffer = (float *)calloc(delay_line_size, sizeof(float));
  }

  float avg = 0.0f;

  const float min_falloff = 0.96f;

  for (int64_t i = 0; i < length; i++) {
    float delayed_left = plugin->buffer[state->delay_line_index * 2];
    float delayed_right = plugin->buffer[1 + state->delay_line_index * 2];

    float in_left = buffer[i * 2];
    float in_right = buffer[1 + i * 2];

    buffer[i * 2] =
        (in_left * delay_dry_mix_left) + (delayed_left * delay_wet_mix_left);
    buffer[1 + i * 2] =
        (in_right * delay_dry_mix_right) + (delayed_left * delay_wet_mix_right);

    float next_left =
        (delay_feedback * (delayed_left + buffer[i * 2])) * min_falloff;
    float next_right =
        (delay_feedback * (delayed_right + buffer[1 + i * 2])) * min_falloff;

    plugin->buffer[state->delay_line_index * 2] = next_left;
    plugin->buffer[1 + state->delay_line_index * 2] = next_right;

    avg += (fabsf(next_left) + fabsf(next_right));
    avg += (fabsf(in_left) + fabsf(in_right));

    if (state->delay_line_index++ >= delay_len_samples) {
      state->delay_line_index = 0;
    }
  }

  avg /= (((float)length) / 2.0f);

  return avg >= 0.00001f || (time <= 1.0f) || plugin->ticks < 256;
}

static void akit_plugin_reverb_destroy(AkitPlugin *plugin) {
  if (plugin->user_ptr != 0) {
    free(plugin->user_ptr);
  }
}

int akit_plugin_reverb_init(AkitPlugin *plugin, AkitPluginReverbConfig cfg) {
  if (!akit_plugin_init(
          plugin, (AkitPluginConfig){
                      .process_callback = akit_plugin_reverb_process_block,
                      .destroy_callback = akit_plugin_reverb_destroy}))
    return 0;
  plugin->user_ptr = NEW(AkitPluginReverbState);

  AkitPluginReverbState *state = (AkitPluginReverbState *)plugin->user_ptr;
  state->config = cfg;

  return plugin->user_ptr != 0;
}
