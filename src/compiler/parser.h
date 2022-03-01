//
//  parser.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#pragma once

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer *lexer;
    Token *current;
} Parser;

Parser *parser_init(Lexer *lexer);

AST *parser_parse(Parser *parser);

AST *parser_parse_statement(Parser *parser);

AST *parser_parse_inner_statement(Parser *parser);

AST *parser_parse_struct_statement(Parser *parser);

AST *parser_parse_struct_members(Parser *parser);

AST *parser_parse_struct_initializer(Parser *parser);

// Maybe add support for __attribute_((packed))?
AST *parser_parse_struct(Parser *parser, bool is_private);

AST *parser_parse_function_call(Parser *parser, char *function_name);

AST *parser_parse_function_or_variable_declaration(Parser *parser, bool is_inner_statement);

AST *parser_parse_variable_declaration(Parser *parser, AST *variable_type, bool is_private, bool is_volatile);

AST *parser_parse_function_declaration(Parser *parser, AST *return_type, bool is_private);

AST *parser_parse_inline_asm(Parser *parser);

AST *parser_parse_import_statement(Parser *parser);

AST *parser_parse_enum(Parser *parser, bool is_private);

AST *parser_parse_return(Parser *parser);

AST *parser_parse_if_else_statement(Parser *parser);

AST *parser_parse_while_statement(Parser *parser);

AST *parser_parse_for_loop(Parser *parser);

AST *parser_parse_do_catch_statement(Parser *parser);

AST *parser_parse_switch_statement(Parser *parser);

AST *parser_parse_matches_statement(Parser *parser);

AST *parser_parse_expression_try(Parser *parser);

AST *parser_parse_alias(Parser *parser);

AST *parser_parse_body(Parser *parser);

AST *parser_parse_expression(Parser *parser);

AST *parser_parse_expression_grouping(Parser *parser);

AST *parser_parse_expression_literal(Parser *parser);

AST *parser_parse_expression_identifier(Parser *parser);

bool parser_is_expression(Token *token);

AST *parser_parse_type(Parser *parser);

Token *parser_advance(Parser *parser, int token_type);

Token *parser_advance_without_check(Parser *parser);