//
// Files.cpp
// Created by Ashwin Paudel on 2022-04-20.
//
// =============================================================================
///
/// \file
/// This file is a source file of the Utils.h file, this has the implementation
/// of file related functions.
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
void readFile(const std::string &file_name, std::string &file_buffer) {
    FILE *fp;
    char *line = nullptr;
    size_t length = 0;
    ssize_t read;

    fp = fopen(file_name.c_str(), "r");
    if (fp == nullptr) {
        fprintf(stderr, "Error: failed to open source file: %s\n",
                file_name.c_str());
        exit(1);
    }

    while ((read = getline(&line, &length, fp)) != -1) {
        file_buffer += line;
    }

    fclose(fp);
    if (line)
        free(line);

    if (file_buffer.empty()) {
        fprintf(stderr, "Error: encountered empty file: %s\n",
                file_buffer.c_str());
    }
}
} // namespace drast::utils