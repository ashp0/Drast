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

static inline AST *parse_struct(Parser *parser);

static inline AST *parse_struct_statements(Parser *parser);

static inline AST *parse_enum(Parser *parser);

static inline AST *parse_expression(Parser *parser);

static inline AST *parse_equality(Parser *parser);

static inline AST *parse_comparison(Parser *parser);

static inline AST *parse_term(Parser *parser);

static inline AST *parse_unary(Parser *parser);

static inline AST *parse_primary(Parser *parser);

static inline AST *parse_return(Parser *parser);

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
        case T_K_STRUCT:
            return parse_struct(parser);
        case T_K_ENUM:
            return parse_enum(parser);
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
        case T_K_RETURN:
            return parse_return(parser);
        default:
            fprintf(stderr, "Parser: Token `%s`, is not supposed to be declared inside curly braces\n",
                    token_print(parser->current->type));
            exit(-2);
    }
}

static inline AST *parse_struct_statements(Parser *parser) {
    switch (parser->current->type) {
        case T_K_VAR:
            return parse_variable(parser, false);
        case T_K_LET:
            return parse_variable(parser, true);
        default:
            fprintf(stderr, "Parser: Token `%s`, is not supposed to be declared inside of structs\n",
                    token_print(parser->current->type));
            exit(-2);
    }
}

static inline AST *parse_expression(Parser *parser) {
    Token *next_token = lexer_get_next_token_without_advance(parser->lexer);
    if (next_token->type == T_OPERATOR_ADD || next_token->type == T_OPERATOR_SUB ||
        next_token->type == T_OPERATOR_MUL || next_token->type == T_OPERATOR_DIV) {
        return parse_term(parser);
    } else if (next_token->type == T_EQUAL_EQUAL || next_token->type == T_NOT_EQUAL ||
               next_token->type == T_GREATER_THAN || next_token->type == T_GREATER_THAN_EQUAL ||
               next_token->type == T_LESS_THAN || next_token->type == T_LESS_THAN_EQUAL) {
        return parse_equality(parser);
    } else if (parser->current->type == T_K_TRUE || parser->current->type == T_K_FALSE ||
               parser->current->type == T_INT || parser->current->type == T_STRING ||
               parser->current->type == T_FLOAT || parser->current->type == T_PARENS_OPEN ||
               parser->current->type == T_IDENTIFIER) {
        return parse_primary(parser);
    } else {
        return parse_equality(parser);
    }
}

static inline AST *parse_equality(Parser *parser) {
    AST *left_expr = parse_unary(parser);
    AST *ast = ast_init_with_type(AST_TYPE_BINARY);

    while (parser->current->type == T_EQUAL_EQUAL || parser->current->type == T_NOT_EQUAL ||
           parser->current->type == T_GREATER_THAN || parser->current->type == T_GREATER_THAN_EQUAL ||
           parser->current->type == T_LESS_THAN || parser->current->type == T_LESS_THAN_EQUAL) {
        Token *operator = parser->current;

        advance(parser, operator->type);

        AST *right_expr = parse_unary(parser);

        ast->value.Binary.left = left_expr;
        ast->value.Binary.operator = operator;
        ast->value.Binary.right = right_expr;
    }

    return ast;
}

static inline AST *parse_term(Parser *parser) {
    AST *tree = ast_init_with_type(AST_TYPE_BINARY);
    AST *left_factor = parse_unary(parser);

    while (parser->current->type == T_OPERATOR_ADD || parser->current->type == T_OPERATOR_SUB ||
           parser->current->type == T_OPERATOR_MUL || parser->current->type == T_OPERATOR_DIV) {
        Token *operator = parser->current;
        advance(parser, operator->type);
        AST *right_factor = parse_unary(parser);

        tree->value.Binary.left = left_factor;
        tree->value.Binary.operator = operator;
        tree->value.Binary.right = right_factor;
    }

    return tree;
}

static inline AST *parse_unary(Parser *parser) {
    AST *tree = ast_init_with_type(AST_TYPE_UNARY);

    if (parser->current->type == T_OPERATOR_SUB || parser->current->type == T_NOT) {
        Token *operator = parser->current;
        advance(parser, operator->type);

        AST *right = parse_unary(parser);

        tree->value.Unary.operator = operator;
        tree->value.Unary.right = right;

        return tree;
    }

    return parse_primary(parser);
}

