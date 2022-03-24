//
// Created by Ashwin Paudel on 2022-03-24.
//

#include "Print.h"

static constexpr auto c_error = "\033[1;31m";
static constexpr auto c_warning = "\033[1;33m";
static constexpr auto c_reset = "\033[0m";

namespace print {
int error(std::string msg, Location location) {
    std::cout << c_error << "Error: " << msg << " || L`" << location.line
              << "`C`" << location.column << c_reset << std::endl;
    exit(-1);
}

int warning(std::string msg, Location location) {
    std::cout << c_warning << "Warning: " << msg << " || L`" << location.line
              << "`C`" << location.column << c_reset << std::endl;
    return 0;
}
} // namespace print