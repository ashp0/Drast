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

static inline AST *parse_struct_or_union(Parser *parser, bool is_union);

static inline AST *parse_struct_statements(Parser *parser);

static inline AST *parse_enum(Parser *parser);

static inline AST *parse_expression(Parser *parser);

static inline AST *parse_equality(Parser *parser);

static inline AST *parse_term(Parser *parser);

static inline AST *parse_unary(Parser *parser);

static inline AST *parse_primary(Parser *parser);

static inline AST *parse_return(Parser *parser);

static inline AST *parse_literal(Parser *parser);

static inline AST *parse_function_call(Parser *parser);

static inline AST *parse_identifier(Parser *parser);

static inline AST *parse_variable_call(Parser *parser);

static inline AST *parse_if_else_statement(Parser *parser, bool is_else);

static inline AST *parse_while_statement(Parser *parser);

static inline AST *parse_asm(Parser *parser);

static inline AST *parse_switch_statement(Parser *parser);

static inline AST *parse_for_loop(Parser *parser);

static inline AST *parse_do_catch_statement(Parser *parser);

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

static inline AST *parse_variable_or_function_definition(Parser *parser) {
    if (lexer_get_next_token_without_advance(parser->lexer)->type == T_DOUBLE_COLON) {
        return parse_function(parser);
    } else {
        return parse_variable(parser, false);
    }

}

static inline AST *parse_statement(Parser *parser) {
    switch (parser->current->type) {
        case T_K_ASM:
            return parse_asm(parser);
        case T_K_INT:
        case T_K_STRING:
        case T_K_CHAR:
        case T_K_BOOL:
        case T_K_FLOAT:
        case T_K_VOID:
            return parse_variable_or_function_definition(parser);
        case T_K_IMPORT:
            return parse_import(parser);
        case T_K_STRUCT:
            return parse_struct_or_union(parser, false);
        case T_K_UNION:
            return parse_struct_or_union(parser, true);
        case T_K_ENUM:
            return parse_enum(parser);
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
        case T_K_SWITCH:
            return parse_switch_statement(parser);
        case T_K_DO:
            // TODO: Check for while do statements
            return parse_do_catch_statement(parser);
        case T_K_INT:
        case T_K_STRING:
        case T_K_CHAR:
        case T_K_BOOL:
        case T_K_FLOAT:
        case T_K_VOID:
            return parse_variable_or_function_definition(parser);
        case T_K_RETURN:
            return parse_return(parser);
        case T_IDENTIFIER:
            return parse_identifier(parser);
        case T_K_IF:
            return parse_if_else_statement(parser, false);
        case T_K_ELSE:
            return parse_if_else_statement(parser, true);
        case T_K_WHILE:
            return parse_while_statement(parser);
        case T_K_FOR:
            return parse_for_loop(parser);
        case T_K_MATCHES:
            return parse_function_call(parser);
        default:
            fprintf(stderr, "Parser: Token `%s`, is not supposed to be declared inside curly braces\n",
                    token_print(parser->current->type));
            exit(-2);
    }
}

static inline AST *parse_do_catch_statement(Parser *parser) {
    AST *new_ast = ast_init_with_type(AST_TYPE_DO_CATCH);

    /* Do Statements */
    advance(parser, T_K_DO);
    advance(parser, T_BRACE_OPEN);
    // Parse Body
    new_ast->value.DoCatchStatement.do_body_size = 0;
    new_ast->value.DoCatchStatement.do_body = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        new_ast->value.DoCatchStatement.do_body_size += 1;

        new_ast->value.DoCatchStatement.do_body = realloc(new_ast->value.DoCatchStatement.do_body,
                                                          new_ast->value.DoCatchStatement.do_body_size *
                                                          sizeof(AST *));

        new_ast->value.DoCatchStatement.do_body[new_ast->value.DoCatchStatement.do_body_size -
                                                1] = parse_inner_statement(parser);
    }

    advance(parser, T_BRACE_CLOSE);
    advance_semi(parser);

    /* Catch Statements */
    advance(parser, T_K_CATCH);

    advance(parser, T_BRACE_OPEN);

    new_ast->value.DoCatchStatement.catch_body_size = 0;
    new_ast->value.DoCatchStatement.catch_body = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        new_ast->value.DoCatchStatement.catch_body_size += 1;

        new_ast->value.DoCatchStatement.catch_body = realloc(new_ast->value.DoCatchStatement.catch_body,
                                                             new_ast->value.DoCatchStatement.catch_body_size *
                                                             sizeof(AST *));
        new_ast->value.DoCatchStatement.catch_body[new_ast->value.DoCatchStatement.catch_body_size -
                                                   1] = parse_inner_statement(parser);
    }

    advance(parser, T_BRACE_CLOSE);
    advance_semi(parser);

    return new_ast;
}

