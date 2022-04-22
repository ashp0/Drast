//
// Created by Ashwin Paudel on 2022-04-09.
//

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

void readFile(const std::string &file_name, std::string &file_buffer);
} // namespace drast::utils

#endif // DRAST_UTILS_H
