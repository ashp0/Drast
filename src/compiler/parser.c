//
//  parser.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "parser.h"

static Parser parser;
mxDynamicArray *scopes;
int current_scope;

#define create_ast$(name, ast_type) \
    AST name = {                    \
        .type = (ast_type),         \
    }                               \

#define parser_error(message) \
    fprintf(stderr, "Parser: %s || Line: %zu\n", message, parser.current.line); \
    exit(EXIT_FAILURE);       \

#define ADD_DECLARATION_TO_HASHMAP(ast_type) \
    AST *scope = mxDynamicArrayGet(scopes, current_scope - 1); \
    mxHashmap_set(scope->value.CompoundStatement.declarations, \
    parser.current.value, parser.current.length, ast_type);

void parser_init(Lexer *lexer) {
    parser.lexer = lexer;
    parser.current = lexer_get_token();
    parser.next_token = lexer_get_token();
    scopes = mxDynamicArrayCreate(sizeof(AST));
    current_scope = 0;
}

AST parser_parse(void) {
    return parser_parse_compound();
}

AST parser_parse_compound(void) {
    create_ast$(compound, AST_TYPE_COMPOUND_STATEMENT);
    compound.value.CompoundStatement.declarations = mxHashmap_create();
    mxDynamicArrayAdd(scopes, &compound);
    current_scope += 1;
    compound.value.CompoundStatement.statements = mxDynamicArrayCreate(sizeof(AST *));
    while (parser.current.type != T_EOF) {
        if (parser.current.type == T_BRACE_CLOSE) {
            parser_advance(T_BRACE_CLOSE);
            break;
        }

        AST statement = parser_parse_statement();
        mxDynamicArrayAdd(compound.value.CompoundStatement.statements, &statement);
    }

    if (compound.value.CompoundStatement.declarations->collided) {
        parser_error("Duplicate Declarations");
    }

    current_scope -= 1;
    return compound;
}

AST parser_parse_statement(void) {
    switch (parser.current.type) {
        case T_K_EXTERN:
        case T_K_PRIVATE:
            return parser_parse_modifiers();
        case T_K_INT:
        case T_K_CHAR:
        case T_K_VOID:
        case T_K_BOOL:
        case T_K_FLOAT:
        case T_K_STRING:
        case T_IDENTIFIER:
            return parser_parse_function_or_variable_declaration(NULL);
        case T_K_IMPORT:
            return parser_parse_import();

    }
    parser_advance_without_check();

    AST dummy;
    return dummy;
}

AST parser_parse_import(void) {
    create_ast$(ast, AST_TYPE_IMPORT);
    parser_advance(T_K_IMPORT);

    ast.value.Import.import = &parser.current;

    parser_advance_without_check();

    return ast;
}


AST parser_parse_function_or_variable_declaration(mxBitmap *modifiers) {
    AST type = parser_parse_type();

    if (parser.current.type == T_DOUBLE_COLON) {
        parser_advance(T_DOUBLE_COLON);
        return parser_parse_function_declaration(modifiers, &type);
    } else {
        // Variable
        return parser_parse_variable_declaration(modifiers, &type);
    }

    parser_advance_without_check();

    AST dummy;
    return dummy;
}

AST parser_parse_function_declaration(mxBitmap *modifiers, AST *return_type) {
    create_ast$(function_declaration, AST_TYPE_FUNCTION_DECLARATION);
    function_declaration.value.FunctionDeclaration.modifiers = modifiers;

    // Set the declaration in the hashmap
    function_declaration.value.FunctionDeclaration.name = &parser.current;
    ADD_DECLARATION_TO_HASHMAP(AST_TYPE_FUNCTION_DECLARATION);

    function_declaration.value.FunctionDeclaration.return_type = return_type;

    parser_advance(T_IDENTIFIER);
    parser_advance(T_PARENS_OPEN);
    function_declaration.value.FunctionDeclaration.arguments = parser_parse_function_arguments();
    if (parser.current.type != T_BRACE_OPEN) {
        function_declaration.value.FunctionDeclaration.is_extern_or_header = true;
        // It's an extern or header function
        goto return_function;
    }

    parser_advance(T_BRACE_OPEN);
    AST compound = parser_parse_compound();
    function_declaration.value.FunctionDeclaration.body = &compound;

    return_function:
    return function_declaration;
}


