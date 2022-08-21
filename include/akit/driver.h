#ifndef AKIT_DRIVER_H
#define AKIT_DRIVER_H
#include <stdint.h>
#include <stdbool.h>

typedef enum {
  AKIT_DRIVER_TYPE_ASOUND,
  AKIT_DRIVER_TYPE_JACK
} AkitDriverType;

typedef struct {
  AkitDriverType type;
  unsigned int sample_rate;
  uint64_t frame_length;
} AkitDriverConfig;

typedef struct {
  unsigned int sample_rate;
  uint64_t frame_length;
  int64_t channels;
} AkitDriverInfo;

typedef struct {
  AkitDriverType type;
  AkitDriverConfig config;
  AkitDriverInfo info;
  bool initialized;
  void* driver;
} AkitDriver;

int akit_driver_init(AkitDriver* driver, AkitDriverConfig config);

int akit_driver_buffer_data(AkitDriver* driver, float* buffer, int64_t length);

int akit_driver_destroy(AkitDriver* driver);

int akit_driver_flush(AkitDriver* driver);
int akit_driver_prepare(AkitDriver* driver);

int64_t akit_driver_get_avail(AkitDriver* driver);


#endif