static inline AST *parse_matches_statement(Parser *parser, AST *argument) {
    AST *new_tree = ast_init_with_type(AST_TYPE_SWITCH_STATEMENT);
    new_tree->value.SwitchStatement.is_matches_statement = true;

    if (argument->value.FunctionCall.arguments_size != 1) {
        fprintf(stderr, "Parser: Matches Token Must Have Only 1 Parameter\n",
                token_print(parser->current->type));
        exit(-2);
    }

    new_tree->value.SwitchStatement.expression = argument->value.FunctionCall.arguments[0];

    new_tree->value.SwitchStatement.switch_cases_size = 0;
    new_tree->value.SwitchStatement.switch_cases = calloc(1, sizeof(AST *));

    advance(parser, T_BRACE_OPEN);
    while (parser->current->type != T_BRACE_CLOSE) {
        new_tree->value.SwitchStatement.switch_cases_size += 1;

        new_tree->value.SwitchStatement.switch_cases = realloc(new_tree->value.SwitchStatement.switch_cases,
                                                               new_tree->value.SwitchStatement.switch_cases_size *
                                                               sizeof(AST *));

        AST *switch_case = ast_init_with_type(AST_TYPE_SWITCH_CASE);

        if (parser->current->type == T_IDENTIFIER && strlen(parser->current->value) == 1 &&
            parser->current->value[0] == '_') {
            // Default Clause
            switch_case->value.SwitchCase.is_default = true;
        }

        switch_case->value.SwitchCase.expression = parse_expression(parser);

        advance(parser, T_COLON);

        switch_case->value.SwitchCase.body = calloc(1, sizeof(AST *));

        if (parser->current->type == T_BRACE_OPEN) {
            advance(parser, T_BRACE_OPEN);

            while (parser->current->type != T_BRACE_CLOSE) {
                switch_case->value.SwitchCase.body_size += 1;
                switch_case->value.SwitchCase.body = realloc(switch_case->value.SwitchCase.body,
                                                             switch_case->value.SwitchCase.body_size *
                                                             sizeof(AST *));

                switch_case->value.SwitchCase.body[switch_case->value.SwitchCase.body_size - 1] = parse_inner_statement(
                        parser);
                if (parser->current->type == T_BRACE_CLOSE)
                    break;
            }

            advance(parser, T_BRACE_CLOSE);
            advance_semi(parser);
        } else {
            switch_case->value.SwitchCase.body_size = 1;
            switch_case->value.SwitchCase.body[switch_case->value.SwitchCase.body_size - 1] = parse_expression(parser);
            advance_semi(parser);
        }

        new_tree->value.SwitchStatement.switch_cases[new_tree->value.SwitchStatement.switch_cases_size -
                                                     1] = switch_case;

        if (parser->current->type == T_BRACE_CLOSE) {
            break;
        }
    }

    advance(parser, T_BRACE_CLOSE);
    advance_semi(parser);

    return new_tree;
}

