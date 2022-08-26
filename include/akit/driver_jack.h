#ifndef AKIT_DRIVER_JACK_H
#define AKIT_DRIVER_JACK_H
#include <akit/driver.h>
int akit_driver_jack_setup(AkitDriver* driver);
int akit_driver_jack_destroy(AkitDriver* driver);
int akit_driver_jack_buffer_data(AkitDriver* driver, float* buffer, int64_t length);
int akit_driver_jack_flush(AkitDriver* driver);
int akit_driver_jack_prepare(AkitDriver* driver);
int akit_driver_jack_reset(AkitDriver* driver);
int64_t akit_driver_jack_get_avail(AkitDriver* driver);
#endif
