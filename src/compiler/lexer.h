//
//  lexer.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "token.h"
#include "../utils/string.h"
#include "../utils/unmap.h"

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

__attribute__((unused)) Token *lexer_get_next_token_without_advance_offset(Lexer *lexer, uintptr_t offset);

Token *lexer_get_next_token(Lexer *lexer);

__attribute__((unused)) bool is_next_token_operator(Lexer *lexer);

Lexer *lexer_duplicate(Lexer *lexer);

// Lexer Functions

Token *lexer_parse_character(Lexer *lexer);

Token *lexer_parse_string(Lexer *lexer);

uintptr_t lexer_get_line_count(Lexer *lexer);

Token *lexer_parse_number(Lexer *lexer);

Token *lexer_parse_identifier(Lexer *lexer);

Token *lexer_is_keyword(char *identifier, Lexer *lexer);

void lexer_skip_line(Lexer *lexer);

void lexer_skip_block_comment(Lexer *lexer);

void lexer_advance(Lexer *lexer);

void lexer_check_eof(Lexer *lexer, char *error_message);

bool lexer_is_eof(Lexer *lexer);

Token *lexer_advance_token(int token_type, char *value, Lexer *lexer, bool does_not_advance);

char lexer_peek_next(Lexer *lexer);

bool lexer_is_whitespace(Lexer *lexer);


void lexer_skip_whitespace(Lexer *lexer);