//
//  string.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef __DRAST_UTILS_STRING_H__
#define __DRAST_UTILS_STRING_H__

#include <stdbool.h>
#include <string.h>

bool string_is_hex(char *s);

bool string_is_float(char *s);

bool string_is_octal(char *s);

#endif //__DRAST_UTILS_STRING_H__
