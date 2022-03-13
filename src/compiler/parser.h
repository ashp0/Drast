//
//  parser.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef PARSE_H
#define PARSE_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer *lexer;
    Token current;
    Token next_token;
} Parser;

void parser_init(Lexer *lexer);

#endif /* PARSE_H */