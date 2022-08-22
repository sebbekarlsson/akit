#include <akit/utils.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

float akit_clamp(float v, float min, float max) {
  return fmaxf(min, fminf(max, v));
}

float akit_random_range(float min, float max) {
  float scale = rand() / (float)RAND_MAX;
  return akit_clamp(min + scale * (max - min), min, max);
}

float akit_lerp(float from, float to, float scale) {
  return from + (to - from) * scale;
}

float akit_sign(float v) {
  return v < 0 ? -1 : 1;
}
