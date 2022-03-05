//
//  parser.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "parser.h"

Parser *parser_init(Lexer *lexer) {
    Parser *parser = malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current = lexer_get_next_token(parser->lexer);
    parser->next_token = lexer_get_next_token(parser->lexer);

    return parser;
}

AST *parser_parse(Parser *parser) {
    return parser_parse_statement(parser);
}

static inline void parser_show_error(Parser *parser) {
    // Parser: Token cannot be parsed || Line: 15, Position: 4
    fprintf(stderr, " || Line: %lu, Position %lu", parser->lexer->line, parser->lexer->position);
    exit(-2);
}

AST *parser_parse_statement(Parser *parser) {
    switch (parser->current->type) {
        case T_K_IMPORT:
            return parser_parse_import_statement(parser);
        case T_K_INT:
        case T_K_FLOAT:
        case T_K_VOID:
        case T_K_STRING:
        case T_K_CHAR:
        case T_K_BOOL:
        case T_IDENTIFIER:
        case T_K_PRIVATE:
        case T_K_VOLATILE:
            return parser_parse_function_or_variable_declaration(parser, false);
        case T_K_ASM:
            return parser_parse_inline_asm(parser);
        case T_K_ENUM:
            return parser_parse_enum(parser, false);
        case T_K_STRUCT:
        case T_K_UNION:
            return parser_parse_struct(parser, false);
        case T_K_ALIAS:
            return parser_parse_alias(parser);
        default:
            fprintf(stderr, "Parser: Token `%s`, cannot be declared outside the scope",
                    token_print(parser->current->type));
            parser_show_error(parser);
    }

    exit(-2);
}

AST *parser_parse_inner_statement(Parser *parser) {
    switch (parser->current->type) {
        case T_K_INT:
        case T_K_FLOAT:
        case T_K_VOID:
        case T_K_STRING:
        case T_K_CHAR:
        case T_K_BOOL:
        case T_IDENTIFIER:
        case T_K_PRIVATE:
        case T_K_VOLATILE:
        case T_K_SELF:
            return parser_parse_function_or_variable_declaration(parser, true);
        case T_K_RETURN:
            return parser_parse_return(parser);
        case T_K_IF:
            return parser_parse_if_else_statement(parser);
        case T_K_WHILE:
            return parser_parse_while_statement(parser);
        case T_K_FOR:
            return parser_parse_for_loop(parser);
        case T_K_DO:
            return parser_parse_do_catch_statement(parser);
        case T_K_SWITCH:
            return parser_parse_switch_statement(parser);
        default:
            fprintf(stderr, "Parser: Token `%s`, cannot be declared inside the scope",
                    token_print(parser->current->type));
            parser_show_error(parser);
    }

    exit(-2);
}

AST *parser_parse_struct_statement(Parser *parser) {
    switch (parser->current->type) {
        case T_K_INT:
        case T_K_FLOAT:
        case T_K_VOID:
        case T_K_STRING:
        case T_K_CHAR:
        case T_K_BOOL:
        case T_IDENTIFIER:
        case T_K_PRIVATE:
        case T_K_VOLATILE:
            return parser_parse_struct_members(parser);
        case T_AT:
            return parser_parse_struct_initializer(parser);
        default:
            fprintf(stderr, "Parser: Token `%s`, cannot be declared inside the struct or union",
                    token_print(parser->current->type));
            parser_show_error(parser);
    }

    exit(-2);
}

AST *parser_parse_struct_members(Parser *parser) {
    bool is_private = false;
    bool is_volatile = false;

    while (parser->current->type == T_K_PRIVATE || parser->current->type == T_K_VOLATILE) {
        if ((is_private && parser->current->type == T_K_PRIVATE) ||
            (is_volatile && parser->current->type == T_K_VOLATILE)) {
            fprintf(stderr, "Parser: Token `%s`, has been declared more than once", token_print(parser->current->type));
            parser_show_error(parser);
        }

        if (parser->current->type == T_K_PRIVATE) {
            parser_advance(parser, T_K_PRIVATE);
            is_private = true;
        }

        if (parser->current->type == T_K_VOLATILE) {
            parser_advance(parser, T_K_VOLATILE);
            is_volatile = true;
        }
    }

    AST *type = parser_parse_type(parser);


    if (parser->current->type == T_DOUBLE_COLON) {
        parser_advance(parser, T_DOUBLE_COLON);
        if (is_volatile) {
            fprintf(stderr, "Parser: Function cannot be declared as volatile");
            parser_show_error(parser);
        }
        return parser_parse_function_declaration(parser, type, is_private);
    } else if (parser->current->type == T_IDENTIFIER) {
        return parser_parse_variable_declaration(parser, type, is_private, is_volatile);
    } else {
        fprintf(stderr, "Parser: Token `%s`, is not parseable", token_print(parser->current->type));
        parser_show_error(parser);
    }

    return type;
}

