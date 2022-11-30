#ifndef AKIT_UTILS_H
#define AKIT_UTILS_H
#include <stdbool.h>
float akit_clamp(float v, float min, float max);
float akit_random_range(float min, float max);
float akit_lerp(float from, float to, float scale);
float akit_sign(float v);


bool akit_number_is_unsafe(float v);


#endif
