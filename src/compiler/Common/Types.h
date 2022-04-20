//
// Created by Ashwin Paudel on 2022-03-24.
//

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