static inline AST *parse_primary(Parser *parser) {
    AST *tree = ast_init_with_type(AST_TYPE_LITERAL);

    if (parser->current->type == T_K_TRUE || parser->current->type == T_K_FALSE || parser->current->type == T_INT ||
        parser->current->type == T_STRING || parser->current->type == T_FLOAT ||
        parser->current->type == T_IDENTIFIER) {
        tree->value.Literal.literal_value = parser->current;
        advance(parser, tree->value.Literal.literal_value->type);

        return tree;
    } else if (parser->current->type == T_PARENS_OPEN) {
        advance(parser, T_PARENS_OPEN);

        AST *expression = parse_expression(parser);
        AST *expression_tree = ast_init_with_type(AST_TYPE_GROUPING);
        advance(parser, T_PARENS_CLOSE);

        expression_tree->value.Grouping.expression = expression;

        return expression_tree;
    }
    return NULL;
}

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
        advance(parser, T_K_PRIVATE);
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
    if (parser->current->type == T_ARROW) {
    	advance(parser, T_ARROW);
        new_ast->value.FunctionDeclaration.has_return_type = true;
    	new_ast->value.FunctionDeclaration.return_type = parse_type_name(parser);
    } else {
        new_ast->value.FunctionDeclaration.has_return_type = false;
        new_ast->value.FunctionDeclaration.return_type = NULL;
    }

    // Parse inner statement
    advance(parser, T_BRACE_OPEN);

    new_ast->value.FunctionDeclaration.body_size = 0;
    new_ast->value.FunctionDeclaration.body = calloc(1, sizeof(AST *));

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
        advance(parser, T_EQUAL);

        tree->value.Variable.is_initialized = true;
        tree->value.Variable.value = parse_expression(parser);

        return tree;
        // TODO: Parse expression
    } else if (parser->current->type == T_COLON) {
        advance(parser, T_COLON);

        tree->value.Variable.is_initialized = false;
        tree->value.Variable.value = parse_type_name(parser);

        // TODO: If there is an equal sign, also check if the they are casting
        if (parser->current->type == T_EQUAL) {
            advance(parser, T_EQUAL);

            tree->value.Variable.is_initialized = true;
            tree->value.Variable.value = parse_expression(parser);
        }
        return tree;
    } else {
        fprintf(stderr, "Parser: Unexpected Token: `%s`, was expecting `=` or `:`\n",
                token_print(parser->current->type));
        exit(-2);
    }

    return tree;
}

static inline AST *parse_enum(Parser *parser) {
    AST *new_ast = ast_init_with_type(AST_TYPE_ENUM_DECLARATION);
    advance(parser, T_K_ENUM);
    char *enum_name = advance(parser, T_IDENTIFIER)->value;

    new_ast->value.EnumDeclaration.enum_name = enum_name;

    advance(parser, T_BRACE_OPEN);

    new_ast->value.EnumDeclaration.case_size = 0;
    new_ast->value.EnumDeclaration.cases = calloc(1, sizeof(AST *));

    int case_counter = 0;
    while (parser->current->type != T_BRACE_CLOSE) {
        new_ast->value.EnumDeclaration.case_size += 1;

        new_ast->value.EnumDeclaration.cases = realloc(new_ast->value.EnumDeclaration.cases,
                                                       new_ast->value.EnumDeclaration.case_size *
                                                       sizeof(AST *));

        AST *enum_case = ast_init_with_type(AST_TYPE_ENUM_ITEM);
        advance(parser, T_K_CASE);
        enum_case->value.EnumItem.case_name = advance(parser, T_IDENTIFIER)->value;

        if (parser->current->type == T_EQUAL) {
            advance(parser, T_EQUAL);
            char *case_counter_string = advance(parser, T_INT)->value;
            case_counter = (int) strtol(case_counter_string, &case_counter_string, 10);
        }

        enum_case->value.EnumItem.case_value = case_counter;
        advance(parser, T_COMMA);

        new_ast->value.EnumDeclaration.cases[new_ast->value.StructDeclaration.member_size -
                                             1] = enum_case;

        case_counter++;
    }

    advance(parser, T_BRACE_CLOSE);
    return new_ast;
}

static inline AST *parse_struct(Parser *parser) {
    AST *new_ast = ast_init_with_type(AST_TYPE_STRUCT_DECLARATION);
    advance(parser, T_K_STRUCT);
    char *struct_name = advance(parser, T_IDENTIFIER)->value;

    new_ast->value.StructDeclaration.struct_name = struct_name;

    advance(parser, T_BRACE_OPEN);


    new_ast->value.StructDeclaration.member_size = 0;
    new_ast->value.StructDeclaration.members = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        new_ast->value.StructDeclaration.member_size += 1;

        new_ast->value.StructDeclaration.members = realloc(new_ast->value.StructDeclaration.members,
                                                           new_ast->value.StructDeclaration.member_size *
                                                           sizeof(AST *));

        new_ast->value.StructDeclaration.members[new_ast->value.StructDeclaration.member_size -
                                                 1] = parse_struct_statements(parser);
    }

    advance(parser, T_BRACE_CLOSE);

    return new_ast;
}

static inline AST *parse_return(Parser *parser) {
    AST *new_ast = ast_init_with_type(AST_TYPE_RETURN);

    advance(parser, T_K_RETURN);
    new_ast->value.Return.return_expression = parse_expression(parser);

    return new_ast;
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