mxDynamicArray *parser_parse_function_arguments(void) {
    mxDynamicArray *arguments = mxDynamicArrayCreate(sizeof(*arguments));

    while (parser.current.type != T_PARENS_CLOSE) {
        create_ast$(argument, AST_TYPE_TYPE);

        if (parser.current.type == T_PERIOD) {
            parser_advance(T_PERIOD);
            parser_advance(T_PERIOD);
            parser_advance(T_PERIOD);
            argument.value.FunctionArgument.is_vaarg = true;
            break;
        }

        AST type = parser_parse_type();
        argument.value.FunctionArgument.type = &type;
        argument.value.FunctionArgument.name = &parser.current;

        parser_advance(T_IDENTIFIER);

        mxDynamicArrayAdd(arguments, &argument);

        if (parser.current.type == T_COMMA) {
            parser_advance(T_COMMA);
        }
    }

    parser_advance(T_PARENS_CLOSE);

    return arguments;
};

AST parser_parse_variable_declaration(mxBitmap *modifiers, AST *variable_type) {
    create_ast$(variable_ast, AST_TYPE_VARIABLE_DECLARATION);

    variable_ast.value.VariableDeclaration.modifiers = modifiers;
    variable_ast.value.VariableDeclaration.type = variable_type;
    variable_ast.value.VariableDeclaration.name = &parser.current;
    ADD_DECLARATION_TO_HASHMAP(AST_TYPE_VARIABLE_DECLARATION)
    parser_advance(T_IDENTIFIER);

    if (parser.current.type == T_EQUAL) {
        parser_advance(T_EQUAL);
        variable_ast.value.VariableDeclaration.is_initialized = true;
        AST expression = parser_parse_expression();
        variable_ast.value.VariableDeclaration.initializer = &expression;
    }

    return variable_ast;
}

AST parser_parse_expression(void) {
    return parser_parse_equality();
}

AST parser_parse_equality(void) {
    AST equality_ast = parser_parse_comparison();

    while (parser.current.type == T_EQUAL_EQUAL || parser.current.type == T_NOT_EQUAL) {
        create_ast$(equality_operator, AST_TYPE_BINARY_EXPRESSION);
        equality_operator.value.BinaryExpression.operator = &parser.current;
        equality_operator.value.BinaryExpression.left = &equality_ast;
        parser_advance_without_check();
        AST right_ast = parser_parse_comparison();
        equality_operator.value.BinaryExpression.right = &right_ast;

        return equality_operator;
    }

    return equality_ast;
}

AST parser_parse_comparison(void) {
    AST term_ast = parser_parse_term();

    while (parser.current.type == T_LESS_THAN || parser.current.type == T_LESS_THAN_EQUAL ||
           parser.current.type == T_GREATER_THAN || parser.current.type == T_GREATER_THAN_EQUAL) {
        create_ast$(comparison_operator, AST_TYPE_BINARY_EXPRESSION);
        comparison_operator.value.BinaryExpression.operator = &parser.current;
        comparison_operator.value.BinaryExpression.left = &term_ast;
        parser_advance_without_check();
        AST right_ast = parser_parse_term();
        comparison_operator.value.BinaryExpression.right = &right_ast;

        return comparison_operator;
    }

    return term_ast;
}

AST parser_parse_term(void) {
    AST term_ast = parser_parse_factor();

    while (parser.current.type == T_OPERATOR_ADD || parser.current.type == T_OPERATOR_SUB ||
           parser.current.type == T_PERIOD) {
        create_ast$(term_operator, AST_TYPE_BINARY_EXPRESSION);
        term_operator.value.BinaryExpression.operator = &parser.current;
        term_operator.value.BinaryExpression.left = &term_ast;
        parser_advance_without_check();
        AST right_term = parser_parse_expression();
        term_operator.value.BinaryExpression.right = &right_term;


        return term_operator;
    }

    return term_ast;
}

AST parser_parse_factor(void) {
    AST unary_ast = parser_parse_unary();

    while (parser.current.type == T_OPERATOR_MUL || parser.current.type == T_OPERATOR_DIV ||
           parser.current.type == T_OPERATOR_MOD) {
        create_ast$(unary_operator, AST_TYPE_BINARY_EXPRESSION);
        unary_operator.value.BinaryExpression.operator = &parser.current;
        unary_operator.value.BinaryExpression.left = &unary_ast;
        parser_advance_without_check();
        AST right_side = parser_parse_expression();
        unary_operator.value.BinaryExpression.right = &right_side;
        unary_ast = unary_operator;
    }

    return unary_ast;
}

