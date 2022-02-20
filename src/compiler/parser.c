//
//  parser.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "parser.h"

static inline Token *advance(Parser *parser, uintptr_t type);

static inline Token *advance_without_check(Parser *parser);

static inline void advance_semi(Parser *parser);

static AST *parse_statement(Parser *parser);

static inline AST *parse_import(Parser *parser);

static inline AST *parse_variable(Parser *parser, bool is_constant);

Parser *parser_init(Lexer *lexer) {
    Parser *parser = malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current = lexer_get_next_token(parser->lexer);
    parser->previous = parser->current;

    return parser;
}

AST *parser_parse(Parser *parser) {
    return parse_statement(parser);
}

static inline AST *parse_statement(Parser *parser) {
    switch (parser->current->type) {
        case T_IMPORT:
            return parse_import(parser);
        case T_VAR:
            return parse_variable(parser, false);
        case T_LET:
            return parse_variable(parser, true);
        default:
            fprintf(stderr, "Parser: Cannot Parse Token: `%s`\n", token_print(parser->current->type));
            exit(-2);
    }
}

static inline AST *parse_import(Parser *parser) {
    AST *tree = ast_init_with_type(AST_TYPE_IMPORT);

    advance(parser, T_IMPORT);
    char *identifier = advance(parser, T_IDENTIFIER)->value;

    if (parser->current->type == T_PERIOD) {
        advance(parser, T_PERIOD);

        tree->value.Import.file = identifier;
        tree->value.Import.is_library = false;

        return tree;
    }

    tree->value.Import.file = identifier;
    tree->value.Import.is_library = true;

    return tree;
}

static inline AST *parse_variable(Parser *parser, bool is_constant) {
    AST *tree = is_constant ? ast_init_with_type(AST_TYPE_LET_DEFINITION) : ast_init_with_type(
            AST_TYPE_VARIABLE_DEFINITION);

    is_constant ? advance(parser, T_LET) : advance(parser, T_VAR);

    // TODO: Check for volatile

    char *identifier = advance(parser, T_IDENTIFIER)->value;

    if (parser->current->type == T_EQUAL) {
        advance(parser, T_COLON);
    } else if (parser->current->type == T_COLON) {
        advance(parser, T_COLON);
    } else {
        fprintf(stderr, "Parser: Unexpected Token: `%s`, was expecting `=` or `:`\n",
                token_print(parser->current->type));
        exit(-2);
    }

    tree->value.Variable.identifier = identifier;
    tree->value.Variable.is_constant = is_constant;

    return tree;
}

static inline Token *advance(Parser *parser, uintptr_t type) {
    if (parser->current->type != type) {
        fprintf(stderr, "Parser: Unexpected Token: `%s`, was expecting `%s`\n", token_print(parser->current->type),
                token_print((int) type));
        exit(-2);
    }
    parser->previous = parser->current;
    parser->current = lexer_get_next_token(parser->lexer);

    return parser->previous;
}

static inline Token *advance_without_check(Parser *parser) {
    parser->previous = parser->current;
    parser->current = lexer_get_next_token(parser->lexer);

    return parser->current;
}

static inline void advance_semi(Parser *parser) {
    if (lexer_get_next_token_without_advance(parser->lexer)->type == T_SEMICOLON) {
        advance(parser, T_SEMICOLON);
    }
}