#ifndef AKIT_LISTENER_H
#define AKIT_LISTENER_H
#include <vec3/vec3.h>
typedef struct {
  Vector3 forward;
  Vector3 up;
  Vector3 position;
  float distance_multiplier;
  float ear_forward_bend;
} AkitListener;
#endif
