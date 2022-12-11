#include <akit/engine.h>
#include <akit/sleep.h>
#include <akit/dsp.h>
#include <akit/utils.h>
#include <stdio.h>
#include <waves/wav.h>
#include <stdlib.h>
#include <string.h>


int akit_main_slice(int argc, char *argv[]) {
  if (argc < 4) return 0;


    Wave wav = {0};
  WaveOptions wav_options = {0};
  wav_options.convert_to_float = true;
  if (!wav_read(&wav, argv[2], wav_options)) {
    fprintf(stderr, "Unable to read wav!\n");
    return 0;
  }

  akit_dsp_extract_onsets(wav, argv[3]);

  return 0;
}


int main(int argc, char *argv[]) {

  if (argc < 2) goto nothing_specified;

  if (strcmp(argv[1], "slice") == 0) {
    return akit_main_slice(argc, argv);
  } else {
    nothing_specified:
    fprintf(stderr, "Please specify an action to perform.\n");
    return 1;
  }

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
  akit_engine_init(&engine, (AkitEngineConfig){ .driver_config = config, .max_sounds = 10, .normalize_stereo = true });
  akit_engine_set_listener(&engine, (AkitListener){
    .forward = VEC3(0, 0, 1),
    .up = VEC3(0, 1, 0),
    .position = VEC3(0, 0, 0)
  });

  akit_engine_start(&engine);

  printf("%12.6f\n", wav.duration);

  akit_msleep(1000);

  AkitWorldInfo world_info = {0};
  world_info.size = VEC3(128, 128, 128);
  world_info.roughness = 0.5f;

  akit_engine_push_sound(&engine, (AkitSound){
    .data = wav.data,
    .length = wav.length,
    .sample_rate = wav.header.sample_rate,
    .duration = fmax(wav.duration, 0.5f),
    .channels = wav.header.channels,
    .block_align = wav.header.block_align,
    .name = "test_sound",
    .gain = 0.5f,
    .world_info = world_info,
    .fade_time = 2.0f
  });


  akit_msleep(2000);


  #if 0
  akit_engine_stop_sound(&engine, "test_sound");


  akit_engine_push_sound(&engine, (AkitSound){
    .data = wav.data,
    .length = wav.length,
    .sample_rate = wav.header.sample_rate,
    .position = VEC3(16.0f, 0.0f, 1.0f),
    .duration = fmax(wav.duration, 0.5f),
    .channels = wav.header.channels,
    .block_align = wav.header.block_align,
    .gain = 1.0f,
    .name = "test2",
    .world_info = world_info
  });

  #endif


  float p = 1.0f;

  while (akit_engine_is_playing(&engine)) {
    akit_engine_update_sound(&engine, "test_sound", (AkitSound){
       .gain = p
    });
    akit_msleep(10);

    p *= 0.99f;
  }

  akit_engine_stop(&engine);


  if (wav.data) {
    free(wav.data);
    wav.data = 0;
  }

  return 0;
}
