//
// Created by Ashwin Paudel on 2022-04-20.
//

#include "Utils.h"

namespace drast::utils {
bool isHexadecimal(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F') || c == '_';
}

bool isOctal(char c) {
    return c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
           c == '5' || c == '6' || c == '7' || c == '_';
}

bool isBinary(char c) { return (c == '0' || c == '1') || c == '_'; }

bool isNumber(char c) { return (c >= '0' && c <= '9') || c == '_' || c == '.'; }

bool isAlphaNumeric(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}
} // namespace drast::utils
