//
//  string.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "string.h"

bool string_is_hex(char *s) {
    size_t n = strlen(s);

    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        return true;
    else
        return false;

//    for(int i = 0; i < n; i++)
//    {
//        char c = s[i];
//
//        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
//            return true;
//        }
//    }
//
//    return false;
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