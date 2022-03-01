//
//  unmap.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#pragma once

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
    char *duplicate_key;
} UNMap;

UNMap *unmap_init(void);

__attribute__((unused)) void unmap_insert_at(UNMap *map, uintptr_t index, UNMapPairValue *pair);

void unmap_push_back(UNMap *map, UNMapPairValue *pair);

__attribute__((unused)) UNMapPairValue *unmap_pop_back(UNMap *map);

__attribute__((unused)) void unmap_append(UNMap *map, UNMap *other);

void unmap_destroy(UNMap *map);

__attribute__((unused)) void unmap_remove_all(UNMap *map);

void unmap_remove_at(UNMap *map, size_t index);

__attribute__((unused)) void unmap_swap(UNMap *map, size_t index1, size_t index2);

__attribute__((unused)) UNMapPairValue *unmap_get_pair_at(UNMap *map, size_t index);

__attribute__((unused)) UNMapPairValue *unmap_get_pair_from_key(UNMap *map, char *key);

__attribute__((unused)) void *unmap_get_key_from_value(UNMap *map, void *value);

bool unmap_exists_key_and_value(UNMap *map, char *key, void *value);

__attribute__((unused)) bool unmap_exists_pair(UNMap *map, UNMapPairValue *pair);

__attribute__((unused)) bool unmap_exists_key(UNMap *map, char *key);

__attribute__((unused)) bool unmap_compare(UNMap *map, UNMap *map2);

__attribute__((unused)) bool unmap_has_duplicate_key(UNMap *map);

__attribute__((unused)) void unmap_print(UNMap *map);