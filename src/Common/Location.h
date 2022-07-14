//
// Created by Ashwin Paudel on 2022-06-02.
//

#ifndef DRAST_LOCATION_H
#define DRAST_LOCATION_H

#include <string>
#include <cstdio>

struct Location {
    size_t line;
    size_t column;

    Location(size_t line, size_t column) : line(line), column(column) {}

    [[nodiscard]] std::string toString() const {
        return "L(" + std::to_string(line) + "), C(" + std::to_string(column) +
               ")";
    }
};


#endif //DRAST_LOCATION_H