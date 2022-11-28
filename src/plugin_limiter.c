#include <akit/plugin_limiter.h>
#include <akit/engine.h>
#include <akit/constants.h>
#include <mif/utils.h>


static int akit_plugin_limiter_process_block(AkitEngine* engine, AkitPlugin* plugin, float* buffer, int64_t length, int64_t frame, double time) {

  for (int64_t i = 0; i < length; i++) {
    float *out_left = &buffer[(i * 2)];
    float *out_right = &buffer[(1 + i * 2)];

    float L = *out_left;
    float R = *out_right;



    *out_left = mif_clamp(L, AKIT_MIN_SAMPLE, AKIT_MAX_SAMPLE);
    *out_right = mif_clamp(R, AKIT_MIN_SAMPLE, AKIT_MAX_SAMPLE);
  }

  return 1;
}

int akit_plugin_limiter_init(AkitPlugin* plugin) {
  return akit_plugin_init(plugin, (AkitPluginConfig){ .process_callback = akit_plugin_limiter_process_block });
}
