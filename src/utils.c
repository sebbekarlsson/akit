#include <akit/utils.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>


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
