#include <akit/array.h>
#include <akit/macros.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RETURN_UNLOCK(G)                                                       \
  { return G; };

void akit_array_init(AkitArray *array, int64_t item_size) {
  array->item_size = item_size;
  array->items = 0;
  array->length = 0;
  array->capacity = Akit_ARRAY_CAP;
  array->stack_items = 0;
}

AkitArray *init_akit_array(int64_t item_size) {
  AkitArray *array = NEW(AkitArray);
  akit_array_init(array, item_size);
  //  if (pthread_mutex_init(&array->lock, 0)) {
  //    fprintf(stderr, "(Akit): Error, failed to initialize array mutex.\n");
  //   return 0;
  // }
  return array;
}

AkitArray *init_akit_array_with_capacity(int64_t item_size, int64_t capacity) {
  AkitArray *array = NEW(AkitArray);
  akit_array_init_with_capacity(array, item_size, MAX(capacity, 1));
  return array;
}

int64_t akit_array_get_capacity(AkitArray *array) {
  return MAX(array->capacity, 1);
}

void akit_array_init_with_capacity(AkitArray *array, int64_t item_size,
                                   int64_t capacity) {
  akit_array_init(array, item_size);
  array->capacity = MAX(capacity, 1);
}

int akit_array_grow(AkitArray *array, int64_t n) {
  if (array->item_size <= 0 || n <= 0)
    return 0;
  array->avail += akit_array_get_capacity(array) * n;
  array->items =
      realloc(array->items, (array->length + array->avail) * array->item_size);
  return array->items != 0;
}

AkitArray *akit_array_push(AkitArray *array, void *item) {
  if (array->avail <= 0 || array->items == 0) {
    if (!akit_array_grow(array, 1)) {
      fprintf(stderr, "Failed to grow array. (%s)\n", __func__);
      return 0;
    }
  }

  assert(array->avail > 0);
  // pthread_mutex_trylock(&array->lock);
  assert(array->item_size != 0);
  // array->items = realloc(array->items, array->length * array->item_size);
  array->items[array->length] = item;
  array->length++;
  array->avail -= 1;
  return array;
  // RETURN_UNLOCK(array);
}

void akit_array_clear(AkitArray *array) {
  if (!array)
    RETURN_UNLOCK();
  // pthread_mutex_trylock(&array->lock);
  if (array->length <= 0)
    RETURN_UNLOCK();
  if (array->items == 0)
    RETURN_UNLOCK();

  free(array->items);
  array->items = 0;
  array->length = 0;
  array->avail = 0;
  assert(array->item_size != 0);
  RETURN_UNLOCK();
}

void akit_array_clear_and_free(AkitArray *array,
                               AkitArrayFreeFunction freefunc) {
  // pthread_mutex_trylock(&array->lock);
  if (!array)
    RETURN_UNLOCK();
  if (array->length <= 0)
    RETURN_UNLOCK();
  if (array->items == 0)
    RETURN_UNLOCK();

  if (freefunc != 0) {
    for (uint32_t i = 0; i < array->length; i++) {
      void *item = array->items[i];
      if (item == 0)
        continue;
      freefunc(item);
      array->items[i] = 0;
    }
  }

  free(array->items);
  array->items = 0;
  array->length = 0;
  array->avail = 0;
  RETURN_UNLOCK();
}

AkitArray *akit_array_push_memcpy(AkitArray *array, void *item,
                                  uint32_t item_size) {
  assert("Do not use this" && 0);
  // pthread_mutex_trylock(&array->lock);
  assert(array->item_size != 0);
  array->item_size = item_size;
  array->length++;
  array->items = realloc(array->items, array->length * array->item_size);
  array->items[array->length - 1] = calloc(1, item_size);
  memcpy(&array->items[array->length - 1], &item, item_size);
  return array;
  // RETURN_UNLOCK(array);
}

void akit_array_clear_and_free_completely(AkitArray *array,
                                          AkitArrayFreeFunction freefunc) {
  akit_array_clear_and_free(array, freefunc);
  free(array);
  array = 0;
}

void akit_array_sort(AkitArray *array, AkitArraySortFunction sort_function) {
  if (!array)
    return;
  if (array->items == 0)
    return;
  if (array->length <= 0)
    return;
  // pthread_mutex_trylock(&array->lock);
  assert(array->item_size != 0);
  qsort(&array->items[0], array->length, array->item_size, sort_function);
  // RETURN_UNLOCK();
}

void akit_array_shift_left(AkitArray *array, int index) {
  for (int i = index; i < array->length - 1; i++)
    array->items[i] = array->items[i + 1];
}

void akit_array_shift_right(AkitArray *array, int index) {
  for (int i = array->length - 1; i >= index; i--) {
    array->items[MIN(array->length - 1, i + 1)] = array->items[i];
    array->items[i] = 0;
  }
}

