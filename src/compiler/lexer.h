//
//  lexer.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>
#include "token.h"
#include "../utils/string.h"

typedef struct {
    size_t line;
    size_t column;
} Position;

typedef struct {
    char *source;
    size_t source_length;

    size_t start;
    size_t index;
    char current;

    Position position;
} Lexer;

void lexer_init(char *source);

Token lexer_get_token(void);

Token lexer_identifier(void);

Token lexer_digit(void);

Token lexer_string(void);

Token lexer_character(void);

Token lexer_make_token(char *value, TokenType type, bool advances);

void lexer_skip_whitespace(void);

void lexer_skip_line(void);

void lexer_skip_block_comment(void);

void lexer_advance(void);

char lexer_peek(void);

#endif /* LEXER_H */