AST parser_parse_unary(void) {
    if (parser.current.type == T_NOT) {
        create_ast$(unary_ast, AST_TYPE_UNARY_EXPRESSION);
        unary_ast.value.UnaryExpression.operator = &parser.current;
        parser_advance_without_check();
        AST right_side = parser_parse_unary();
        unary_ast.value.UnaryExpression.right = &right_side;

        return unary_ast;
    }

    return parser_parse_primary();
}

AST parser_parse_primary(void) {
    if (parser.current.type == T_IDENTIFIER || parser.current.type == T_STRING || parser.current.type == T_NUMBER ||
        parser.current.type == T_FLOAT || parser.current.type == T_HEX || parser.current.type == T_OCTAL ||
        parser.current.type == T_CHAR || parser.current.type == T_K_SELF || parser.current.type == T_K_TRUE ||
        parser.current.type == T_K_FALSE) {
        create_ast$(literal_ast, AST_TYPE_LITERAL);
        literal_ast.value.Literal.token = &parser.current;

        if (parser.current.type == T_IDENTIFIER) {
            for (int i = current_scope; i > 0; --i) {
                AST *scope = mxDynamicArrayGet(scopes, i - 1);
                uintptr_t out = 0;
                if (mxHashmap_get(scope->value.CompoundStatement.declarations, parser.current.value,
                                  parser.current.length, &out)) {
                    if (out != AST_TYPE_VARIABLE_DECLARATION) {
                        parser_error("Function cannot be used as a type");
                    }
                    break;
                } else if (i == 1) {
                    parser_error("Undeclared variable");
                }
            }
        }

        parser_advance_without_check();

//        if (parser.current.type == T_PARENS_OPEN) {
//            function call
//        }

        return literal_ast;
    }

    if (parser.current.type == T_PARENS_OPEN) {
        parser_advance(T_PARENS_OPEN);
        create_ast$(inside_expression, AST_TYPE_GROUPING_EXPRESSION);
        AST expression = parser_parse_expression();
        inside_expression.value.Grouping.expression = &expression;
        parser_advance(T_PARENS_CLOSE);

        return inside_expression;
    }

    // Show error
    parser_error("Cannot Parse Literal")
}


AST parser_parse_type(void) {
    create_ast$(type, AST_TYPE_TYPE);

    switch (parser.current.type) {
        case T_K_INT:
        case T_K_CHAR:
        case T_K_VOID:
        case T_K_BOOL:
        case T_K_FLOAT:
        case T_K_STRING:
        case T_IDENTIFIER:
            break;
        default:
            // Error
//            fprintf(stderr, "Error: `%s`, cannot be used as a type\n", token_print(parser.current.type));
//            exit(1);
        parser_error("Cannot use this token as a type");
    }
    type.value.Type.token_type = &parser.current;
    parser_advance_without_check();

    for (;;) {
        switch (parser.current.type) {
            case T_QUESTION:
                type.value.Type.is_optional = true;
                break;
            case T_OPERATOR_MUL:
                type.value.Type.is_pointer = true;
                break;
            case T_SQUARE_OPEN:
                parser_advance(T_SQUARE_OPEN);
                type.value.Type.is_array = true;
                parser_advance(T_SQUARE_CLOSE);
            default:
                goto for_end;
        }

        parser_advance_without_check();
    }

    for_end:
    return type;
}

AST parser_parse_modifiers(void) {
    mxBitmap modifiers;

    for (;;) {
        switch (parser.current.type) {
            case T_K_EXTERN:
                mxBitmap_set(&modifiers, T_K_EXTERN);
                break;
            case T_K_PRIVATE:
                mxBitmap_set(&modifiers, T_K_PRIVATE);
                break;
            default:
                goto for_exit;
        }
        parser_advance_without_check();
    }

    for_exit:
    {};

    AST dummy;
    return dummy;
}

void parser_advance(int type) {
    if (parser.current.type != type && type != -1) {
        parser_error("Unexpected token");
    }
    parser.current = parser.next_token;
    parser.next_token = lexer_get_token();
}

void parser_advance_without_check(void) {
    parser.current = parser.next_token;
    parser.next_token = lexer_get_token();
}

Parser *parser_get(void) {
    return &parser;
}
