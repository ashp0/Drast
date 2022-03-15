//
// Created by Ashwin Paudel on 2022-03-14.
//

#ifndef DRAST_MXHASHMAP_H
#define DRAST_MXHASHMAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#define MX_HASHMAP_SIZE 16

typedef struct mxHashmapEntry {
    char *key;
    size_t key_len;
    uintptr_t value;
    uint8_t hash;
    struct mxHashmapEntry *next;
} mxHashmapEntry;

typedef struct mxHashmap {
    mxHashmapEntry *entries;
    size_t count;
    size_t capacity;
    bool collided;
    char *collided_key;
} mxHashmap;

mxHashmap *mxHashmap_create(void);

void mxHashmap_set(mxHashmap *map, char *key, size_t key_len, uintptr_t value);

bool mxHashmap_get(mxHashmap *map, char *key, size_t key_len, uintptr_t *out);

void mxHashmap_free(mxHashmap *map);

uint8_t mxHashmap_get_hash(mxHashmap *map, const char *key, size_t key_len);

#endif //DRAST_MXHASHMAP_H
