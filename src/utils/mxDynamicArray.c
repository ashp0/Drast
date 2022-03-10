//
// Created by Ashwin Paudel on 2022-03-01.
//

#include "mxDynamicArray.h"

mxDynamicArray *mxDynamicArrayCreate(int capacity) {
    mxDynamicArray *array = malloc(sizeof(mxDynamicArray));
    array->capacity = capacity;
    array->size = 0;
    array->data = malloc(sizeof(void *) * capacity);
    return array;
}

__attribute__((unused)) void mxDynamicArrayDestroy(mxDynamicArray *array) {
    free(array->data);
    free(array);
}

void mxDynamicArrayAdd(mxDynamicArray *array, void *item) {
    if (array->size == array->capacity) {
        array->capacity *= 2;
        array->data = realloc(array->data, sizeof(void *) * array->capacity);
    }
    array->data[array->size] = item;
    array->size += 1;
}

__attribute__((unused)) void mxDynamicArrayRemove(mxDynamicArray *array, int index) {
    if (index >= array->size) {
        return;
    }
    for (int i = index; i < array->size - 1; i++) {
        array->data[i] = array->data[i + 1];
    }
    array->size--;
}

__attribute__((unused)) void mxDynamicArrayRemoveLast(mxDynamicArray *array) {
    array->size--;
}

__attribute__((unused)) void mxDynamicArrayRemoveAll(mxDynamicArray *array) {
    array->size = 0;
}

void *mxDynamicArrayGet(mxDynamicArray *array, int index) {
    if (index >= array->size) {
        return NULL;
    }
    return array->data[index];
}

__attribute__((unused)) void mxDynamicArraySet(mxDynamicArray *array, int index, void *item) {
    if (index >= array->size) {
        return;
    }
    array->data[index] = item;
}

__attribute__((unused)) void mxDynamicArraySort(mxDynamicArray *array, int (*compare)(const void *, const void *)) {
    qsort(array->data, array->size, sizeof(void *), compare);
}

__attribute__((unused)) void mxDynamicArrayReverse(mxDynamicArray *array) {
    for (int i = 0; i < array->size / 2; i++) {
        void *temp = array->data[i];
        array->data[i] = array->data[array->size - i - 1];
        array->data[array->size - i - 1] = temp;
    }
}