static inline AST *parse_switch_statement(Parser *parser) {
    AST *new_tree = ast_init_with_type(AST_TYPE_SWITCH_STATEMENT);

    advance(parser, T_K_SWITCH);
    advance(parser, T_PARENS_OPEN);
    new_tree->value.SwitchStatement.expression = parse_expression(parser);
    advance(parser, T_PARENS_CLOSE);

    advance(parser, T_BRACE_OPEN);

    new_tree->value.SwitchStatement.switch_cases_size = 0;
    new_tree->value.SwitchStatement.switch_cases = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        new_tree->value.SwitchStatement.switch_cases_size += 1;

        new_tree->value.SwitchStatement.switch_cases = realloc(new_tree->value.SwitchStatement.switch_cases,
                                                               new_tree->value.SwitchStatement.switch_cases_size *
                                                               sizeof(AST *));
        bool is_default = false;
        if (parser->current->type == T_K_CASE)
            advance(parser, T_K_CASE);
        else {
            is_default = true;
            advance(parser, T_K_DEFAULT);
        }

        AST *switch_case = ast_init_with_type(AST_TYPE_SWITCH_CASE);
        if (!is_default)
            switch_case->value.SwitchCase.expression = parse_expression(parser);
        else
            switch_case->value.SwitchCase.is_default = true;

        advance(parser, T_COLON);

        switch_case->value.SwitchCase.body_size = 0;
        switch_case->value.SwitchCase.body = calloc(1, sizeof(AST *));

        while (parser->current->type != T_BRACE_CLOSE) {
            switch_case->value.SwitchCase.body_size += 1;

            switch_case->value.SwitchCase.body = realloc(switch_case->value.SwitchCase.body,
                                                         switch_case->value.SwitchCase.body_size *
                                                         sizeof(AST *));

            if (parser->current->type == T_K_BREAK) {
                AST *literal_value = ast_init_with_type(AST_TYPE_LITERAL);
                literal_value->value.Literal.literal_value = parser->current;
                switch_case->value.SwitchCase.body[switch_case->value.SwitchCase.body_size -
                                                   1] = literal_value;
                advance(parser, T_K_BREAK);
            } else {
                switch_case->value.SwitchCase.body[switch_case->value.SwitchCase.body_size -
                                                   1] = parse_inner_statement(parser);
            }

            if (parser->current->type == T_BRACE_CLOSE || parser->current->type == T_K_CASE ||
                parser->current->type == T_K_DEFAULT) {
                break;
            }
        }

        new_tree->value.SwitchStatement.switch_cases[new_tree->value.SwitchStatement.switch_cases_size -
                                                     1] = switch_case;
    }
    advance(parser, T_BRACE_CLOSE);
    advance_semi(parser);

    return new_tree;
}

static inline AST *parse_asm(Parser *parser) {
    AST *new_tree = ast_init_with_type(AST_TYPE_INLINE_ASSEMBLY);

    advance(parser, T_K_ASM);
    advance(parser, T_BRACE_OPEN);

    new_tree->value.InlineAssembly.instructions_size = 0;
    new_tree->value.InlineAssembly.instruction = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        new_tree->value.InlineAssembly.instructions_size += 1;

        new_tree->value.InlineAssembly.instruction = realloc(new_tree->value.InlineAssembly.instruction,
                                                             new_tree->value.InlineAssembly.instructions_size *
                                                             sizeof(AST *));

        new_tree->value.InlineAssembly.instruction[new_tree->value.InlineAssembly.instructions_size -
                                                   1] = advance(parser, T_STRING)->value;
    }

    advance(parser, T_BRACE_CLOSE);
    advance_semi(parser);

    return new_tree;
}

static inline AST *parse_if_else_statement(Parser *parser, bool is_else) {
    AST *new_ast = ast_init_with_type(AST_TYPE_IF_ELSE_STATEMENT);

    is_else ? advance(parser, T_K_ELSE) : advance(parser, T_K_IF);

    if (is_else) {
        if (parser->current->type == T_K_IF) {
            new_ast->value.IfElseStatement.is_else_if_statement = true;
            advance(parser, T_K_IF);
        } else {
            new_ast->value.IfElseStatement.is_else_statement = true;
        }
    } else {
        new_ast->value.IfElseStatement.is_else_if_statement = false;
        new_ast->value.IfElseStatement.is_else_statement = false;
    }

    // The expression

    if (!new_ast->value.IfElseStatement.is_else_statement) {
        advance(parser, T_PARENS_OPEN);
        new_ast->value.IfElseStatement.expression = parse_expression(parser);
        advance(parser, T_PARENS_CLOSE);
    }

    // The body
    advance(parser, T_BRACE_OPEN);

    new_ast->value.IfElseStatement.body_size = 0;
    new_ast->value.IfElseStatement.body = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        new_ast->value.IfElseStatement.body_size += 1;

        new_ast->value.IfElseStatement.body = realloc(new_ast->value.IfElseStatement.body,
                                                      new_ast->value.IfElseStatement.body_size *
                                                      sizeof(AST *));

        new_ast->value.IfElseStatement.body[new_ast->value.IfElseStatement.body_size -
                                            1] = parse_inner_statement(parser);
    }

    advance(parser, T_BRACE_CLOSE);
    advance_semi(parser);

    return new_ast;
}

