//
// Created by Ashwin Paudel on 2022-03-01.
//

#pragma once

#include <stdlib.h>

typedef struct {
    void **data;
    int capacity;
    int size;
} mxDynamicArray;

mxDynamicArray *mxDynamicArrayCreate(int capacity);

__attribute__((unused)) void mxDynamicArrayDestroy(mxDynamicArray *array);

void mxDynamicArrayAdd(mxDynamicArray *array, void *item);

__attribute__((unused)) void mxDynamicArrayRemove(mxDynamicArray *array, int index);

__attribute__((unused)) void mxDynamicArrayRemoveLast(mxDynamicArray *array);

__attribute__((unused)) void mxDynamicArrayRemoveAll(mxDynamicArray *array);

void *mxDynamicArrayGet(mxDynamicArray *array, int index);

__attribute__((unused)) void mxDynamicArraySet(mxDynamicArray *array, int index, void *item);

__attribute__((unused)) void mxDynamicArraySort(mxDynamicArray *array, int (*compare)(const void *, const void *));

__attribute__((unused)) void mxDynamicArrayReverse(mxDynamicArray *array);
