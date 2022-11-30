#include <akit/engine.h>
#include <akit/sleep.h>
#include <akit/dsp.h>
#include <akit/utils.h>
#include <stdio.h>
#include <waves/wav.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {


  Wave wav1 = {0};
  WaveOptions wav_options = {0};
  wav_options.convert_to_float = true;
  if (!wav_read(&wav1, "../assets/vocal.wav", wav_options)) {
    fprintf(stderr, "Unable to read wav!\n");
    return 0;
  }

  Wave wav2 = {0};
  if (!wav_read(&wav2, "../assets/flute.wav", wav_options)) {
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

  akit_msleep(1000);

  for (int i = 0; i < 2; i++) {

    Wave wav = {0};

    wav = i % 2 == 0 ? wav1 : wav2;


  akit_engine_push_sound(&engine, (AkitSound){
    .data = wav.data,
    .length = wav.length,
    .sample_rate = wav.header.sample_rate,
    .duration = fmax(wav.duration, 0.5f),
    .channels = wav.header.channels,
 //   .position = VEC3(4.0f, 0.0f, 1.0f),
    .block_align = wav.header.block_align,
    .gain = 0.07f,
    .random_seed = akit_random_range(1.0f, 100.0f),
    .random_factor = 0.99f,
    .reverb.mix = 0.5f,
    .reverb.delay = 0.6f,
    .reverb.feedback = 0.45f,
    .reverb.pingpong_amplitude = 0.7f,
    .reverb.pingpong_speed = VEC2(8.0f, 16.0f),
    .fade_time = 0.0f
  });
  akit_msleep(500);
  }



#if 0

//  akit_engine_stop_sound(&engine, "test_sound");


  akit_engine_push_sound(&engine, (AkitSound){
    .data = wav.data,
    .length = wav.length,
    .sample_rate = wav.header.sample_rate,
    .position = VEC3(16.0f, 0.0f, 1.0f),
    .duration = fmax(wav.duration, 0.5f),
    .channels = wav.header.channels,
    .block_align = wav.header.block_align,
    .gain = 1.0f,
    .world_info = world_info
  });
#endif



  while (akit_engine_is_playing(&engine)) {
   // akit_engine_update_sound(&engine, "test_sound", (AkitSound){
    //   .gain = p
   // });
    akit_msleep(10);

  }

  akit_engine_stop(&engine);
  akit_engine_destroy(&engine);


  if (wav1.data) {
    free(wav1.data);
    wav1.data = 0;
  }

  if (wav2.data) {
    free(wav2.data);
    wav2.data = 0;
  }

  return 0;
}
