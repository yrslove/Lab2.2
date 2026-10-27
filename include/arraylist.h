#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct ArrayList ArrayList;

ArrayList *arraylist_create(size_t element_size);
void arraylist_destroy(ArrayList *list);
size_t arraylist_size(const ArrayList *list);
bool arraylist_add(ArrayList *list, const void *item);
bool arraylist_insert(ArrayList *list, size_t index, const void *item);
bool arraylist_remove(ArrayList *list, size_t index);
bool arraylist_set(ArrayList *list, size_t index, const void *item);
bool arraylist_get(const ArrayList *list, size_t index, void *item);

#endif
