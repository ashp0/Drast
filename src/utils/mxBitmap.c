//
// Created by Ashwin Paudel on 2022-03-13.
//

#include "mxBitmap.h"

void mxBitmap_set(mxBitmap *bitmap, uint32_t attribute) {
    *bitmap |= attribute;
}

void mxBitmap_unset(mxBitmap *bitmap, uint32_t attribute) {
    *bitmap &= ~attribute;
}

bool mxBitmap_exists(mxBitmap *bitmap, uint32_t attribute) {
    return *bitmap & attribute;
}