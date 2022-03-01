//
//  unmap.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "unmap.h"

#define STARTING_CAPACITY 10

UNMap *unmap_init(void) {
    UNMap *map = (UNMap *) malloc(sizeof(UNMap));
    map->items = 0;
    map->capacity = STARTING_CAPACITY;
    map->pair_values = malloc(sizeof(UNMapPairValue *) * map->capacity);

    return map;
}

__attribute__((unused)) void unmap_insert_at(UNMap *map, uintptr_t index, UNMapPairValue *pair) {
    if (map->items >= map->capacity) {
        map->capacity *= 2;
        map->pair_values = realloc(map->pair_values, sizeof(UNMapPairValue *) * map->capacity);
    }

    for (size_t i = map->items; i > index; i--) {
        map->pair_values[i] = map->pair_values[i - 1];
    }

    map->pair_values[index] = pair;
    map->items++;
}

void unmap_push_back(UNMap *map, UNMapPairValue *pair) {
    map->items += 1;

    if (map->items >= map->capacity) {
        map->capacity *= 2;
        map->pair_values = realloc(map->pair_values, sizeof(UNMapPairValue *) * map->capacity);
    }
    map->pair_values[map->items - 1] = pair;
}

__attribute__((unused)) UNMapPairValue *unmap_pop_back(UNMap *map) {
    if (map->items == 0)
        return NULL;
    UNMapPairValue *old_pair = map->pair_values[map->items - 1];
    map->items--;

    if (map->items < map->capacity / 2) {
        map->capacity /= 2;
        map->pair_values = realloc(map->pair_values, sizeof(UNMapPairValue *) * map->capacity);
    }

    return old_pair;
}

__attribute__((unused)) void unmap_append(UNMap *map, UNMap *other) {
    for (int i = 0; i < other->items; i++)
        unmap_push_back(map, other->pair_values[i]);
}

void unmap_destroy(UNMap *map) {
    for (int i = 0; i < map->items; i++)
        free(map->pair_values[i]);
    free(map->pair_values);
    free(map);
}

__attribute__((unused)) void unmap_remove_all(UNMap *map) {
    for (int i = 0; i < map->items; i++)
        unmap_remove_at(map, i);
}

void unmap_remove_at(UNMap *map, size_t index) {
    if (index < 0 || index >= map->items)
        return;

    UNMapPairValue *pair = map->pair_values[index];

    free(pair);

    for (size_t i = index; i < map->items - 1; i++)
        map->pair_values[i] = map->pair_values[i + 1];

    map->items--;
}

__attribute__((unused)) void unmap_swap(UNMap *map, size_t index1, size_t index2) {
    if (index1 < 0 || index1 >= map->items || index2 < 0 || index2 >= map->items)
        return;

    UNMapPairValue *pair1 = map->pair_values[index1];
    UNMapPairValue *pair2 = map->pair_values[index2];

    map->pair_values[index1] = pair2;
    map->pair_values[index2] = pair1;
}

__attribute__((unused)) UNMapPairValue *unmap_get_pair_at(UNMap *map, size_t index) {
    if (index > 0 || index <= map->items)
        return map->pair_values[index];
    return NULL;
}

__attribute__((unused)) UNMapPairValue *unmap_get_pair_from_key(UNMap *map, char *key) {
    size_t low = 0;
    size_t high = map->items;

    while (low < high) {
        size_t mid = (low + high) / 2;
        int c = strcmp(map->pair_values[mid]->key, key);
        if (c == 0)
            return map->pair_values[mid];
        else if (c < 0)
            low = mid + 1;
        else
            high = mid;
    }

    return NULL;
}

__attribute__((unused)) void *unmap_get_key_from_value(UNMap *map, void *value) {
    size_t low = 0;
    size_t high = map->items;

    while (low < high) {
        size_t mid = (low + high) / 2;
        bool c = map->pair_values[mid]->value == value;
        if (c == true)
            return map->pair_values[mid];
        else if (c == false)
            low = mid + 1;
        else
            high = mid;
    }

    return NULL;
}

bool unmap_exists_key_and_value(UNMap *map, char *key, void *value) {
    size_t low = 0;
    size_t high = map->items;

    while (low < high) {
        size_t mid = (low + high) / 2;
        bool c = (map->pair_values[mid]->value == value) && (strcmp(map->pair_values[mid]->key, key) == 0);
        if (c == true)
            return true;
        else if (c == false)
            low = mid + 1;
        else
            high = mid;
    }

    return false;
}

__attribute__((unused)) bool unmap_exists_pair(UNMap *map, UNMapPairValue *pair) {
    return unmap_exists_key_and_value(map, pair->key, pair->value);
}

__attribute__((unused)) bool unmap_exists_key(UNMap *map, char *key) {
    size_t low = 0;
    size_t high = map->items;

    while (low < high) {
        size_t mid = (low + high) / 2;
        bool c = (strcmp(map->pair_values[mid]->key, key) == 0);
        if (c == true)
            return true;
        else if (c == false)
            low = mid + 1;
        else
            high = mid;
    }

    return false;
}

__attribute__((unused)) bool unmap_compare(UNMap *map, UNMap *map2) {
    if (map->items != map2->items)
        return false;

    for (int i = 0; i < map->items; i++) {
        if (strcmp(map->pair_values[i]->key, map2->pair_values[i]->key) != 0)
            return false;
    }

    return true;
}

__attribute__((unused)) bool unmap_has_duplicate_key(UNMap *map) {
    for (int i = 0; i < map->items - 1; i++) {
        for (int j = i + 1; j < map->items; j++) {
            if (map->pair_values[i]->key != NULL) {
                if (strcmp(map->pair_values[i]->key, map->pair_values[j]->key) == 0 &&
                    map->pair_values[i]->key[0] != '\0') {
                    map->duplicate_key = map->pair_values[j]->key;
                    return true;
                }
            }
        }
    }

    return false;
}

__attribute__((unused)) __attribute__((unused)) void unmap_print(UNMap *map) {
    for (int i = 0; i < map->items; i++) {
        UNMapPairValue *pair = map->pair_values[i];
        printf("%s: %s\n", (char *) pair->key, (char *) pair->value);
    }
}