static inline AST *parse_while_statement(Parser *parser) {
    AST *new_ast = ast_init_with_type(AST_TYPE_WHILE_STATEMENT);

    advance(parser, T_K_WHILE);

    // The expression
    advance(parser, T_PARENS_OPEN);
    new_ast->value.WhileStatement.expression = parse_expression(parser);
    advance(parser, T_PARENS_CLOSE);

    // The body
    advance(parser, T_BRACE_OPEN);

    new_ast->value.WhileStatement.body_size = 0;
    new_ast->value.WhileStatement.body = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        new_ast->value.WhileStatement.body_size += 1;

        new_ast->value.WhileStatement.body = realloc(new_ast->value.WhileStatement.body,
                                                     new_ast->value.WhileStatement.body_size *
                                                     sizeof(AST *));

        if (parser->current->type == T_K_CONTINUE || parser->current->type == T_K_BREAK) {
            AST *literal_value = ast_init_with_type(AST_TYPE_LITERAL);
            literal_value->value.Literal.literal_value = parser->current;
            new_ast->value.WhileStatement.body[new_ast->value.WhileStatement.body_size -
                                               1] = literal_value;
            advance_without_check(parser);
        } else {
            new_ast->value.WhileStatement.body[new_ast->value.WhileStatement.body_size -
                                               1] = parse_inner_statement(parser);
        }
    }

    advance(parser, T_BRACE_CLOSE);
    advance_semi(parser);

    return new_ast;
}

static inline AST *parse_struct_statements(Parser *parser) {
    switch (parser->current->type) {
        case T_K_INT:
        case T_K_STRING:
        case T_K_CHAR:
        case T_K_BOOL:
        case T_K_FLOAT:
        case T_K_VOID:
            return parse_variable_or_function_definition(parser);
        default:
            fprintf(stderr, "Parser: Token `%s`, is not supposed to be declared inside of structs\n",
                    token_print(parser->current->type));
            exit(-2);
    }
}

static inline AST *parse_for_loop(Parser *parser) {
    AST *tree = ast_init_with_type(AST_TYPE_FOR_LOOP);

    // Parse the arguments
    advance(parser, T_K_FOR);
    advance(parser, T_PARENS_OPEN);
    tree->value.ForLoop.variable = parse_variable(parser, false);
    advance(parser, T_COMMA);
    tree->value.ForLoop.condition = parse_expression(parser);
    advance(parser, T_COMMA);
    tree->value.ForLoop.condition2 = parse_expression(parser);
    advance(parser, T_PARENS_CLOSE);

    advance(parser, T_BRACE_OPEN);

    // Parse the body
    tree->value.ForLoop.body_size = 0;
    tree->value.ForLoop.body = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        tree->value.ForLoop.body_size += 1;

        tree->value.ForLoop.body = realloc(tree->value.ForLoop.body,
                                           tree->value.ForLoop.body_size *
                                           sizeof(AST *));

        if (parser->current->type == T_K_CONTINUE || parser->current->type == T_K_BREAK) {
            AST *literal_value = ast_init_with_type(AST_TYPE_LITERAL);
            literal_value->value.Literal.literal_value = parser->current;
            tree->value.ForLoop.body[tree->value.ForLoop.body_size -
                                     1] = literal_value;
            advance_without_check(parser);
        } else {
            tree->value.ForLoop.body[tree->value.ForLoop.body_size -
                                     1] = parse_inner_statement(parser);
        }
    }

    advance(parser, T_BRACE_CLOSE);
    advance_semi(parser);

    return tree;
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
    advance_semi(parser);

    tree->value.Import.file = identifier;
    tree->value.Import.is_library = true;

    return tree;
}

static inline AST *parse_identifier(Parser *parser) {
    advance(parser, T_IDENTIFIER);

    if (parser->current->type == T_PARENS_OPEN) {
        return parse_function_call(parser);
    } else {
        return parse_variable_call(parser);
    }
}

static inline AST *parse_variable_call(Parser *parser) {
    AST *new_ast = ast_init_with_type(AST_TYPE_VARIABLE_CALL);
    new_ast->value.VariableCall.variable_name = parser->previous->value;

    new_ast->value.VariableCall.is_expression = true;

    new_ast->value.VariableCall.operator = advance_without_check(parser);
    new_ast->value.VariableCall.expression = parse_expression(parser);
    advance_semi(parser);

    return new_ast;
}