AST *parser_parse_struct_initializer(Parser *parser) {
    AST *struct_initializer_ast = ast_init_with_type(AST_TYPE_STRUCT_INITIALIZER);

    parser_advance(parser, T_AT);
    char *function_name = parser->current->value;
    parser_advance(parser, T_IDENTIFIER);
    if (strcmp(function_name, "init") != 0) {
        free(function_name);
        fprintf(stderr, "Parser: `@` sign can only be used for structure initialization");
        parser_show_error(parser);
        return NULL; // Just to remove CLion warnings
    }
    free(function_name);

    parser_advance(parser, T_PARENS_OPEN);

    // Allocate the Parameters
    struct_initializer_ast->value.StructInitializer.arguments = mxDynamicArrayCreate(sizeof(AST *));
    struct_initializer_ast->value.StructInitializer.body = mxDynamicArrayCreate(sizeof(AST *));

    while (parser->current->type != T_PARENS_CLOSE) {
        AST *parameter_ast = ast_init_with_type(AST_TYPE_FUNCTION_ARGUMENT);
        parameter_ast->value.FunctionDeclarationArgument.argument_type = parser_parse_type(parser);

        parameter_ast->value.FunctionDeclarationArgument.argument_name = parser->current->value;
        parser_advance(parser, T_IDENTIFIER);

        // Insert the item
        mxDynamicArrayAdd(struct_initializer_ast->value.StructInitializer.arguments, parameter_ast);

        // Advance to the next argument
        if (parser->current->type == T_COMMA)
            parser_advance(parser, T_COMMA);
    }

    parser_advance(parser, T_PARENS_CLOSE);

    parser_advance(parser, T_BRACE_OPEN);

    while (parser->current->type != T_BRACE_CLOSE) {
        mxDynamicArrayAdd(struct_initializer_ast->value.StructInitializer.body, parser_parse_inner_statement(parser));
    }

    parser_advance(parser, T_BRACE_CLOSE);

    return struct_initializer_ast;
}

AST *parser_parse_struct(Parser *parser, __attribute__((unused)) bool is_private) {
    AST *struct_ast = ast_init_with_type(AST_TYPE_STRUCT_OR_UNION_DECLARATION);

    if (parser->current->type == T_K_UNION) {
        struct_ast->value.StructOrUnionDeclaration.is_union = true;
        parser_advance(parser, T_K_UNION);
    } else {
        // Defined false by default
        parser_advance(parser, T_K_STRUCT);
    }


    char *name = parser->current->value;
    parser_advance(parser, T_IDENTIFIER);

    struct_ast->value.StructOrUnionDeclaration.name = name;

    parser_advance(parser, T_BRACE_OPEN);


    struct_ast->value.StructOrUnionDeclaration.members = mxDynamicArrayCreate(sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        mxDynamicArrayAdd(struct_ast->value.StructOrUnionDeclaration.members, parser_parse_struct_statement(parser));
    }

    parser_advance(parser, T_BRACE_CLOSE);

    return struct_ast;
}

AST *parser_parse_function_call(Parser *parser, char *function_name) {
    AST *function_call_ast = ast_init_with_type(AST_TYPE_FUNCTION_CALL);
    function_call_ast->value.FunctionCall.function_call_name = function_name;

    // Parse the arguments
    parser_advance(parser, T_PARENS_OPEN);

    function_call_ast->value.FunctionCall.arguments = mxDynamicArrayCreate(sizeof(AST *));

    while (parser->current->type != T_PARENS_CLOSE) {
        mxDynamicArrayAdd(function_call_ast->value.FunctionCall.arguments, parser_parse_expression(parser));

        if (parser->current->type != T_PARENS_CLOSE)
            parser_advance(parser, T_COMMA);
    }

    parser_advance(parser, T_PARENS_CLOSE);

    return function_call_ast;
}

