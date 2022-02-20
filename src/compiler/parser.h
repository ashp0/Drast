//
//  parser.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef __DRAST_COMPILER_PARSER_H__
#define __DRAST_COMPILER_PARSER_H__

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer *lexer;
    Token *current;
    Token *previous;
} Parser;

Parser *parser_init(Lexer *lexer);

AST *parser_parse(Parser *parser);

#endif // __DRAST_COMPILER_PARSER_H__