static inline AST *parse_function_call(Parser *parser) {
    AST *new_ast = ast_init_with_type(AST_TYPE_FUNCTION_CALL);
    Token *prev_token = parser->previous;
    new_ast->value.FunctionCall.function_call_name = prev_token->value;

    // Parse the arguments
    advance(parser, T_PARENS_OPEN);

    new_ast->value.FunctionCall.arguments_size = 0;
    new_ast->value.FunctionCall.arguments = malloc(sizeof(AST *));

    while (parser->current->type != T_PARENS_CLOSE) {
        new_ast->value.FunctionCall.arguments_size += 1;

        new_ast->value.FunctionCall.arguments = realloc(new_ast->value.FunctionCall.arguments,
                                                        new_ast->value.FunctionCall.arguments_size *
                                                        sizeof(AST *));

        AST *argument = ast_init_with_type(AST_TYPE_LITERAL);
        argument->value.Literal.literal_value = parser->current;
        advance(parser, argument->value.Literal.literal_value->type);

        new_ast->value.FunctionCall.arguments[new_ast->value.FunctionCall.arguments_size - 1] = argument;

        if (parser->current->type != T_PARENS_CLOSE)
            advance(parser, T_COMMA);
    }

    advance(parser, T_PARENS_CLOSE);

    if (prev_token->type == T_K_MATCHES)
        return parse_matches_statement(parser, new_ast);

    advance_semi(parser);

    return new_ast;
}

static inline AST *parse_function(Parser *parser) {
    AST *new_ast = ast_init_with_type(AST_TYPE_FUNCTION_DECLARATION);
    new_ast->value.FunctionDeclaration.has_return_type = true;
    new_ast->value.FunctionDeclaration.return_type = parse_type_name(parser);

    advance(parser, T_DOUBLE_COLON);

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
        argument->value.FunctionDeclarationArgument.argument_type = parse_type_name(parser);
        argument->value.FunctionDeclarationArgument.argument_name = advance(parser, T_IDENTIFIER)->value;

        new_ast->value.FunctionDeclaration.arguments[new_ast->value.FunctionDeclaration.argument_size - 1] = argument;


        if (parser->current->type == T_COMMA)
            advance(parser, T_COMMA);
    }

    advance(parser, T_PARENS_CLOSE);
    // External Functions??

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
    advance(parser, T_BRACE_CLOSE);
    advance_semi(parser);

    return new_ast;
}

static inline AST *parse_variable(Parser *parser, bool is_constant) {
    // float myVariable = 4
    // volatile float myVariable = 3
    AST *tree = is_constant ? ast_init_with_type(AST_TYPE_LET_DEFINITION) : ast_init_with_type(
            AST_TYPE_VARIABLE_DEFINITION);
    if (parser->current->type == T_K_VOLATILE) {
        tree->value.VariableDeclaration.is_volatile = true;
        advance(parser, T_K_VOLATILE);
    } else {
        tree->value.VariableDeclaration.is_volatile = false;
    }
    tree->value.VariableDeclaration.type = parse_type_name(parser);
    char *identifier = advance(parser, T_IDENTIFIER)->value;
    tree->value.VariableDeclaration.identifier = identifier;
    if (parser->current->type == T_QUESTION) {
        tree->value.VariableDeclaration.type->value.ValueKeyword.is_optional = true;
        advance(parser, T_QUESTION);
        advance_semi(parser);
    }

    if (parser->current->type == T_EQUAL) {
        advance(parser, T_EQUAL);

        tree->value.VariableDeclaration.is_initialized = true;
        tree->value.VariableDeclaration.value = parse_expression(parser);

        advance_semi(parser);
        return tree;
    }

    tree->value.VariableDeclaration.is_initialized = false;
    advance_semi(parser);

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

        new_ast->value.EnumDeclaration.cases[new_ast->value.EnumDeclaration.case_size -
                                             1] = enum_case;

        case_counter++;
    }

    advance(parser, T_BRACE_CLOSE);
    advance_semi(parser);
    return new_ast;
}

