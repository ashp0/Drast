//
// Created by Ashwin Paudel on 2022-03-13.
//

#ifndef DRAST_MXBITMAP_H
#define DRAST_MXBITMAP_H

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t mxBitmap;

void mxBitmap_set(mxBitmap *bitmap, uint32_t attribute);

void mxBitmap_unset(mxBitmap *bitmap, uint32_t attribute);

bool mxBitmap_exists(mxBitmap *bitmap, uint32_t attribute);

#endif /* DRAST_MXBITMAP_H */