AST *parser_parse_function_or_variable_declaration(Parser *parser, bool is_inner_statement) {
    bool is_private = false;
    bool is_volatile = false;

    while (parser->current->type == T_K_PRIVATE || parser->current->type == T_K_VOLATILE) {
        if ((is_private && parser->current->type == T_K_PRIVATE) ||
            (is_volatile && parser->current->type == T_K_VOLATILE)) {
            fprintf(stderr, "Parser: Token `%s`, has been declared more than once", token_print(parser->current->type));
            parser_show_error(parser);
        }
        if (parser->current->type == T_K_PRIVATE) {
            parser_advance(parser, T_K_PRIVATE);
            is_private = true;
        }

        if (parser->current->type == T_K_VOLATILE) {
            parser_advance(parser, T_K_VOLATILE);
            is_volatile = true;
        }
    }

    if (parser_is_binary_operator(parser->next_token) && is_inner_statement) {
        return parser_parse_expression(parser);
    } else if (parser->current->type == T_K_ENUM && !is_inner_statement) {
        return parser_parse_enum(parser, is_private);
    } else if (parser->current->type == T_K_STRUCT || parser->current->type == T_K_UNION) {
        return parser_parse_struct(parser, is_private);
    }

    AST *type = parser_parse_type(parser);


    if (parser->current->type == T_PARENS_OPEN) {
        return parser_parse_function_call(parser, type->value.Literal.literal_value->value);
    } else if (parser->current->type == T_DOUBLE_COLON && !is_inner_statement) {
        parser_advance(parser, T_DOUBLE_COLON);
        if (is_volatile) {
            fprintf(stderr, "Parser: Function cannot be declared as volatile");
            parser_show_error(parser);
        }
        return parser_parse_function_declaration(parser, type, is_private);
    } else if (parser->current->type == T_IDENTIFIER && is_inner_statement) {
        return parser_parse_variable_declaration(parser, type, is_private, is_volatile);
    } else {
        free(type);
        fprintf(stderr, "Parser: Token `%s`, is cannot be parsed", token_print(parser->current->type));
        parser_show_error(parser);
    }

    return NULL;
}

AST *parser_parse_variable_declaration(Parser *parser, AST *variable_type, bool is_private, bool is_volatile) {
    AST *variable_ast = ast_init_with_type(AST_TYPE_VARIABLE_DEFINITION);
    variable_ast->value.VariableDeclaration.type = variable_type;
    variable_ast->value.VariableDeclaration.identifier = parser->current->value;
    parser_advance(parser, T_IDENTIFIER);
    if (parser->current->type == T_QUESTION) {
        variable_ast->value.VariableDeclaration.type->value.ValueKeyword.is_optional = true;
        parser_advance(parser, T_QUESTION);
    }

    variable_ast->value.VariableDeclaration.is_private = is_private;
    variable_ast->value.VariableDeclaration.is_volatile = is_volatile;

    if (parser->current->type == T_EQUAL) {
        parser_advance(parser, T_EQUAL);
        variable_ast->value.VariableDeclaration.is_initialized = true;
        variable_ast->value.VariableDeclaration.value = parser_parse_expression(parser);
    }

    return variable_ast;
}

AST *parser_parse_function_declaration(Parser *parser, AST *return_type, bool is_private) {
    AST *function_ast = ast_init_with_type(AST_TYPE_FUNCTION_DECLARATION);

    function_ast->value.FunctionDeclaration.return_type = return_type;
    function_ast->value.FunctionDeclaration.function_name = parser->current->value;
    parser_advance(parser, T_IDENTIFIER);
    function_ast->value.FunctionDeclaration.is_private = is_private;

    // Parse the parameters
    parser_advance(parser, T_PARENS_OPEN);

    // Allocate the Parameters
    function_ast->value.FunctionDeclaration.arguments = mxDynamicArrayCreate(sizeof(AST *));
    function_ast->value.FunctionDeclaration.body = mxDynamicArrayCreate(sizeof(AST *));

    while (parser->current->type != T_PARENS_CLOSE) {
        AST *parameter_ast = ast_init_with_type(AST_TYPE_FUNCTION_ARGUMENT);
        parameter_ast->value.FunctionDeclarationArgument.argument_type = parser_parse_type(parser);
        parameter_ast->value.FunctionDeclarationArgument.argument_name = parser->current->value;
        parser_advance(parser, T_IDENTIFIER);

        // Insert the item
        mxDynamicArrayAdd(function_ast->value.FunctionDeclaration.arguments, parameter_ast);

        // Advance to the next argument
        if (parser->current->type == T_COMMA)
            parser_advance(parser, T_COMMA);
    }

    parser_advance(parser, T_PARENS_CLOSE);

    parser_advance(parser, T_BRACE_OPEN);

    while (parser->current->type != T_BRACE_CLOSE) {
        mxDynamicArrayAdd(function_ast->value.FunctionDeclaration.body, parser_parse_inner_statement(parser));
    }

    parser_advance(parser, T_BRACE_CLOSE);

    return function_ast;
}

