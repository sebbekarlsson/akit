#include <akit/driver.h>
#include <akit/driver_jack.h>
#include <stdio.h>

int akit_driver_jack_setup(AkitDriver* driver) {
  fprintf(stderr, "JACK is not implemented.\n");
  return 0;
}

int akit_driver_jack_destroy(AkitDriver* driver) {
  fprintf(stderr, "JACK is not implemented.\n");
  return 0;
}

int akit_driver_jack_buffer_data(AkitDriver* driver, float* buffer, int64_t length) {
  fprintf(stderr, "JACK is not implemented.\n");
  return 0;
}

int akit_driver_jack_flush(AkitDriver* driver) {
  fprintf(stderr, "JACK is not implemented.\n");
  return 0;
}

int akit_driver_jack_prepare(AkitDriver* driver) {
  fprintf(stderr, "JACK is not implemented.\n");
  return 0;
}

int akit_driver_jack_reset(AkitDriver* driver) {
  fprintf(stderr, "JACK is not implemented.\n");
  return 0;
}

int64_t akit_driver_jack_get_avail(AkitDriver* driver) {
  fprintf(stderr, "JACK is not implemented.\n");
  return 0;
}