static inline AST *parse_struct_or_union(Parser *parser, bool is_union) {
    AST *new_ast = ast_init_with_type(AST_TYPE_STRUCT_OR_UNION_DECLARATION);

    if (is_union)
        advance(parser, T_K_UNION);
    else
        advance(parser, T_K_STRUCT);

    char *name = advance(parser, T_IDENTIFIER)->value;

    new_ast->value.StructOrUnionDeclaration.name = name;

    advance(parser, T_BRACE_OPEN);


    new_ast->value.StructOrUnionDeclaration.member_size = 0;
    new_ast->value.StructOrUnionDeclaration.members = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        new_ast->value.StructOrUnionDeclaration.member_size += 1;

        new_ast->value.StructOrUnionDeclaration.members = realloc(new_ast->value.StructOrUnionDeclaration.members,
                                                                  new_ast->value.StructOrUnionDeclaration.member_size *
                                                                  sizeof(AST *));

        new_ast->value.StructOrUnionDeclaration.members[new_ast->value.StructOrUnionDeclaration.member_size -
                                                        1] = parse_struct_statements(parser);
    }

    advance(parser, T_BRACE_CLOSE);
    advance_semi(parser);

    if (is_union)
        new_ast->value.StructOrUnionDeclaration.is_union = true;
    else
        new_ast->value.StructOrUnionDeclaration.is_union = false;

    return new_ast;
}

static inline AST *parse_return(Parser *parser) {
    AST *new_ast = ast_init_with_type(AST_TYPE_RETURN);

    advance(parser, T_K_RETURN);
    new_ast->value.Return.return_expression = parse_expression(parser);
    advance_semi(parser);

    return new_ast;
}

static inline AST *parse_type_name(Parser *parser) {
    if (parser->current->type == T_K_INT || parser->current->type == T_K_FLOAT || parser->current->type == T_K_BOOL ||
        parser->current->type == T_K_STRING || parser->current->type == T_K_VOID || parser->current->type == T_K_CHAR) {
        AST *new_ast = ast_init_with_type(AST_TYPE_VALUE_KEYWORD);
        new_ast->value.ValueKeyword.token = parser->current;

        if (lexer_get_next_token_without_advance(parser->lexer)->type == T_SQUARE_OPEN) {
            advance(parser, new_ast->value.ValueKeyword.token->type);
            advance(parser, T_SQUARE_OPEN);
            advance(parser, T_SQUARE_CLOSE);

            new_ast->value.ValueKeyword.is_array = true;

            if (parser->current->type == T_OPERATOR_MUL) {
                advance(parser, T_OPERATOR_MUL);
                new_ast->value.ValueKeyword.is_pointer = true;
            }

            return new_ast;
        }

        advance(parser, new_ast->value.ValueKeyword.token->type);
        if (parser->current->type == T_OPERATOR_MUL) {
            advance(parser, T_OPERATOR_MUL);
            new_ast->value.ValueKeyword.is_pointer = true;
        }

        return new_ast;
    } else {
        fprintf(stderr, "Parser: Unexpected Token Value `%s`\n", token_print(parser->current->type));
        exit(-2);
    }
}


static inline AST *parse_expression(Parser *parser) {
    Token *next_token = lexer_get_next_token_without_advance(parser->lexer);
    if (next_token->type == T_OPERATOR_ADD || next_token->type == T_OPERATOR_SUB ||
        next_token->type == T_OPERATOR_MUL || next_token->type == T_OPERATOR_DIV ||
        next_token->type == T_OPERATOR_MOD || next_token->type == T_BITWISE_PIPE ||
        next_token->type == T_BITWISE_SHIFT_LEFT || next_token->type == T_BITWISE_SHIFT_RIGHT ||
        next_token->type == T_BITWISE_POWER || next_token->type == T_BITWISE_NOT) {
        return parse_term(parser);
    } else if (next_token->type == T_EQUAL_EQUAL || next_token->type == T_NOT_EQUAL ||
               next_token->type == T_GREATER_THAN || next_token->type == T_GREATER_THAN_EQUAL ||
               next_token->type == T_LESS_THAN || next_token->type == T_LESS_THAN_EQUAL ||
               next_token->type == T_OPERATOR_ADD_EQUAL || next_token->type == T_OPERATOR_SUB_EQUAL ||
               next_token->type == T_OPERATOR_MUL_EQUAL || next_token->type == T_OPERATOR_DIV_EQUAL ||
               next_token->type == T_OPERATOR_MOD_EQUAL ||
               next_token->type == T_BITWISE_AND_EQUAL || next_token->type == T_BITWISE_AND_AND_EQUAL ||
               next_token->type == T_BITWISE_PIPE_EQUAL || next_token->type == T_BITWISE_PIPE_PIPE_EQUAL ||
               next_token->type == T_BITWISE_SHIFT_LEFT_EQUAL || next_token->type == T_BITWISE_SHIFT_RIGHT_EQUAL ||
               next_token->type == T_BITWISE_POWER_EQUAL || next_token->type == T_NOT ||
               next_token->type == T_BITWISE_PIPE_PIPE) {
        return parse_equality(parser);
    } else if (parser->current->type == T_K_TRUE || parser->current->type == T_K_FALSE ||
               parser->current->type == T_INT || parser->current->type == T_STRING ||
               parser->current->type == T_FLOAT || parser->current->type == T_PARENS_OPEN ||
               parser->current->type == T_IDENTIFIER || parser->current->type == T_K_MATCHES ||
               parser->current->type == T_K_TRY) {
        return parse_primary(parser);
    } else if (parser->current->type == T_K_BOOL || parser->current->type == T_K_INT ||
               parser->current->type == T_K_STRING || parser->current->type == T_K_FLOAT ||
               parser->current->type == T_K_VOID) {
        return parse_type_name(parser);
    } else {
        return parse_equality(parser);
    }
}