AST *parser_parse_inline_asm(Parser *parser) {
    AST *asm_tree = ast_init_with_type(AST_TYPE_INLINE_ASSEMBLY);

    parser_advance(parser, T_K_ASM);
    parser_advance(parser, T_BRACE_OPEN);

    asm_tree->value.InlineAssembly.instructions = mxDynamicArrayCreate(sizeof(char *));

    while (parser->current->type != T_BRACE_CLOSE) {
        // Update the size of the array
        mxDynamicArrayAdd(asm_tree->value.InlineAssembly.instructions, parser->current->value);

        parser_advance(parser, T_STRING);
    }

    parser_advance(parser, T_BRACE_CLOSE);

    return asm_tree;
}

AST *parser_parse_import_statement(Parser *parser) {
    AST *import_tree = ast_init_with_type(AST_TYPE_IMPORT);

    parser_advance(parser, T_K_IMPORT);
    char *identifier = parser->current->value;
    parser_advance(parser, T_IDENTIFIER);

    if (parser->current->type == T_PERIOD) {
        parser_advance(parser, T_PERIOD);

        import_tree->value.Import.file = identifier;
        import_tree->value.Import.is_library = false;

        return import_tree;
    }

    import_tree->value.Import.file = identifier;
    import_tree->value.Import.is_library = true;

    return import_tree;
}

AST *parser_parse_enum(Parser *parser, bool is_private) {
    AST *enum_ast = ast_init_with_type(AST_TYPE_ENUM_DECLARATION);
    parser_advance(parser, T_K_ENUM);
    char *enum_name = parser->current->value;
    parser_advance(parser, T_IDENTIFIER);

    enum_ast->value.EnumDeclaration.enum_name = enum_name;

    parser_advance(parser, T_BRACE_OPEN);

    enum_ast->value.EnumDeclaration.cases = mxDynamicArrayCreate(sizeof(AST *));

    int case_counter = 0;
    while (parser->current->type != T_BRACE_CLOSE) {
        AST *enum_case = ast_init_with_type(AST_TYPE_ENUM_ITEM);
        parser_advance(parser, T_K_CASE);
        enum_case->value.EnumItem.case_name = parser->current->value;
        parser_advance(parser, T_IDENTIFIER);

        if (parser->current->type == T_EQUAL) {
            parser_advance(parser, T_EQUAL);
            char *case_counter_string = parser->current->value;
            parser_advance(parser, T_INT);
            case_counter = (int) strtol(case_counter_string, &case_counter_string, 10);
        }

        enum_case->value.EnumItem.case_value = case_counter;
        parser_advance(parser, T_COMMA);

        mxDynamicArrayAdd(enum_ast->value.EnumDeclaration.cases, enum_case);

        case_counter++;
    }
    parser_advance(parser, T_BRACE_CLOSE);

    if (is_private)
        enum_ast->value.EnumDeclaration.is_private = true;

    return enum_ast;
}

AST *parser_parse_return(Parser *parser) {
    AST *new_ast = ast_init_with_type(AST_TYPE_RETURN);

    parser_advance(parser, T_K_RETURN);
    new_ast->value.Return.return_expression = parser_parse_expression(parser);

    return new_ast;
}

AST *parser_parse_if_else_statement(Parser *parser) {
    AST *if_else_statement_ast = ast_init_with_type(AST_TYPE_IF_ELSE_STATEMENT);
    parser_advance(parser, T_K_IF);

    // The right
    parser_advance(parser, T_PARENS_OPEN);
    if_else_statement_ast->value.IfElseStatement.if_condition = parser_parse_expression(parser);
    parser_advance(parser, T_PARENS_CLOSE);

    // The body
    if_else_statement_ast->value.IfElseStatement.if_body = parser_parse_body(parser);

    // The else body


    if (parser->current->type == T_K_ELSE) {
        if (parser->current->type == T_K_ELSE && parser->next_token->type ==
                                                 T_K_IF) {
            while (parser->current->type == T_K_ELSE && parser->next_token->type ==
                                                        T_K_IF) {

                // Allocate
                if_else_statement_ast->value.IfElseStatement.else_if_bodies = mxDynamicArrayCreate(sizeof(AST *));
                if_else_statement_ast->value.IfElseStatement.else_body = mxDynamicArrayCreate(sizeof(AST *));
                if_else_statement_ast->value.IfElseStatement.else_if_conditions = mxDynamicArrayCreate(sizeof(AST *));

                parser_advance(parser, T_K_ELSE);
                parser_advance(parser, T_K_IF);
                // Open bracket
                parser_advance(parser, T_PARENS_OPEN);
                mxDynamicArrayAdd(if_else_statement_ast->value.IfElseStatement.else_if_conditions,
                                  parser_parse_expression(parser));
                parser_advance(parser, T_PARENS_CLOSE);

                mxDynamicArrayAdd(if_else_statement_ast->value.IfElseStatement.else_if_bodies,
                                  parser_parse_body(parser));
            }
        }
    }

    if (parser->current->type == T_K_ELSE) {
        if_else_statement_ast->value.IfElseStatement.has_else_statement = true;
        parser_advance(parser, T_K_ELSE);

        mxDynamicArrayAdd(if_else_statement_ast->value.IfElseStatement.else_body, parser_parse_body(parser));
    }

    return if_else_statement_ast;
}