void akit_array_remove(AkitArray *array, void *element,
                       AkitArrayFreeFunction free_method) {
  if (!element)
    return;
  if (!array)
    return;
  if (array->length == 0)
    return;
  if (!array->items)
    return;
  if (array->item_size == 0)
    return;
  if (array->items == 0)
    return;
  if (array->item_size != sizeof(element))
    return;

  if (((int64_t)array->length) - 1 <= 0) {
    if (array->items != 0) {
      free(array->items);
    }
    array->items = 0;
    array->length = 0;
    array->avail = 0;
    if (free_method) {
      free_method(element);
    }
    return;
  }

  // pthread_mutex_trylock(&array->lock);
  assert(array->item_size != 0);
  int index = -1;

  if (element == 0) {
    RETURN_UNLOCK();
  }

  for (int i = 0; i < array->length; i++) {
    if (!array->items[i])
      continue;
    if (array->items[i] == element) {
      index = i;
      break;
    }
  }

  if (index < 0) {
    RETURN_UNLOCK();
  }

  if (array->items[index] != element) {
    printf("Bad!\n");
    RETURN_UNLOCK();
  }

  if (free_method && free_method != (void *)0)
    free_method(array->items[index]);

  akit_array_shift_left(array,
                        index); /* First shift the elements, then reallocate */
  //  void *tmp = realloc(array->items, (array->length - 1) * array->item_size);
  //  if (tmp == NULL && array->length > 1) {
  //   return;
  /* No memory available */
  // exit(EXIT_FAILURE);
  // }
  array->length = MAX(0, array->length - 1);
  if (array->items != 0) {
    array->avail += 1;
  }
  // array->items = tmp;
  RETURN_UNLOCK();
}

void akit_array_concat(AkitArray *a, AkitArray *b) {
  if (!b)
    return;
  if (akit_array_is_empty(b))
    return;
  if (b->length >= a->avail) {
    akit_array_grow(a, MAX(1, b->length / akit_array_get_capacity(a)));
  }
  Akit_ITER_ARRAY(b, void, item, { akit_array_push(a, item); });
}

void *akit_array_pop_zero(AkitArray *array, uint32_t index) {
  assert("DONT USE POP ZERO" && 0);
  if (!array)
    return 0;
  if (!array->items)
    return 0;
  if (!array->length)
    return 0;
  if (array->length <= index)
    return 0;

  void *item = array->items[index];
  array->items[index] = 0;

  akit_array_shift_left(array, index);

  return item;
}

int akit_array_slice(AkitArray *src, AkitArray *dest, uint32_t start,
                     uint32_t end) {
  if (!src)
    return 0;
  if (!src->length)
    return 0;
  if (!src->items)
    return 0;

  dest->item_size = src->item_size;
  dest->length = 0;
  dest->items = 0;
  dest->stack_items = 0;

  for (int64_t i = start; i < ((int64_t)end) - 1; i++) {
    akit_array_push(dest, src->items[i]);
    if (i >= src->length) {
      break;
    }
  }

  return dest->length > 0 && dest->items != 0;
}

unsigned int akit_array_is_empty(AkitArray *array) {
  if (!array)
    return 1;
  if (array->items == 0)
    return 1;
  if (array->length <= 0)
    return 1;
  if (array->item_size <= 0)
    return 1;

  return 0;
}

unsigned int akit_array_includes(AkitArray *array, void *item) {
  if (!array)
    return 0;
  if (!array->items)
    return 0;
  if (!array->length)
    return 0;

  for (uint32_t i = 0; i < array->length; i++) {
    if (!array->items)
      continue;
    if (array->items[i] == item)
      return 1;
  }

  return 0;
}

int64_t akit_array_index_of(AkitArray *array, void *item) {
  if (!array)
    return -1;
  if (!array->item_size)
    return -1;
  if (!array->length)
    return -1;

  for (int64_t i = 0; i < array->length; i++) {
    if (array->items[i] == item)
      return i;
  }

  return -1;
}

int akit_array_make_unique(AkitArray *array, AkitArrayFreeFunction freefunc) {
  if (!array)
    return 0;
  if (!array->items)
    return 0;
  if (!array->item_size)
    return 0;
  if (!array->length)
    return 0;

loop_start:
  for (int64_t i = 0; i < array->length; i++) {
    void *item = array->items[i];

    if (akit_array_count(array,
                         array->selector ? array->selector(item) : item) > 1) {
      akit_array_remove(array, item, freefunc);
      goto loop_start;
    }
  }

  return 1;
}

int64_t akit_array_count(AkitArray *array, void *item) {
  if (!array)
    return 0;
  if (!array->items)
    return 0;
  if (!array->length)
    return 0;
  if (!array->item_size)
    return 0;

  int64_t counter = 0;

  for (int64_t i = 0; i < array->length; i++)
    counter += (int)((array->selector ? array->selector(array->items[i])
                                      : array->items[i]) == item);

  return counter;
}

int akit_array_copy(AkitArray *dest, AkitArray src) {
  if (!dest)
    return 0;
  if (!src.item_size)
    return 0;
  if (!src.items)
    return 0;
  if (akit_array_is_empty(&src))
    return 0;

  if (dest->items) {
    free(dest->items);
    dest->items = 0;
  }

  dest->length = src.length;
  dest->item_size = src.item_size;

  if (!dest->length || !dest->item_size)
    return 0;

  dest->items = calloc(dest->length, dest->item_size);
  memcpy(dest->items, src.items, src.length * src.item_size);

  return dest->items != 0 && dest->length > 0;
}

int akit_array_reverse(AkitArray *array) {
  if (!array->items)
    return 0;
  if (!array->length)
    return 0;

  int64_t len = array->length;

  for (int64_t i = 0; i < len; i++) {
    array->items[len - 1 - i] = array->items[i];
  }

  return 1;
}
