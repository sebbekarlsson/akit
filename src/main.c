#include <akit/engine.h>
#include <akit/sleep.h>
#include <stdio.h>
#include <waves/wav.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

  Wave wav = {0};
  WaveOptions wav_options = {0};
  wav_options.convert_to_float = true;
  if (!wav_read(&wav, argv[1], wav_options)) {
    fprintf(stderr, "Unable to read wav!\n");
    return 0;
  }


  AkitDriverConfig config = {0};
  config.sample_rate = 44100;
  config.frame_length = 512;
  config.type = AKIT_DRIVER_TYPE_ASOUND;

  AkitEngine engine = {0};
  akit_engine_init(&engine, (AkitEngineConfig){ .driver_config = config });
  akit_engine_set_listener(&engine, (AkitListener){
    .forward = VEC3(0, 0, 1),
    .up = VEC3(0, 1, 0),
    .position = VEC3(0, 0, 0)
  });

  akit_engine_start(&engine);

  akit_msleep(1000);

  akit_engine_push_sound(&engine, (AkitSound){
    .data = wav.data,
    .length = wav.length,
    .sample_rate = wav.header.sample_rate,
    .position = VEC3(-16.0f, 0, 1.0f),
    .duration = fmaxf(wav.duration, 0.5f),
    .channels = wav.header.channels,
    .block_align = wav.header.block_align
  });

  akit_msleep(3000);


  akit_engine_push_sound(&engine, (AkitSound){
    .data = wav.data,
    .length = wav.length,
    .sample_rate = wav.header.sample_rate,
    .position = VEC3(16.0f, 0.0f, 1.0f),
    .duration = fmaxf(wav.duration, 0.5f),
    .channels = wav.header.channels,
    .block_align = wav.header.block_align
  });


  akit_msleep(1500 * (1.0f + wav.duration));

  akit_engine_stop(&engine);


  if (wav.data) {
    free(wav.data);
    wav.data = 0;
  }

  return 0;
}