static inline AST *parse_equality(Parser *parser) {
    AST *left_expr = parse_unary(parser);
    AST *ast = ast_init_with_type(AST_TYPE_BINARY);

    if (parser->current->type == T_EQUAL_EQUAL || parser->current->type == T_NOT_EQUAL ||
        parser->current->type == T_GREATER_THAN || parser->current->type == T_GREATER_THAN_EQUAL ||
        parser->current->type == T_LESS_THAN || parser->current->type == T_LESS_THAN_EQUAL ||
        parser->current->type == T_OPERATOR_ADD_EQUAL || parser->current->type == T_OPERATOR_SUB_EQUAL ||
        parser->current->type == T_OPERATOR_MUL_EQUAL || parser->current->type == T_OPERATOR_DIV_EQUAL ||
        parser->current->type == T_OPERATOR_MOD_EQUAL ||
        parser->current->type == T_BITWISE_AND_EQUAL || parser->current->type == T_BITWISE_AND_AND_EQUAL ||
        parser->current->type == T_BITWISE_PIPE_EQUAL || parser->current->type == T_BITWISE_PIPE_PIPE_EQUAL ||
        parser->current->type == T_BITWISE_SHIFT_LEFT_EQUAL || parser->current->type == T_BITWISE_SHIFT_RIGHT_EQUAL ||
        parser->current->type == T_BITWISE_POWER_EQUAL || parser->current->type == T_NOT ||
        parser->current->type == T_BITWISE_PIPE_PIPE) {
        Token *operator = parser->current;

        advance(parser, operator->type);

        AST *right_expr = parse_expression(parser);

        ast->value.Binary.left = left_expr;
        ast->value.Binary.operator = operator;
        ast->value.Binary.right = right_expr;
    } else {
        fprintf(stderr, "Parser: Expected Equality Operators\n");
        exit(-2);
    }

    return ast;
}

static inline AST *parse_term(Parser *parser) {
    AST *tree = ast_init_with_type(AST_TYPE_BINARY);
    AST *left_factor = parse_unary(parser);

    if (parser->current->type == T_OPERATOR_ADD || parser->current->type == T_OPERATOR_SUB ||
        parser->current->type == T_OPERATOR_MUL || parser->current->type == T_OPERATOR_DIV ||
        parser->current->type == T_OPERATOR_MOD || parser->current->type == T_BITWISE_PIPE ||
        parser->current->type == T_BITWISE_SHIFT_LEFT || parser->current->type == T_BITWISE_SHIFT_RIGHT ||
        parser->current->type == T_BITWISE_POWER || parser->current->type == T_BITWISE_NOT) {
        Token *operator = parser->current;
        advance(parser, operator->type);
        AST *right_factor = parse_expression(parser);

        tree->value.Binary.left = left_factor;
        tree->value.Binary.operator = operator;
        tree->value.Binary.right = right_factor;
    } else {
        fprintf(stderr, "Parser: Expected Math Operators\n");
        exit(-2);
    }

    return tree;
}

