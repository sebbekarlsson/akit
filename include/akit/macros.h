#ifndef AKIT_MACROS_H
#define AKIT_MACROS_H
#define NEW(T) ((T *)calloc(1, sizeof(T)))

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif
#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#ifndef OR
#define OR(a, b) a ? a : b
#endif

#define AKIT_WARNING(...)                                                      \
  {                                                                            \
    printf("(Akit)(Warning)(%s): \n", __func__);   \
    fprintf(__VA_ARGS__);                                                      \
  }


#define AKIT_WARNING_RETURN(ret, ...)                                          \
  {                                                                            \
    printf("\n****\n");                                                        \
    printf("(Akit)(Warning)(%s): \n", __func__);   \
    fprintf(__VA_ARGS__);                                                      \
    printf("\n****\n");                                                        \
    return ret;                                                                \
  }

#endif
