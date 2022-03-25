//
// Created by Ashwin Paudel on 2022-03-24.
//

#ifndef DRAST_PRINT_H
#define DRAST_PRINT_H

#include "Types.h"
#include <iostream>
#include <sstream>

class Print {
  private:
    static constexpr auto c_error = "\033[1;31m";
    static constexpr auto c_warning = "\033[1;33m";
    static constexpr auto c_reset = "\033[0m";

    std::string &source;
    std::string &file_name;

  public:
    Print(std::string &source, std::string &file_name)
        : source(source), file_name(file_name) {}

    int error(std::string msg, Location location);

    int warning(std::string msg, Location location);
};

#endif // DRAST_PRINT_H