AST *parser_parse_while_statement(Parser *parser) {
    AST *while_ast = ast_init_with_type(AST_TYPE_WHILE_STATEMENT);

    parser_advance(parser, T_K_WHILE);

    // The right
    parser_advance(parser, T_PARENS_OPEN);
    while_ast->value.WhileStatement.expression = parser_parse_expression(parser);
    parser_advance(parser, T_PARENS_CLOSE);

    // The body
    while_ast->value.WhileStatement.body = parser_parse_body(parser);

    return while_ast;
}

AST *parser_parse_for_loop(Parser *parser) {
    AST *for_ast = ast_init_with_type(AST_TYPE_FOR_LOOP);

    // Parse the arguments
    parser_advance(parser, T_K_FOR);
    parser_advance(parser, T_PARENS_OPEN);
    for_ast->value.ForLoop.variable = parser_parse_variable_declaration(parser, parser_parse_type(parser), false,
                                                                        false);
    parser_advance(parser, T_COMMA);
    for_ast->value.ForLoop.condition = parser_parse_expression(parser);
    parser_advance(parser, T_COMMA);
    for_ast->value.ForLoop.condition2 = parser_parse_expression(parser);
    parser_advance(parser, T_PARENS_CLOSE);

    parser_advance(parser, T_BRACE_OPEN);

    for_ast->value.ForLoop.body = mxDynamicArrayCreate(sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        if (parser->current->type == T_K_CONTINUE || parser->current->type == T_K_BREAK) {
            AST *literal_value = ast_init_with_type(AST_TYPE_LITERAL);
            literal_value->value.Literal.literal_value = parser->current;
            mxDynamicArrayAdd(for_ast->value.ForLoop.body, literal_value);
            parser_advance_without_check(parser);
        } else {
            mxDynamicArrayAdd(for_ast->value.ForLoop.body, parser_parse_inner_statement(parser));
        }
    }

    parser_advance(parser, T_BRACE_CLOSE);

    return for_ast;
}

AST *parser_parse_do_catch_statement(Parser *parser) {
    AST *do_catch_ast = ast_init_with_type(AST_TYPE_DO_CATCH);

    /* Do Statements */
    parser_advance(parser, T_K_DO);
    parser_advance(parser, T_BRACE_OPEN);
    // Parse Body
    do_catch_ast->value.DoCatchOrWhileStatement.do_body = mxDynamicArrayCreate(sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        mxDynamicArrayAdd(do_catch_ast->value.DoCatchOrWhileStatement.do_body, parser_parse_inner_statement(parser));
    }

    parser_advance(parser, T_BRACE_CLOSE);

    /* Catch Or While Statements */
    if (parser->current->type == T_K_WHILE) {
        parser_advance(parser, T_K_WHILE);

        parser_advance(parser, T_PARENS_OPEN);
        do_catch_ast->value.DoCatchOrWhileStatement.is_while_statement = true;
        do_catch_ast->value.DoCatchOrWhileStatement.while_statement_expression = parser_parse_expression(parser);
        parser_advance(parser, T_PARENS_CLOSE);
    } else if (parser->current->type == T_K_CATCH) {
        parser_advance(parser, T_K_CATCH);

        parser_advance(parser, T_BRACE_OPEN);

        do_catch_ast->value.DoCatchOrWhileStatement.second_body = mxDynamicArrayCreate(sizeof(AST *));

        while (parser->current->type != T_BRACE_CLOSE) {
            mxDynamicArrayAdd(do_catch_ast->value.DoCatchOrWhileStatement.second_body,
                              parser_parse_inner_statement(parser));
        }

        parser_advance(parser, T_BRACE_CLOSE);
    } else {
        fprintf(stderr, "Parser: Expected `catch` or `while` after `do` statement");
        parser_show_error(parser);
    }

    return do_catch_ast;
}

