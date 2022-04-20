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

#endif // DRAST_ERROR_H
