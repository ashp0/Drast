//
// Created by Ashwin Paudel on 2022-03-24.
//

#ifndef DRAST_PRINT_H
#define DRAST_PRINT_H

#include "Types.h"
#include <iostream>

namespace print {
int error(std::string msg, Location location);
int warning(std::string msg, Location location);
} // namespace print

#endif // DRAST_PRINT_H