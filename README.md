# Akit (Audio kit)
> 3D Audio engine for Linux

## Dependencies
> Currently, the only driver supported is `alsa`.

## Example usage
```C
#include <akit/engine.h>

int main(int argc, char* argv[]) {
  // setup configuration
  AkitDriverConfig config = {0};
  config.sample_rate = 44100;
  config.frame_length = 512;
  config.type = AKIT_DRIVER_TYPE_ASOUND;

  // initialize engine
  AkitEngine engine = {0};
  akit_engine_init(&engine, (AkitEngineConfig){.driver_config = config,
                                               .max_sounds = 10,
                                               .normalize_stereo = true});
											   
  // update or set where listener is
  akit_engine_set_listener(&engine, (AkitListener){.forward = VEC3(0, 0, 1),
                                                   .up = VEC3(0, 1, 0),
                                                   .position = VEC3(0, 0, 0)});
  
  // start engine
  akit_engine_start(&engine);
  
 
  // emit sound
  akit_engine_push_sound(&engine, (AkitSound){
    .data = wav.data, // raw data from wav file
    .length = wav.length,
    .sample_rate = wav.header.sample_rate,
    .position = VEC3(16.0f, 0.0f, 1.0f),  // position where sound is supposed to origin
    .duration = fmax(wav.duration, 0.5f),
    .channels = wav.header.channels,
    .block_align = wav.header.block_align,
    .gain = 1.0f // gain (0 to 1)
  });
 
  // wait for engine to finish playing audio
  while (akit_engine_is_playing(&engine)) {
    akit_msleep(10);
  }
  
  // stop and destroy engine
  akit_engine_stop(&engine);
  akit_engine_destroy(&engine);

  return 0;
}
```
