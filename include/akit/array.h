#ifndef Akit_ARRAY_H
#define Akit_ARRAY_H
#include <stdint.h>
#include <pthread.h>

#define Akit_ARRAY_CAP 64

typedef void *(*AkitArraySelectorFunction)(void *item);
typedef struct {
  volatile int64_t length;
  int64_t item_size;
  int64_t avail;
  int64_t capacity;
  void **items;
  void *stack_items;
  AkitArraySelectorFunction selector;
  //  pthread_mutex_t lock;
} AkitArray;

#define Akit_ARRAY(item_size) ((AkitArray){0, item_size, 0})

#define Akit_ITER_ARRAY(arr, T, valuename, body)  \
  {                                              \
    for (uint32_t i = 0; i < arr->length; i++) { \
      T *valuename = (T *)arr->items[i];         \
      body                                       \
    }                                            \
  }

AkitArray *init_akit_array(int64_t item_size);

AkitArray* init_akit_array_with_capacity(int64_t item_size, int64_t capacity);

int64_t akit_array_get_capacity(AkitArray* array);

void akit_array_init_with_capacity(AkitArray *array, int64_t item_size, int64_t capacity);


int akit_array_grow(AkitArray* array, int64_t n);

void akit_array_init(AkitArray *array, int64_t item_size);


AkitArray *akit_array_push(AkitArray *array, void *item);

void akit_array_clear(AkitArray *array);

AkitArray *akit_array_push_memcpy(AkitArray *array, void *item,
                                uint32_t item_size);

typedef void (*AkitArrayFreeFunction)(void *item);

void akit_array_clear_and_free(AkitArray *array, AkitArrayFreeFunction freefunc);
void akit_array_clear_and_free_completely(AkitArray *array,
                                         AkitArrayFreeFunction freefunc);

typedef int (*AkitArraySortFunction)(const void *a, const void *b);

void akit_array_sort(AkitArray *array, AkitArraySortFunction sort_function);

void akit_array_shift_left(AkitArray *array, int index);

void akit_array_shift_right(AkitArray *array, int index);

void akit_array_remove(AkitArray *array, void *element, AkitArrayFreeFunction);

void akit_array_concat(AkitArray *a, AkitArray *b);

int akit_array_slice(AkitArray *src, AkitArray *dest, uint32_t start,
                    uint32_t end);

void *akit_array_pop_zero(AkitArray *array, uint32_t index);

unsigned int akit_array_is_empty(AkitArray *array);

unsigned int akit_array_includes(AkitArray *array, void *item);

int64_t akit_array_index_of(AkitArray *array, void *item);

int akit_array_make_unique(AkitArray *array, AkitArrayFreeFunction freefunc);

int64_t akit_array_count(AkitArray *array, void *item);

int akit_array_copy(AkitArray *dest, AkitArray src);

int akit_array_reverse(AkitArray *array);

#define Akit_ARRAY_AT(T, arr, INDEX) ((T)((arr)->items[INDEX]))

#endif
