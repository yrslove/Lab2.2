#include "arraylist.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct ArrayList {
    unsigned char *data;
    size_t size;
    size_t capacity;
    size_t element_size;
};

static bool arraylist_reserve(ArrayList *list, size_t needed) {
    if (needed <= list->capacity) {
        return true;
    }

    if (needed > SIZE_MAX / list->element_size) {
        return false;
    }

    size_t capacity = list->capacity == 0 ? 4 : list->capacity;
    while (capacity < needed) {
        if (capacity > SIZE_MAX / 2) {
            capacity = needed;
            break;
        }
        capacity *= 2;
    }

    if (capacity > SIZE_MAX / list->element_size) {
        capacity = needed;
    }

    void *data = realloc(list->data, capacity * list->element_size);
    if (data == NULL) {
        return false;
    }

    list->data = data;
    list->capacity = capacity;
    return true;
}

ArrayList *arraylist_create(size_t element_size) {
    if (element_size == 0) {
        return NULL;
    }

    ArrayList *list = calloc(1, sizeof(*list));
    if (list != NULL) {
        list->element_size = element_size;
    }
    return list;
}

void arraylist_destroy(ArrayList *list) {
    if (list == NULL) {
        return;
    }
    free(list->data);
    free(list);
}

size_t arraylist_size(const ArrayList *list) {
    return list == NULL ? 0 : list->size;
}

bool arraylist_add(ArrayList *list, const void *item) {
    return list != NULL && arraylist_insert(list, list->size, item);
}

bool arraylist_insert(ArrayList *list, size_t index, const void *item) {
    if (list == NULL || item == NULL || index > list->size || list->size == SIZE_MAX) {
        return false;
    }
    if (!arraylist_reserve(list, list->size + 1)) {
        return false;
    }

    unsigned char *position = list->data + index * list->element_size;
    memmove(position + list->element_size, position,
            (list->size - index) * list->element_size);
    memcpy(position, item, list->element_size);
    ++list->size;
    return true;
}

bool arraylist_remove(ArrayList *list, size_t index) {
    if (list == NULL || index >= list->size) {
        return false;
    }

    unsigned char *position = list->data + index * list->element_size;
    memmove(position, position + list->element_size,
            (list->size - index - 1) * list->element_size);
    --list->size;
    return true;
}

bool arraylist_set(ArrayList *list, size_t index, const void *item) {
    if (list == NULL || item == NULL || index >= list->size) {
        return false;
    }
    memcpy(list->data + index * list->element_size, item, list->element_size);
    return true;
}

bool arraylist_get(const ArrayList *list, size_t index, void *item) {
    if (list == NULL || item == NULL || index >= list->size) {
        return false;
    }
    memcpy(item, list->data + index * list->element_size, list->element_size);
    return true;
}
