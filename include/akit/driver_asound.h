#ifndef AKIT_DRIVER_ASOUND_H
#define AKIT_DRIVER_ASOUND_H
#include <akit/driver.h>
#include <alsa/asoundlib.h>
typedef struct {
  unsigned int pcm;
  snd_pcm_t* pcm_handle;
  snd_pcm_hw_params_t* params;
  snd_pcm_sw_params_t* sw_params;
  snd_pcm_uframes_t frames;
} AkitDriverAsound;


int akit_driver_asound_setup(AkitDriver* driver);
int akit_driver_asound_destroy(AkitDriver* driver);
int akit_driver_asound_buffer_data(AkitDriver* driver, float* buffer, int64_t length);

int akit_driver_asound_flush(AkitDriver* driver);
int akit_driver_asound_prepare(AkitDriver* driver);
int akit_driver_asound_reset(AkitDriver* driver);
int akit_driver_asound_wait(AkitDriver* driver, int timeout);
int64_t akit_driver_asound_get_delay(AkitDriver* driver);

int64_t akit_driver_asound_get_avail(AkitDriver* driver);

#endif
