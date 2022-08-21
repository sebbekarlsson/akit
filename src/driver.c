#include <akit/driver.h>
#include <akit/driver_asound.h>
#include <akit/driver_jack.h>
#include <assert.h>
#include <stdio.h>

static int akit_driver_setup(AkitDriver *driver, AkitDriverType type) {
  switch (type) {
  case AKIT_DRIVER_TYPE_ASOUND:
    return akit_driver_asound_setup(driver);
    break;
  case AKIT_DRIVER_TYPE_JACK:
    return akit_driver_jack_setup(driver);
    break;
  }

  assert(0 && "Not reached");
  return 0;
}

int akit_driver_init(AkitDriver *driver, AkitDriverConfig config) {
  if (driver->initialized)
    return 0;
  driver->initialized = true;
  driver->config = config;
  driver->type = config.type;

  if (!akit_driver_setup(driver, driver->type)) {
    fprintf(stderr, "Failed to setup driver %d\n", driver->type);
    driver->initialized = false;
    return 0;
  }

  return 1;
}

int akit_driver_buffer_data(AkitDriver *driver, float *buffer, int64_t length) {
  switch (driver->type) {
  case AKIT_DRIVER_TYPE_ASOUND:
    return akit_driver_asound_buffer_data(driver, buffer, length);
    break;
  case AKIT_DRIVER_TYPE_JACK:
    return akit_driver_jack_buffer_data(driver, buffer, length);
    break;
  }

  assert(0 && "Not reached");
  return 0;
}

int akit_driver_destroy(AkitDriver *driver) {
  switch (driver->type) {
  case AKIT_DRIVER_TYPE_ASOUND:
    return akit_driver_asound_destroy(driver);
    break;
  case AKIT_DRIVER_TYPE_JACK:
    return akit_driver_jack_destroy(driver);
    break;
  }

  assert(0 && "Not reached");
  return 0;
}

int akit_driver_flush(AkitDriver *driver) {
  switch (driver->type) {
  case AKIT_DRIVER_TYPE_ASOUND:
    return akit_driver_asound_flush(driver);
    break;
  case AKIT_DRIVER_TYPE_JACK:
    return akit_driver_jack_flush(driver);
    break;
  }

  assert(0 && "Not reached");
  return 0;
}

int akit_driver_prepare(AkitDriver *driver) {
  switch (driver->type) {
  case AKIT_DRIVER_TYPE_ASOUND:
    return akit_driver_asound_prepare(driver);
    break;
  case AKIT_DRIVER_TYPE_JACK:
    return akit_driver_jack_prepare(driver);
    break;
  }

  assert(0 && "Not reached");
  return 0;
}

int64_t akit_driver_get_avail(AkitDriver *driver) {
  switch (driver->type) {
  case AKIT_DRIVER_TYPE_ASOUND:
    return akit_driver_asound_get_avail(driver);
    break;
  case AKIT_DRIVER_TYPE_JACK:
    return akit_driver_jack_get_avail(driver);
    break;
  }

  assert(0 && "Not reached");
  return 0;
}
