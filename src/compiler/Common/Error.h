//
// Error.h
// Created by Ashwin Paudel on 2022-04-17.
//
// =============================================================================
///
/// \file
/// This file contains the declaration of the Error class. Which is used by the
/// Lexer, Parser, Semantic Analyzer and Code Generator to report errors.
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

#ifndef DRAST_COMPILER_COMMON_ERROR_H
#define DRAST_COMPILER_COMMON_ERROR_H

#include "Types.h"
#include <iostream>
#include <sstream>
#include <vector>

struct Error {
    const char *file_name;
    std::string &buffer;

    std::vector<std::pair<std::string, Location>> error_messages;
    std::vector<std::pair<std::string, Location>> warning_messages;

  public:
    void addError(const char *msg, Location location);

    void addError(const std::string &msg, Location location);

    void addWarning(const char *msg, Location location);

    void addWarning(const std::string &msg, Location location);

    void displayMessages();

    void displayMessage(const std::pair<std::string, Location> &message,
                        bool is_warning);
};

#endif // DRAST_COMPILER_COMMON_ERROR_H