AST *parser_parse_switch_statement(Parser *parser) {
    AST *switch_tree = ast_init_with_type(AST_TYPE_SWITCH_STATEMENT);

    parser_advance(parser, T_K_SWITCH);
    parser_advance(parser, T_PARENS_OPEN);
    switch_tree->value.SwitchStatement.expression = parser_parse_expression(parser);
    parser_advance(parser, T_PARENS_CLOSE);

    parser_advance(parser, T_BRACE_OPEN);

    switch_tree->value.SwitchStatement.switch_cases = mxDynamicArrayCreate(sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        bool is_default = false;
        if (parser->current->type == T_K_CASE)
            parser_advance(parser, T_K_CASE);
        else {
            is_default = true;
            parser_advance(parser, T_K_DEFAULT);
        }

        AST *switch_case = ast_init_with_type(AST_TYPE_SWITCH_CASE);
        if (!is_default)
            switch_case->value.SwitchCase.expression = parser_parse_expression(parser);
        else
            switch_case->value.SwitchCase.is_default = true;

        parser_advance(parser, T_COLON);

        switch_case->value.SwitchCase.body = mxDynamicArrayCreate(sizeof(AST *));

        while (parser->current->type != T_BRACE_CLOSE) {
            if (parser->current->type == T_K_BREAK) {
                AST *literal_value = ast_init_with_type(AST_TYPE_LITERAL);
                literal_value->value.Literal.literal_value = parser->current;
                mxDynamicArrayAdd(switch_case->value.SwitchCase.body, literal_value);
                parser_advance(parser, T_K_BREAK);
            } else {
                mxDynamicArrayAdd(switch_case->value.SwitchCase.body, parser_parse_inner_statement(parser));
            }

            if (parser->current->type == T_BRACE_CLOSE || parser->current->type == T_K_CASE ||
                parser->current->type == T_K_DEFAULT) {
                break;
            }
        }
        mxDynamicArrayAdd(switch_tree->value.SwitchStatement.switch_cases, switch_case);
    }
    parser_advance(parser, T_BRACE_CLOSE);

    return switch_tree;
}

AST *parser_parse_matches_statement(Parser *parser) {
    AST *matches_tree = ast_init_with_type(AST_TYPE_SWITCH_STATEMENT);
    matches_tree->value.SwitchStatement.is_matches_statement = true;
    parser_advance(parser, T_K_MATCHES);

    parser_advance(parser, T_PARENS_OPEN);
    matches_tree->value.SwitchStatement.expression = parser_parse_expression(parser);
    parser_advance(parser, T_PARENS_CLOSE);

    matches_tree->value.SwitchStatement.switch_cases = mxDynamicArrayCreate(sizeof(AST *));

    parser_advance(parser, T_BRACE_OPEN);
    while (parser->current->type != T_BRACE_CLOSE) {
        AST *switch_case = ast_init_with_type(AST_TYPE_SWITCH_CASE);

        if (parser->current->type == T_IDENTIFIER && strlen(parser->current->value) == 1 &&
            parser->current->value[0] == '_') {
            // Default Clause
            switch_case->value.SwitchCase.is_default = true;
        }

        switch_case->value.SwitchCase.expression = parser_parse_expression(parser);

        parser_advance(parser, T_COLON);

        switch_case->value.SwitchCase.body = mxDynamicArrayCreate(sizeof(AST *));

        if (parser->current->type == T_BRACE_OPEN) {
            parser_advance(parser, T_BRACE_OPEN);

            while (parser->current->type != T_BRACE_CLOSE) {
                mxDynamicArrayAdd(switch_case->value.SwitchCase.body, parser_parse_inner_statement(parser));
                if (parser->current->type == T_BRACE_CLOSE)
                    break;
            }

            parser_advance(parser, T_BRACE_CLOSE);
        } else {
            mxDynamicArrayAdd(switch_case->value.SwitchCase.body, parser_parse_expression(parser));
        }

        mxDynamicArrayAdd(matches_tree->value.SwitchStatement.switch_cases, switch_case);

        if (parser->current->type == T_BRACE_CLOSE) {
            break;
        }
    }

    parser_advance(parser, T_BRACE_CLOSE);

    return matches_tree;
}

AST *parser_parse_expression_try(Parser *parser) {
    AST *try_statement = ast_init_with_type(AST_TYPE_TRY_STATEMENT);
    parser_advance(parser, T_K_TRY);

    try_statement->value.TryStatement.expression = parser_parse_expression(parser);

    return try_statement;
}

AST *parser_parse_alias(Parser *parser) {
    AST *alias_ast = ast_init_with_type(AST_TYPE_ALIAS);
    parser_advance(parser, T_K_ALIAS);
    alias_ast->value.Alias.alias_name = parser->current->value;
    parser_advance(parser, T_IDENTIFIER);
    parser_advance(parser, T_COLON);
    alias_ast->value.Alias.alias_value = parser_parse_expression(parser);

    return alias_ast;
}

