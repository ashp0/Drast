//
// Created by Ashwin Paudel on 2022-03-14.
//

#include "mxHashmap.h"

#define MX_HASHMAP_INITIAL_CAPACITY 20
#define MX_HASHMAP_RESIZE_FACTOR 2

static void mxHashmap_resize(mxHashmap *map);

mxHashmap *mxHashmap_create(void) {
    mxHashmap *map = malloc(sizeof(mxHashmap));
    map->count = 0;
    map->capacity = MX_HASHMAP_INITIAL_CAPACITY;
    map->entries = malloc(sizeof(mxHashmapEntry) * map->capacity);
    map->collided = false;

    // Set all entries to NULL
    for (int i = 0; i < map->capacity; i++) {
        map->entries[i].key = NULL;
        map->entries[i].value = 0;
    }

    return map;
}

void mxHashmap_set(mxHashmap *map, char *key, size_t key_len, uintptr_t value) {
    if (map->count >= map->capacity) {
        mxHashmap_resize(map);
    }

    uint8_t hash = mxHashmap_get_hash(map, key, key_len);
    mxHashmapEntry *entry = &map->entries[hash];

    if (entry->key == NULL) {
        entry = malloc(sizeof(mxHashmapEntry));
        entry->key = key;
        entry->key_len = key_len;
        entry->value = value;
        map->entries[hash] = *entry;
        map->count++;
    } else {
        map->collided = true; // for the semantic analyzer to check for duplicate declarations
        map->collided_key = key;
        entry->value = value;
    }
}

bool mxHashmap_get(mxHashmap *map, char *key, size_t key_len, uintptr_t *out) {
    uint8_t hash = mxHashmap_get_hash(map, key, key_len);
    mxHashmapEntry *entry = &map->entries[hash];
    if (entry->key == NULL) {
        return false;
    }

    *out = entry->value;
    return true;
}

void mxHashmap_free(mxHashmap *map) {
    free(map->entries);
    free(map);
}


static void mxHashmap_resize(mxHashmap *map) {
    mxHashmapEntry *entries = map->entries;
    size_t capacity = map->capacity;
    size_t new_capacity = capacity * MX_HASHMAP_RESIZE_FACTOR;
    mxHashmapEntry *new_entries = malloc(sizeof(mxHashmapEntry) * new_capacity);
    for (size_t i = 0; i < capacity; i++) {
        mxHashmapEntry *entry = &entries[i];
        if (entry->key) {
            size_t hash = mxHashmap_get_hash(map, entry->key, entry->key_len);
            mxHashmapEntry *new_entry = &new_entries[hash];
            new_entry->key = entry->key;
            new_entry->key_len = entry->key_len;
            new_entry->value = entry->value;
        }
    }
    free(map->entries);
    map->entries = new_entries;
    map->capacity = new_capacity;
}

uint8_t mxHashmap_get_hash(mxHashmap *map, const char *key, size_t key_len) {
    uint8_t hash = 0;

    for (int i = 0; i < key_len; i++) {
        hash = key[i] + (hash << 6) + (hash << 16) - hash;
    }

    hash = hash % map->capacity;

    return hash;
}
