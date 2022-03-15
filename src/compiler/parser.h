//
//  parser.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef DRAST_PARSE_H
#define DRAST_PARSE_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer *lexer;
    Token current;
    Token next_token;
} Parser;

void parser_init(Lexer *lexer);

AST parser_parse(void);

AST parser_parse_compound(void);

AST parser_parse_statement(void);

AST parser_parse_import(void);

AST parser_parse_function_or_variable_declaration(mxBitmap *modifiers);

AST parser_parse_function_declaration(mxBitmap *modifiers, AST *return_type);

mxDynamicArray *parser_parse_function_arguments(void);

AST parser_parse_variable_declaration(mxBitmap *modifiers, AST *variable_type);

AST parser_parse_expression(void);

AST parser_parse_equality(void);

AST parser_parse_comparison(void);

AST parser_parse_term(void);

AST parser_parse_factor(void);

AST parser_parse_unary(void);

AST parser_parse_primary(void);

AST parser_parse_type(void);

AST parser_parse_modifiers(void);

void parser_advance(int type);

void parser_advance_without_check(void);

Parser *parser_get(void);

#endif /* DRAST_PARSE_H */