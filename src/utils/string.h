//
//  string.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#pragma once

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

bool string_is_hex(const char *s);

bool string_is_float(char *s);

bool string_is_octal(char *s);

bool char_is_alpha(char c);

bool char_is_alnum(char c);

bool char_is_digit(char c);

bool char_is_whitespace(char c);

uint8_t string_compare(const char *str, const char *str2, size_t len);