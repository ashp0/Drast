//
//  string.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "string.h"

bool string_is_hex(const char *s) {
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        return true;
    else
        return false;
}

bool string_is_float(char *s) {
    size_t n = strlen(s);

    for (int i = 0; i < n; i++) {
        char ch = s[i];

        if (ch == '.')
            return true;
    }

    return false;
}

bool string_is_octal(char *s) {
    size_t n = strlen(s);

    if (s[0] != '0' || n == 1)
        return false;

    for (int i = 0; i < n; i++) {
        char c = s[i];

        if ((c - '0' >= 0) && (c - '0' <= 7))
            return true;
    }

    return false;
}

bool char_is_alpha(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
        return true;
    }
    return false;
}

bool char_is_alnum(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || (c >= '0' && c <= '9')) {
        return true;
    }
    return false;
}

bool char_is_digit(char c) {
    if ((c >= '0' && c <= '9') || c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-' || c == 'x' || c == 'X' ||
        c == 'o' || c == 'O' || c == 'b' || c == 'B' || c == 'f' || c == 'F')
        return true;
    else
        return false;
}

bool char_is_whitespace(char c) {
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        return true;
    else
        return false;
}

uint8_t string_compare(const char *str, const char *str2, size_t len) {
    while (len--)
        if (str[len] != str2[len])
            return 1;
    return 0;
}