static inline AST *parse_unary(Parser *parser) {
    AST *tree = ast_init_with_type(AST_TYPE_UNARY);

    if (parser->current->type == T_OPERATOR_SUB || parser->current->type == T_NOT) {
        Token *operator = parser->current;
        advance(parser, operator->type);

        AST *right = parse_expression(parser);

        tree->value.Unary.operator = operator;
        tree->value.Unary.right = right;

        return tree;
    }

    return parse_primary(parser);
}

static inline AST *parse_primary(Parser *parser) {
    if (parser->current->type == T_K_TRUE || parser->current->type == T_K_FALSE || parser->current->type == T_INT ||
        parser->current->type == T_STRING || parser->current->type == T_FLOAT ||
        parser->current->type == T_IDENTIFIER || parser->current->type == T_K_MATCHES ||
        parser->current->type == T_ARROW || parser->current->type == T_K_TRY) {
        return parse_literal(parser);
    } else if (parser->current->type == T_PARENS_OPEN) {
        advance(parser, T_PARENS_OPEN);

        AST *expression = parse_expression(parser);
        AST *expression_tree = ast_init_with_type(AST_TYPE_GROUPING);
        advance(parser, T_PARENS_CLOSE);
        expression_tree->value.Grouping.expression = expression;

        AST *new_tree = ast_init_with_type(AST_TYPE_BINARY);
        new_tree->value.Binary.left = expression_tree;

        if (parser->current->type == T_OPERATOR_ADD || parser->current->type == T_OPERATOR_SUB ||
            parser->current->type == T_OPERATOR_MUL || parser->current->type == T_OPERATOR_DIV ||
            parser->current->type == T_OPERATOR_MOD || parser->current->type == T_BITWISE_PIPE ||
            parser->current->type == T_BITWISE_SHIFT_LEFT || parser->current->type == T_BITWISE_SHIFT_RIGHT ||
            parser->current->type == T_BITWISE_POWER || parser->current->type == T_BITWISE_NOT) {
            new_tree->value.Binary.operator = advance_without_check(parser);
            new_tree->value.Binary.right = parse_expression(parser);

            return new_tree;
        }

        return expression_tree;
    } else {
        fprintf(stderr, "Parser: Expected Literal Values or Open Braces %s\n", token_print(parser->current->type));
        exit(-2);
    }
}

static inline AST *parse_literal(Parser *parser) {
    AST *tree = ast_init_with_type(AST_TYPE_LITERAL);

    if (parser->current->type == T_K_TRUE || parser->current->type == T_K_FALSE || parser->current->type == T_INT ||
        parser->current->type == T_STRING || parser->current->type == T_FLOAT ||
        parser->current->type == T_IDENTIFIER || parser->current->type == T_K_MATCHES ||
        parser->current->type == T_ARROW || parser->current->type == T_K_TRY) {

        if (parser->current->type == T_K_TRY) {
            AST *try_statement = ast_init_with_type(AST_TYPE_TRY_STATEMENT);
            advance(parser, T_K_TRY);

            try_statement->value.TryStatement.expression = parse_expression(parser);
            advance_semi(parser);

            return try_statement;
        }

        tree->value.Literal.literal_value = parser->current;
        advance(parser, tree->value.Literal.literal_value->type);

        if (parser->current->type == T_PARENS_OPEN) {
            return parse_function_call(parser);
        } else if (parser->current->type == T_ARROW) {
            AST *binary_tree = ast_init_with_type(AST_TYPE_BINARY);
            binary_tree->value.Binary.left = tree;
            binary_tree->value.Binary.operator = parser->current;
            advance(parser, T_ARROW);
            binary_tree->value.Binary.right = parse_expression(parser);

            return binary_tree;
        }

        return tree;
    } else {
        fprintf(stderr, "Parser: Not a literal %s\n", token_print(parser->current->type));
        exit(-2);
    }
}

static inline Token *advance(Parser *parser, uintptr_t type) {
    if (parser->current->type != type) {
        fprintf(stderr, "Parser: Unexpected Token: `%s` :: `%s`, was expecting `%s`\n",
                token_print(parser->current->type), parser->current->value,
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

    return parser->previous;
}

static inline void advance_semi(Parser *parser) {
    if (parser->current->type == T_SEMICOLON) {
        advance(parser, T_SEMICOLON);
    }
}