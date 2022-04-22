//
// Utils.h
// Created by Ashwin Paudel on 2022-04-09.
//
// =============================================================================
///
/// \file
/// This file contains the declaration of a useful set of functions, which are
/// used by the many classes.
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

#ifndef DRAST_UTILS_H
#define DRAST_UTILS_H

#include <iostream>
#include <sstream>
#include <string>

namespace drast::utils {
bool isHexadecimal(char c);

bool isOctal(char c);

bool isBinary(char c);

bool isNumber(char c);

bool isAlphaNumeric(char c);

bool isWhitespace(char c);

void readFile(const std::string &file_name, std::string &file_buffer);
} // namespace drast::utils

#endif // DRAST_UTILS_H
