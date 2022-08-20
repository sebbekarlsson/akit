#include <akit/driver_asound.h>
#include <akit/macros.h>
#include <akit/constants.h>
#include <stdio.h>
#include <akit/sleep.h>

#define AKIT_DRIVER_ASOUND_SAMPLE_TYPE SND_PCM_FORMAT_FLOAT

#define AKIT_DRIVER_ASOUND_PCM_DEVICE "default"

int akit_driver_asound_setup(AkitDriver *driver) {
  if (driver->driver) {
    fprintf(stderr, "Driver already setup!\n");
    return 0;
  }
  unsigned int tmp = 0;
  snd_pcm_uframes_t frames = 0;
  unsigned int sample_rate = driver->config.sample_rate;

  AkitDriverAsound *asound = NEW(AkitDriverAsound);

  if ((asound->pcm =
           snd_pcm_open(&asound->pcm_handle, AKIT_DRIVER_ASOUND_PCM_DEVICE,
                        SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    fprintf(stderr, "ERROR: Can't open \"%s\" PCM device. %s\n",
            AKIT_DRIVER_ASOUND_PCM_DEVICE, snd_strerror(asound->pcm));

  snd_pcm_hw_params_alloca(&asound->params);
  snd_pcm_hw_params_any(asound->pcm_handle, asound->params);

  if ((asound->pcm =
           snd_pcm_hw_params_set_access(asound->pcm_handle, asound->params,
                                        SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    fprintf(stderr, "ERROR: Can't set interleaved mode. %s\n",
            snd_strerror(asound->pcm));
  if ((asound->pcm =
           snd_pcm_hw_params_set_format(asound->pcm_handle, asound->params,
                                        AKIT_DRIVER_ASOUND_SAMPLE_TYPE)) < 0)
    fprintf(stderr, "ERROR: Can't set format. %s\n", snd_strerror(asound->pcm));

  if ((asound->pcm = snd_pcm_hw_params_set_channels(
           asound->pcm_handle, asound->params, AKIT_CHANNELS)) < 0)
    fprintf(stderr, "ERROR: Can't set channels number. %s\n",
            snd_strerror(asound->pcm));

  if ((asound->pcm = snd_pcm_hw_params_set_rate_near(
           asound->pcm_handle, asound->params, &sample_rate, 0)) < 0)
    fprintf(stderr, "ERROR: Can't set rate. %s\n", snd_strerror(asound->pcm));

  if ((asound->pcm = snd_pcm_hw_params(asound->pcm_handle, asound->params)) < 0)
    fprintf(stderr, "ERROR: Can't set harware parameters. %s\n",
            snd_strerror(asound->pcm));

  snd_pcm_hw_params_get_channels(asound->params, &tmp);
  printf("audio engine channels: %i ", tmp);
  driver->info.channels = tmp;

  if (tmp == 1) {
    printf("(mono)\n");
  } else if (tmp == 2) {
    printf("(stereo)\n");
  }

  snd_pcm_hw_params_get_rate(asound->params, &tmp, 0);
  printf("rate: %d bps\n", tmp);
  driver->info.sample_rate = tmp;
  sample_rate = tmp;

  /* Allocate buffer to hold single period */
  // snd_pcm_hw_params_get_period_size(params, &frames, 0);
  uint64_t min_period = driver->config.frame_length;
  snd_pcm_hw_params_set_period_size_near(asound->pcm_handle, asound->params,
                                         &min_period, 0);

  snd_pcm_hw_params_get_period_time(asound->params, &tmp, NULL);
  printf("frame_length: %d\n", tmp);
  driver->info.frame_length = tmp;


  snd_pcm_nonblock(asound->pcm_handle, 1);

  while (snd_pcm_state(asound->pcm_handle) != SND_PCM_STATE_PREPARED) {
    akit_msleep(1);
  }


  snd_pcm_start(asound->pcm_handle);

  driver->driver = asound;
  driver->initialized = true;




  return 1;
}

int akit_driver_asound_buffer_data(AkitDriver *driver, float *buffer,
                                   int64_t length) {
  if (!driver->driver) {
    fprintf(stderr, "(Akit): No driver!\n");
    return 0;
  }
  if (driver->type != AKIT_DRIVER_TYPE_ASOUND) {
    fprintf(stderr, "(Akit): Invalid driver!\n");
    return 0;
  }
  if (!driver->initialized) {
    fprintf(stderr, "(Akit): Driver not initialized!\n");
    return 0;
  }


  AkitDriverAsound *asound = (AkitDriverAsound *)driver->driver;
  int err = 0;
  if ((err = snd_pcm_writei(asound->pcm_handle, buffer, length)) < 0) {
    fprintf(stderr, "XRUN.\n");
    snd_pcm_recover(asound->pcm_handle, err, 0);
    //snd_pcm_prepare(asound->pcm_handle);
    return 0;
  }

  return 1;
}

int akit_driver_asound_destroy(AkitDriver *driver) {
  if (!driver->driver)
    return 0;
  if (driver->type != AKIT_DRIVER_TYPE_ASOUND)
    return 0;
  if (!driver->initialized)
    return 0;

  printf("destroying ASOUND driver.\n");

  AkitDriverAsound *asound = (AkitDriverAsound *)driver->driver;

  snd_pcm_drain(asound->pcm_handle);


  free(asound);
  driver->driver = 0;

  return 1;
}

int akit_driver_asound_prepare(AkitDriver* driver) {
    if (!driver->driver)
    return 0;
  if (driver->type != AKIT_DRIVER_TYPE_ASOUND)
    return 0;
  if (!driver->initialized)
    return 0;

  AkitDriverAsound *asound = (AkitDriverAsound *)driver->driver;
  snd_pcm_prepare(asound->pcm_handle);
  return 1;
}

int akit_driver_asound_flush(AkitDriver* driver) {
  if (!driver->driver)
    return 0;
  if (driver->type != AKIT_DRIVER_TYPE_ASOUND)
    return 0;
  if (!driver->initialized)
    return 0;

  AkitDriverAsound *asound = (AkitDriverAsound *)driver->driver;

//  snd_pcm_prepare(asound->pcm_handle);
  snd_pcm_nonblock(asound->pcm_handle, 1);

  return 1;
}
