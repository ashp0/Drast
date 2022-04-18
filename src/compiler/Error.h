//
// Created by Ashwin Paudel on 2022-04-17.
//

#ifndef DRAST_ERROR_H
#define DRAST_ERROR_H

#include "Types.h"
#include <iostream>
#include <sstream>
#include <vector>

struct Error {
    const char *file_name;
    std::string &buffer;

    std::vector<std::pair<std::string, Location>> error_messages;
    size_t message_index = 0;

  public:
    void append(const char *msg, Location location);

    void append(const std::string &msg, Location location);

    void displayErrors();

    void displayError(const std::pair<std::string, Location> &error_message);
};

#endif // DRAST_ERROR_H
