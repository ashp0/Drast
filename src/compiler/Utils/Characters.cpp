//
// Characters.cpp
// Created by Ashwin Paudel on 2022-04-20.
//
// =============================================================================
///
/// \file
/// This file is a source file of the Utils.h file, this has the implementation
/// of character and string related functions.
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

#include "Utils.h"

namespace drast::utils {
bool isHexadecimal(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F') || c == '_';
}

bool isOctal(char c) {
    return c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
           c == '5' || c == '6' || c == '7' || c == '_';
}

bool isBinary(char c) { return (c == '0' || c == '1') || c == '_'; }

bool isNumber(char c) { return (c >= '0' && c <= '9') || c == '_'; }

bool isAlphaNumeric(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

bool isWhitespace(char c) {
    return c == ' ' || c == '\t' /*|| c == '\n' || c == '\r'*/;
}
} // namespace drast::utils
