//
//  string.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#pragma once

#include <stdbool.h>
#include <string.h>

bool string_is_hex(char *s);

bool string_is_float(char *s);

bool string_is_octal(char *s);
