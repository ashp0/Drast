//
// Types.h
// Created by Ashwin Paudel on 2022-03-24.
//
// =============================================================================
///
/// \file
/// This file contains the declaration of the variety of structs and classes,
/// which is used by the many classes.
///
// =============================================================================
//
// Copyright (c) 2022, Drast Programming Language Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//
// =============================================================================
//
// Contributed by:
//  - Ashwin Paudel <ashwonixer123@gmail.com>
//
// =============================================================================

#ifndef DRAST_TYPES_H
#define DRAST_TYPES_H

#include <string>

struct Location {
    uint32_t line;
    uint32_t column;

    Location(uint32_t line, uint32_t column) : line(line), column(column) {}

    [[nodiscard]] std::string toString() const {
        return "L(" + std::to_string(line) + "), C(" + std::to_string(column) +
               ")";
    }
};

#endif // DRAST_TYPES_H
