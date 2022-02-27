//
//  unmap.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef __DRAST_UTILS_UNMAP_H__
#define __DRAST_UTILS_UNMAP_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    char *key;
    void *value;
} UNMapPairValue;

typedef struct {
    size_t items;
    size_t capacity;
    UNMapPairValue **pair_values;
} UNMap;

UNMap *unmap_init(void);

void unmap_insert_at(UNMap *map, uintptr_t index, UNMapPairValue *pair);

void unmap_push_back(UNMap *map, UNMapPairValue *pair);

UNMapPairValue *unmap_pop_back(UNMap *map);

void unmap_append(UNMap *map, UNMap *other);

void unmap_destroy(UNMap *map);

void unmap_remove_all(UNMap *map);

void unmap_remove_at(UNMap *map, size_t index);

void unmap_swap(UNMap *map, size_t index1, size_t index2);

UNMapPairValue *unmap_get_pair_at(UNMap *map, size_t index);

UNMapPairValue *unmap_get_pair_from_key(UNMap *map, char *key);

void *unmap_get_key_from_value(UNMap *map, void *value);

bool unmap_exists_key_and_value(UNMap *map, char *key, void *value);

bool unmap_exists_pair(UNMap *map, UNMapPairValue *pair);

bool unmap_exists_key(UNMap *map, char *key);

bool unmap_compare(UNMap *map, UNMap *map2);

void unmap_print(UNMap *map);

#endif // __DRAST_UTILS_UNMAP_H__