AST *parser_parse_body(Parser *parser) {
    AST *body_ast = ast_init_with_type(AST_TYPE_BODY);

    parser_advance(parser, T_BRACE_OPEN);

    // Allocate the body
    body_ast->value.Body.body_size = 0;
    body_ast->value.Body.body = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        // Re-size the array
        body_ast->value.Body.body_size += 1;
        body_ast->value.Body.body = realloc(body_ast->value.Body.body, body_ast->value.Body.body_size * sizeof(AST *));

        // Insert the item
        body_ast->value.Body.body[body_ast->value.Body.body_size - 1] = parser_parse_inner_statement(parser);
    }

    parser_advance(parser, T_BRACE_CLOSE);

    return body_ast;
}

AST *parser_parse_expression(Parser *parser) {
    return parser_parse_equality(parser);
}

AST *parser_parse_equality(Parser *parser) {
    AST *equality_ast = parser_parse_comparison(parser);

    while (parser->current->type == T_EQUAL_EQUAL || parser->current->type == T_EQUAL ||
           parser->current->type == T_NOT_EQUAL) {
        AST *equality_operator = ast_init_with_type(AST_TYPE_BINARY);
        equality_operator->value.Binary.operator = parser->current;
        equality_operator->value.Binary.left = equality_ast;
        parser_advance(parser, parser->current->type);
        equality_operator->value.Binary.right = parser_parse_comparison(parser);
        equality_ast = equality_operator;
    }

    return equality_ast;
}

AST *parser_parse_comparison(Parser *parser) {
    AST *term_ast = parser_parse_term(parser);

    while (parser->current->type == T_LESS_THAN || parser->current->type == T_LESS_THAN_EQUAL ||
           parser->current->type == T_GREATER_THAN || parser->current->type == T_GREATER_THAN_EQUAL) {
        AST *comparison_operator = ast_init_with_type(AST_TYPE_BINARY);
        comparison_operator->value.Binary.operator = parser->current;
        comparison_operator->value.Binary.left = term_ast;
        parser_advance(parser, parser->current->type);
        comparison_operator->value.Binary.right = parser_parse_term(parser);
        term_ast = comparison_operator;
    }

    return term_ast;
}

AST *parser_parse_term(Parser *parser) {
    AST *term_ast = parser_parse_factor(parser);

    while (parser->current->type == T_OPERATOR_ADD || parser->current->type == T_OPERATOR_SUB ||
           parser->current->type == T_PERIOD) {
        AST *term_operator = ast_init_with_type(AST_TYPE_BINARY);
        term_operator->value.Binary.operator = parser->current;
        term_operator->value.Binary.left = term_ast;
        parser_advance(parser, parser->current->type);
        term_operator->value.Binary.right = parser_parse_factor(parser);
        term_ast = term_operator;
    }

    return term_ast;
}

AST *parser_parse_factor(Parser *parser) {
    AST *unary_ast = parser_parse_unary(parser);

    while (parser->current->type == T_OPERATOR_MUL || parser->current->type == T_OPERATOR_DIV ||
           parser->current->type == T_OPERATOR_MOD) {
        AST *unary_operator = ast_init_with_type(AST_TYPE_BINARY);
        unary_operator->value.Binary.operator = parser->current;
        unary_operator->value.Binary.left = unary_ast;
        parser_advance(parser, parser->current->type);
        unary_operator->value.Binary.right = parser_parse_unary(parser);
        unary_ast = unary_operator;
    }

    return unary_ast;
}

AST *parser_parse_unary(Parser *parser) {
    if (parser->current->type == T_NOT) {
        AST *unary_ast = ast_init_with_type(AST_TYPE_UNARY);
        unary_ast->value.Unary.operator = parser->current;
        parser_advance(parser, parser->current->type);
        unary_ast->value.Unary.right = parser_parse_unary(parser);
        return unary_ast;
    }

    return parser_parse_primary(parser);
}

