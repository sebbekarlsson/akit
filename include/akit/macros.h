#ifndef AKIT_MACROS_H
#define AKIT_MACROS_H
#define NEW(T) ((T *)calloc(1, sizeof(T)))

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif
#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#define OR(a, b) a ? a : b

#endif
