//
//  lexer.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef __DRAST_COMPILER_LEXER_H__
#define __DRAST_COMPILER_LEXER_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "token.h"

typedef struct {
    char *source;
    uintptr_t source_length;

    uintptr_t index;
    char current;

    uintptr_t line;
    uintptr_t position;
} Lexer;

Lexer *lexer_init(char *source);

Token *lexer_get_next_token_without_advance(Lexer *lexer);

Token *lexer_get_next_token(Lexer *lexer);

bool is_next_token_operator(Lexer *lexer);

#endif // __DRAST_COMPILER_LEXER_H__