AST *parser_parse_primary(Parser *parser) {
    if (parser->current->type == T_K_MATCHES) {
        return parser_parse_matches_statement(parser);
    }
    if (parser->current->type == T_IDENTIFIER || parser->current->type == T_STRING || parser->current->type == T_INT ||
        parser->current->type == T_FLOAT || parser->current->type == T_HEX || parser->current->type == T_OCTAL ||
        parser->current->type == T_CHAR || parser->current->type == T_K_SELF) {
        AST *literal_ast = ast_init_with_type(AST_TYPE_LITERAL);
        literal_ast->value.Literal.literal_value = parser->current;
        parser_advance(parser, parser->current->type);

        if (parser->current->type == T_PARENS_OPEN) {
            return parser_parse_function_call(parser, literal_ast->value.Literal.literal_value->value);
        }

        return literal_ast;
    }

    if (parser->current->type == T_BRACE_OPEN) {
        return parser_parse_body(parser);
    }

    if (parser->current->type == T_PARENS_OPEN) {
        parser_advance(parser, T_PARENS_OPEN);
        AST *inside_expression = ast_init_with_type(AST_TYPE_GROUPING);
        inside_expression->value.Grouping.expression = parser_parse_expression(parser);
        parser_advance(parser, T_PARENS_CLOSE);

        return inside_expression;
    }

    // Show error
    fprintf(stderr, "Error: Cannot Parse Literal `%s`", parser->current->value);
    parser_show_error(parser);
    return NULL;
}

bool parser_is_binary_operator(Token *token) {
    return (token->type == T_EQUAL_EQUAL || token->type == T_NOT_EQUAL ||
            token->type == T_GREATER_THAN || token->type == T_GREATER_THAN_EQUAL ||
            token->type == T_LESS_THAN || token->type == T_LESS_THAN_EQUAL ||
            token->type == T_OPERATOR_ADD_EQUAL || token->type == T_OPERATOR_SUB_EQUAL ||
            token->type == T_OPERATOR_MUL_EQUAL || token->type == T_OPERATOR_DIV_EQUAL ||
            token->type == T_OPERATOR_MOD_EQUAL ||
            token->type == T_BITWISE_AND_EQUAL || token->type == T_BITWISE_AND_AND_EQUAL ||
            token->type == T_BITWISE_PIPE_EQUAL || token->type == T_BITWISE_PIPE_PIPE_EQUAL ||
            token->type == T_BITWISE_SHIFT_LEFT_EQUAL ||
            token->type == T_BITWISE_SHIFT_RIGHT_EQUAL ||
            token->type == T_BITWISE_POWER_EQUAL || token->type == T_NOT ||
            token->type == T_BITWISE_PIPE_PIPE || token->type == T_OPERATOR_ADD ||
            token->type == T_OPERATOR_SUB ||
            token->type == T_OPERATOR_MUL || token->type == T_OPERATOR_DIV ||
            token->type == T_OPERATOR_MOD || token->type == T_BITWISE_PIPE ||
            token->type == T_BITWISE_SHIFT_LEFT || token->type == T_BITWISE_SHIFT_RIGHT ||
            token->type == T_BITWISE_POWER || token->type == T_BITWISE_NOT ||
            token->type == T_PERIOD || token->type == T_K_MATCHES || token->type == T_EQUAL);
}

AST *parser_parse_type(Parser *parser) {
    AST *type = ast_init_with_type(AST_TYPE_VALUE_KEYWORD);

    if (parser->current->type == T_K_INT || parser->current->type == T_K_FLOAT || parser->current->type == T_K_VOID ||
        parser->current->type == T_K_STRING || parser->current->type == T_K_CHAR || parser->current->type == T_K_BOOL ||
        parser->current->type == T_IDENTIFIER) {
        type->value.ValueKeyword.token = parser->current;

        parser_advance_without_check(parser);
    } else {
        fprintf(stderr, "Parser: Invalid Type");
        parser_show_error(parser);
    }

    if (parser->current->type == T_SQUARE_OPEN) {
        parser_advance(parser, T_SQUARE_OPEN);
        parser_advance(parser, T_SQUARE_CLOSE);
        type->value.ValueKeyword.is_array = true;
    }

    if (parser->current->type == T_OPERATOR_MUL) {
        parser_advance(parser, T_OPERATOR_MUL);
        type->value.ValueKeyword.is_pointer = true;
    }

    if (parser->current->type == T_SQUARE_OPEN || parser->current->type == T_OPERATOR_MUL) {
        fprintf(stderr, "Parser: Type is declared incorrectly, pattern is `int<array><pointer>`");
        parser_show_error(parser);
    }

    return type;
}

void parser_advance(Parser *parser, int token_type) {
    if (parser->current->type != token_type) {
        fprintf(stderr, "Parser: Expected `%s`, instead got `%s`", token_print(token_type),
                token_print(parser->current->type));
        parser_show_error(parser);
    }

    parser->current = parser->next_token;
    parser->next_token = lexer_get_next_token(parser->lexer);
}

void parser_advance_without_check(Parser *parser) {
    parser->current = parser->next_token;
    parser->next_token = lexer_get_next_token(parser->lexer);
}
