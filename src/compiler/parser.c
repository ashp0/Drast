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

static inline AST *parse_function(Parser *parser);

static inline AST *parse_variable(Parser *parser, bool is_constant);

static inline AST *parse_type_name(Parser *parser);

static inline AST *parse_inner_statement(Parser *parser);

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
        case T_K_IMPORT:
            return parse_import(parser);
        case T_K_FUNC:
            return parse_function(parser);
        case T_K_VAR:
        case T_K_LET:
        case T_BRACE_OPEN:
        case T_BRACE_CLOSE:
        case T_K_IF:
            fprintf(stderr, "Parser: Cannot declare`%s` outside of scope\n", token_print(parser->current->type));
            exit(-2);
        default:
            fprintf(stderr, "Parser: Cannot Parse Token: `%s`\n", token_print(parser->current->type));
            exit(-2);
    }
}

static inline AST *parse_inner_statement(Parser *parser) {
    switch (parser->current->type) {
        case T_K_VAR:
            return parse_variable(parser, false);
        case T_K_LET:
            return parse_variable(parser, true);
        default:
            fprintf(stderr, "Parser: Token `%s`, is not supposed to be declared inside curly braces\n",
                    token_print(parser->current->type));
            exit(-2);
    }
}

//static inline AST *parse_expression(Parser *parser) {
//    // var myVariable = (50 + 20) - 50
//    // var myVariable = (50 + myVariable) - myOtherVariable
//    switch (parser->current->type) {
//        case
//    }
//}

static inline AST *parse_import(Parser *parser) {
    AST *tree = ast_init_with_type(AST_TYPE_IMPORT);

    advance(parser, T_K_IMPORT);
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

static inline AST *parse_function(Parser *parser) {
    AST *new_ast = ast_init_with_type(AST_TYPE_FUNCTION_DECLARATION);
    advance(parser, T_K_FUNC);

    if (parser->current->type == T_K_PRIVATE) {
        new_ast->value.FunctionDeclaration.is_private = true;
        advance(parser, T_IDENTIFIER);
    }

    new_ast->value.FunctionDeclaration.function_name = advance(parser, T_IDENTIFIER)->value;

    // Parse Arguments
    advance(parser, T_PARENS_OPEN);

    new_ast->value.FunctionDeclaration.argument_size = 0;
    new_ast->value.FunctionDeclaration.arguments = malloc(sizeof(AST *));

    while (parser->current->type != T_PARENS_CLOSE) {
        new_ast->value.FunctionDeclaration.argument_size += 1;

        new_ast->value.FunctionDeclaration.arguments = realloc(new_ast->value.FunctionDeclaration.arguments,
                                                               new_ast->value.FunctionDeclaration.argument_size *
                                                               sizeof(AST *));

        AST *argument = ast_init_with_type(AST_TYPE_FUNCTION_ARGUMENT);
        argument->value.FunctionArgument.argument_name = advance(parser, T_IDENTIFIER)->value;
        advance(parser, T_COLON);
        argument->value.FunctionArgument.argument_type = parse_type_name(parser);

        new_ast->value.FunctionDeclaration.arguments[new_ast->value.FunctionDeclaration.argument_size - 1] = argument;

        if (parser->current->type == T_PARENS_CLOSE) {
            advance(parser, T_PARENS_CLOSE);
            break;
        } else {
            advance(parser, T_COMMA);
        }
    }

    // Return Type
    advance(parser, T_ARROW);
    new_ast->value.FunctionDeclaration.return_type = parse_type_name(parser);

    // Parse inner statement
    advance(parser, T_BRACE_OPEN);

    new_ast->value.FunctionDeclaration.body_size = 0;
    new_ast->value.FunctionDeclaration.body = malloc(sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        new_ast->value.FunctionDeclaration.body_size += 1;

        new_ast->value.FunctionDeclaration.body = realloc(new_ast->value.FunctionDeclaration.body,
                                                          new_ast->value.FunctionDeclaration.body_size *
                                                          sizeof(AST *));

        new_ast->value.FunctionDeclaration.body[new_ast->value.FunctionDeclaration.body_size -
                                                1] = parse_inner_statement(parser);
    }

    return new_ast;
}

static inline AST *parse_variable(Parser *parser, bool is_constant) {
    AST *tree = is_constant ? ast_init_with_type(AST_TYPE_LET_DEFINITION) : ast_init_with_type(
            AST_TYPE_VARIABLE_DEFINITION);

    is_constant ? advance(parser, T_K_LET) : advance(parser, T_K_VAR);

    if (parser->current->type == T_K_VOLATILE) {
        tree->value.Variable.is_volatile = true;
        advance(parser, T_K_VOLATILE);
    } else {
        tree->value.Variable.is_volatile = false;
    }

    char *identifier = advance(parser, T_IDENTIFIER)->value;
    tree->value.Variable.identifier = identifier;
    tree->value.Variable.is_constant = is_constant;

    if (parser->current->type == T_EQUAL) {
        advance(parser, T_COLON);

        // TODO: Parse expression
    } else if (parser->current->type == T_COLON) {
        advance(parser, T_COLON);

        tree->value.Variable.is_initialized = false;
        tree->value.Variable.value = parse_type_name(parser);

        // TODO: If there is an equal sign, also check if the they are casting
        return tree;
    } else {
        fprintf(stderr, "Parser: Unexpected Token: `%s`, was expecting `=` or `:`\n",
                token_print(parser->current->type));
        exit(-2);
    }

    return tree;
}

static inline AST *parse_type_name(Parser *parser) {
    if (parser->current->type == T_K_INT || parser->current->type == T_K_FLOAT || parser->current->type == T_K_BOOL ||
        parser->current->type == T_K_STRING) {
        AST *new_ast = ast_init_with_type(AST_TYPE_VALUE_KEYWORD);
        new_ast->value.ValueKeyword.token = parser->current;

        if (lexer_get_next_token_without_advance(parser->lexer)->type == T_SQUARE_OPEN) {
            advance(parser, new_ast->value.ValueKeyword.token->type);
            advance(parser, T_SQUARE_OPEN);
            advance(parser, T_SQUARE_CLOSE);
            new_ast->value.ValueKeyword.is_array = true;

            return new_ast;
        }

        advance(parser, new_ast->value.ValueKeyword.token->type);

        return new_ast;
    } else {
        fprintf(stderr, "Parser: Unexpected Token Value `%s`\n", token_print(parser->current->type));
        exit(-2);
    }
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