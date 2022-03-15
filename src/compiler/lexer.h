//
//  lexer.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef DRAST_LEXER_H
#define DRAST_LEXER_H

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

void lexer_init(char *source, long length);

Token lexer_get_token(void);

Token lexer_identifier(void);

Token lexer_digit(void);

Token lexer_string(void);

Token lexer_character(void);

Token lexer_make_token(char *value, size_t length, bool advances, TokenType type);

void lexer_skip_whitespace(void);

void lexer_skip_line(void);

void lexer_skip_block_comment(void);

void lexer_advance(void);

char lexer_peek(void);

Lexer *lexer_get(void);

#endif /* DRAST_LEXER_H */