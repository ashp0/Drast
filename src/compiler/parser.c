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
    parser->previous = parser->current;

    return parser;
}

AST *parser_parse(Parser *parser) {
    return parser_parse_statement(parser);
}

void parser_show_error(Parser *parser) {
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
        case T_K_ELSE:
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
}

AST *parser_parse_function_call(Parser *parser, char *function_name) {
    AST *function_call_ast = ast_init_with_type(AST_TYPE_FUNCTION_CALL);
    function_call_ast->value.FunctionCall.function_call_name = function_name;

    // Parse the arguments
    parser_advance(parser, T_PARENS_OPEN);

    function_call_ast->value.FunctionCall.arguments_size = 0;
    function_call_ast->value.FunctionCall.arguments = malloc(sizeof(AST *));

    while (parser->current->type != T_PARENS_CLOSE) {
        function_call_ast->value.FunctionCall.arguments_size += 1;
        function_call_ast->value.FunctionCall.arguments = realloc(function_call_ast->value.FunctionCall.arguments,
                                                                  function_call_ast->value.FunctionCall.arguments_size *
                                                                  sizeof(AST *));

        function_call_ast->value.FunctionCall.arguments[function_call_ast->value.FunctionCall.arguments_size -
                                                        1] = parser_parse_expression(parser);

        if (parser->current->type != T_PARENS_CLOSE)
            parser_advance(parser, T_COMMA);
    }

    parser_advance(parser, T_PARENS_CLOSE);

    return function_call_ast;
}

AST *parser_parse_struct_members(Parser *parser) {
    bool is_private = false;
    bool is_volatile = false;

    while (parser->current->type == T_K_PRIVATE || parser->current->type == T_K_VOLATILE) {
        if (is_private && parser->current->type == T_K_PRIVATE ||
            is_volatile && parser->current->type == T_K_VOLATILE) {
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
    AST *struct_initializer_ast = ast_init_with_type(AST_TYPE_FUNCTION_DECLARATION);
    struct_initializer_ast->value.FunctionDeclaration.is_struct_initializer = true;

    parser_advance(parser, T_AT);
    if (strcmp(parser_advance(parser, T_IDENTIFIER)->value, "init") != 0) {
        fprintf(stderr, "Parser: `@` sign can only be used for structure initialization");
        parser_show_error(parser);
    }

    parser_advance(parser, T_PARENS_OPEN);

    // Allocate the Parameters
    struct_initializer_ast->value.FunctionDeclaration.argument_size = 0;
    struct_initializer_ast->value.FunctionDeclaration.arguments = calloc(1, sizeof(AST *));

    while (parser->current->type != T_PARENS_CLOSE) {
        // Re-size the array
        struct_initializer_ast->value.FunctionDeclaration.argument_size += 1;
        struct_initializer_ast->value.FunctionDeclaration.arguments = realloc(
                struct_initializer_ast->value.FunctionDeclaration.arguments,
                struct_initializer_ast->value.FunctionDeclaration.argument_size *
                sizeof(AST *));

        // Create item
        AST *parameter_ast = ast_init_with_type(AST_TYPE_FUNCTION_ARGUMENT);
        parameter_ast->value.FunctionDeclarationArgument.argument_type = parser_parse_type(parser);
        parameter_ast->value.FunctionDeclarationArgument.argument_name = parser_advance(parser, T_IDENTIFIER)->value;

        // Insert the item
        struct_initializer_ast->value.FunctionDeclaration.arguments[
                struct_initializer_ast->value.FunctionDeclaration.argument_size -
                1] = parameter_ast;

        // Advance to the next argument
        if (parser->current->type == T_COMMA)
            parser_advance(parser, T_COMMA);
    }

    parser_advance(parser, T_PARENS_CLOSE);

    struct_initializer_ast->value.FunctionDeclaration.body = parser_parse_body(parser);

    return struct_initializer_ast;
}

AST *parser_parse_struct(Parser *parser, bool is_private) {
    AST *struct_ast = ast_init_with_type(AST_TYPE_STRUCT_OR_UNION_DECLARATION);

    if (parser->current->type == T_K_UNION) {
        struct_ast->value.StructOrUnionDeclaration.is_union = true;
        parser_advance(parser, T_K_UNION);
    } else {
        // Defined false by default
        // struct_ast->value.StructOrUnionDeclaration.is_union = false;
        parser_advance(parser, T_K_STRUCT);
    }

    char *name = parser_advance(parser, T_IDENTIFIER)->value;

    struct_ast->value.StructOrUnionDeclaration.name = name;

    parser_advance(parser, T_BRACE_OPEN);


    struct_ast->value.StructOrUnionDeclaration.member_size = 0;
    struct_ast->value.StructOrUnionDeclaration.members = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        struct_ast->value.StructOrUnionDeclaration.member_size += 1;

        struct_ast->value.StructOrUnionDeclaration.members = realloc(struct_ast->value.StructOrUnionDeclaration.members,
                                                                     struct_ast->value.StructOrUnionDeclaration.member_size *
                                                                     sizeof(AST *));

        struct_ast->value.StructOrUnionDeclaration.members[struct_ast->value.StructOrUnionDeclaration.member_size -
                                                           1] = parser_parse_struct_statement(parser);
    }

    parser_advance(parser, T_BRACE_CLOSE);

    return struct_ast;
}

AST *parser_parse_function_or_variable_declaration(Parser *parser, bool is_inner_statement) {
    bool is_private = false;
    bool is_volatile = false;

    while (parser->current->type == T_K_PRIVATE || parser->current->type == T_K_VOLATILE) {
        if (is_private && parser->current->type == T_K_PRIVATE ||
            is_volatile && parser->current->type == T_K_VOLATILE) {
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

    Token *next_token = lexer_get_next_token_without_advance(parser->lexer);
    if (parser_is_expression(next_token) && is_inner_statement) {
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
        fprintf(stderr, "Parser: Token `%s`, is cannot be parsed", token_print(parser->current->type));
        parser_show_error(parser);
    }

    return type;
}

AST *parser_parse_variable_declaration(Parser *parser, AST *variable_type, bool is_private, bool is_volatile) {
    AST *variable_ast = ast_init_with_type(AST_TYPE_VARIABLE_DEFINITION);
    variable_ast->value.VariableDeclaration.type = variable_type;
    variable_ast->value.VariableDeclaration.identifier = parser_advance(parser, T_IDENTIFIER)->value;
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
    function_ast->value.FunctionDeclaration.function_name = parser_advance(parser, T_IDENTIFIER)->value;
    function_ast->value.FunctionDeclaration.is_private = is_private;

    // Parse the parameters
    parser_advance(parser, T_PARENS_OPEN);

    // Allocate the Parameters
    function_ast->value.FunctionDeclaration.argument_size = 0;
    function_ast->value.FunctionDeclaration.arguments = calloc(1, sizeof(AST *));

    while (parser->current->type != T_PARENS_CLOSE) {
        // Re-size the array
        function_ast->value.FunctionDeclaration.argument_size += 1;
        function_ast->value.FunctionDeclaration.arguments = realloc(function_ast->value.FunctionDeclaration.arguments,
                                                                    function_ast->value.FunctionDeclaration.argument_size *
                                                                    sizeof(AST *));

        // Create item
        AST *parameter_ast = ast_init_with_type(AST_TYPE_FUNCTION_ARGUMENT);
        parameter_ast->value.FunctionDeclarationArgument.argument_type = parser_parse_type(parser);
        parameter_ast->value.FunctionDeclarationArgument.argument_name = parser_advance(parser, T_IDENTIFIER)->value;

        // Insert the item
        function_ast->value.FunctionDeclaration.arguments[function_ast->value.FunctionDeclaration.argument_size -
                                                          1] = parameter_ast;

        // Advance to the next argument
        if (parser->current->type == T_COMMA)
            parser_advance(parser, T_COMMA);
    }

    parser_advance(parser, T_PARENS_CLOSE);

    function_ast->value.FunctionDeclaration.body = parser_parse_body(parser);

    return function_ast;
}

AST *parser_parse_inline_asm(Parser *parser) {
    AST *asm_tree = ast_init_with_type(AST_TYPE_INLINE_ASSEMBLY);

    parser_advance(parser, T_K_ASM);
    parser_advance(parser, T_BRACE_OPEN);

    asm_tree->value.InlineAssembly.instructions_size = 0;
    asm_tree->value.InlineAssembly.instruction = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        // Update the size of the array
        asm_tree->value.InlineAssembly.instructions_size += 1;
        asm_tree->value.InlineAssembly.instruction = realloc(asm_tree->value.InlineAssembly.instruction,
                                                             asm_tree->value.InlineAssembly.instructions_size *
                                                             sizeof(AST *));

        // Insert the item
        asm_tree->value.InlineAssembly.instruction[asm_tree->value.InlineAssembly.instructions_size -
                                                   1] = parser_advance(parser, T_STRING)->value;
    }

    parser_advance(parser, T_BRACE_CLOSE);

    return asm_tree;
}

AST *parser_parse_import_statement(Parser *parser) {
    AST *import_tree = ast_init_with_type(AST_TYPE_IMPORT);

    parser_advance(parser, T_K_IMPORT);
    char *identifier = parser_advance(parser, T_IDENTIFIER)->value;

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
    char *enum_name = parser_advance(parser, T_IDENTIFIER)->value;

    enum_ast->value.EnumDeclaration.enum_name = enum_name;

    parser_advance(parser, T_BRACE_OPEN);

    enum_ast->value.EnumDeclaration.case_size = 0;
    enum_ast->value.EnumDeclaration.cases = calloc(1, sizeof(AST *));

    int case_counter = 0;
    while (parser->current->type != T_BRACE_CLOSE) {
        enum_ast->value.EnumDeclaration.case_size += 1;
        enum_ast->value.EnumDeclaration.cases = realloc(enum_ast->value.EnumDeclaration.cases,
                                                        enum_ast->value.EnumDeclaration.case_size *
                                                        sizeof(AST *));

        AST *enum_case = ast_init_with_type(AST_TYPE_ENUM_ITEM);
        parser_advance(parser, T_K_CASE);
        enum_case->value.EnumItem.case_name = parser_advance(parser, T_IDENTIFIER)->value;

        if (parser->current->type == T_EQUAL) {
            parser_advance(parser, T_EQUAL);
            char *case_counter_string = parser_advance(parser, T_INT)->value;
            case_counter = (int) strtol(case_counter_string, &case_counter_string, 10);
        }

        enum_case->value.EnumItem.case_value = case_counter;
        parser_advance(parser, T_COMMA);

        enum_ast->value.EnumDeclaration.cases[enum_ast->value.EnumDeclaration.case_size -
                                              1] = enum_case;

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
    bool is_else = parser->current->type == T_K_ELSE;

    if (is_else)
        parser_advance(parser, T_K_ELSE);
    else
        parser_advance(parser, T_K_IF);

    if (is_else) {
        if (parser->current->type == T_K_IF) {
            if_else_statement_ast->value.IfElseStatement.is_else_if_statement = true;
            parser_advance(parser, T_K_IF);
        } else {
            if_else_statement_ast->value.IfElseStatement.is_else_statement = true;
        }
    } else {
        if_else_statement_ast->value.IfElseStatement.is_else_if_statement = false;
        if_else_statement_ast->value.IfElseStatement.is_else_statement = false;
    }

    // The expression
    if (!if_else_statement_ast->value.IfElseStatement.is_else_statement) {
        parser_advance(parser, T_PARENS_OPEN);
        if_else_statement_ast->value.IfElseStatement.expression = parser_parse_expression(parser);
        parser_advance(parser, T_PARENS_CLOSE);
    }

    // The body
    if_else_statement_ast->value.IfElseStatement.body = parser_parse_body(parser);

    return if_else_statement_ast;
}

AST *parser_parse_while_statement(Parser *parser) {
    AST *while_ast = ast_init_with_type(AST_TYPE_WHILE_STATEMENT);

    parser_advance(parser, T_K_WHILE);

    // The expression
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

    for_ast->value.ForLoop.body_size = 0;
    for_ast->value.ForLoop.body = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        for_ast->value.ForLoop.body_size += 1;

        for_ast->value.ForLoop.body = realloc(for_ast->value.ForLoop.body,
                                              for_ast->value.ForLoop.body_size *
                                              sizeof(AST *));

        if (parser->current->type == T_K_CONTINUE || parser->current->type == T_K_BREAK) {
            AST *literal_value = ast_init_with_type(AST_TYPE_LITERAL);
            literal_value->value.Literal.literal_value = parser->current;
            for_ast->value.ForLoop.body[for_ast->value.ForLoop.body_size -
                                        1] = literal_value;
            parser_advance_without_check(parser);
        } else {
            for_ast->value.ForLoop.body[for_ast->value.ForLoop.body_size -
                                        1] = parser_parse_inner_statement(parser);
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
    do_catch_ast->value.DoCatchOrWhileStatement.do_body_size = 0;
    do_catch_ast->value.DoCatchOrWhileStatement.do_body = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        do_catch_ast->value.DoCatchOrWhileStatement.do_body_size += 1;

        do_catch_ast->value.DoCatchOrWhileStatement.do_body = realloc(
                do_catch_ast->value.DoCatchOrWhileStatement.do_body,
                do_catch_ast->value.DoCatchOrWhileStatement.do_body_size *
                sizeof(AST *));

        do_catch_ast->value.DoCatchOrWhileStatement.do_body[do_catch_ast->value.DoCatchOrWhileStatement.do_body_size -
                                                            1] = parser_parse_inner_statement(parser);
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

        do_catch_ast->value.DoCatchOrWhileStatement.second_body_size = 0;
        do_catch_ast->value.DoCatchOrWhileStatement.second_body = calloc(1, sizeof(AST *));

        while (parser->current->type != T_BRACE_CLOSE) {
            do_catch_ast->value.DoCatchOrWhileStatement.second_body_size += 1;

            do_catch_ast->value.DoCatchOrWhileStatement.second_body = realloc(
                    do_catch_ast->value.DoCatchOrWhileStatement.second_body,
                    do_catch_ast->value.DoCatchOrWhileStatement.second_body_size *
                    sizeof(AST *));
            do_catch_ast->value.DoCatchOrWhileStatement.second_body[
                    do_catch_ast->value.DoCatchOrWhileStatement.second_body_size -
                    1] = parser_parse_inner_statement(parser);
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

    switch_tree->value.SwitchStatement.switch_cases_size = 0;
    switch_tree->value.SwitchStatement.switch_cases = calloc(1, sizeof(AST *));

    while (parser->current->type != T_BRACE_CLOSE) {
        switch_tree->value.SwitchStatement.switch_cases_size += 1;

        switch_tree->value.SwitchStatement.switch_cases = realloc(switch_tree->value.SwitchStatement.switch_cases,
                                                                  switch_tree->value.SwitchStatement.switch_cases_size *
                                                                  sizeof(AST *));
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
                parser_advance(parser, T_K_BREAK);
            } else {
                switch_case->value.SwitchCase.body[switch_case->value.SwitchCase.body_size -
                                                   1] = parser_parse_inner_statement(parser);
            }

            if (parser->current->type == T_BRACE_CLOSE || parser->current->type == T_K_CASE ||
                parser->current->type == T_K_DEFAULT) {
                break;
            }
        }

        switch_tree->value.SwitchStatement.switch_cases[switch_tree->value.SwitchStatement.switch_cases_size -
                                                        1] = switch_case;
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

    matches_tree->value.SwitchStatement.switch_cases_size = 0;
    matches_tree->value.SwitchStatement.switch_cases = calloc(1, sizeof(AST *));

    parser_advance(parser, T_BRACE_OPEN);
    while (parser->current->type != T_BRACE_CLOSE) {
        matches_tree->value.SwitchStatement.switch_cases_size += 1;

        matches_tree->value.SwitchStatement.switch_cases = realloc(matches_tree->value.SwitchStatement.switch_cases,
                                                                   matches_tree->value.SwitchStatement.switch_cases_size *
                                                                   sizeof(AST *));

        AST *switch_case = ast_init_with_type(AST_TYPE_SWITCH_CASE);

        if (parser->current->type == T_IDENTIFIER && strlen(parser->current->value) == 1 &&
            parser->current->value[0] == '_') {
            // Default Clause
            switch_case->value.SwitchCase.is_default = true;
        }

        switch_case->value.SwitchCase.expression = parser_parse_expression(parser);

        parser_advance(parser, T_COLON);

        switch_case->value.SwitchCase.body = calloc(1, sizeof(AST *));

        if (parser->current->type == T_BRACE_OPEN) {
            parser_advance(parser, T_BRACE_OPEN);

            while (parser->current->type != T_BRACE_CLOSE) {
                switch_case->value.SwitchCase.body_size += 1;
                switch_case->value.SwitchCase.body = realloc(switch_case->value.SwitchCase.body,
                                                             switch_case->value.SwitchCase.body_size *
                                                             sizeof(AST *));

                switch_case->value.SwitchCase.body[switch_case->value.SwitchCase.body_size -
                                                   1] = parser_parse_inner_statement(
                        parser);
                if (parser->current->type == T_BRACE_CLOSE)
                    break;
            }

            parser_advance(parser, T_BRACE_CLOSE);
        } else {
            switch_case->value.SwitchCase.body_size = 1;
            switch_case->value.SwitchCase.body[switch_case->value.SwitchCase.body_size - 1] = parser_parse_expression(
                    parser);
        }

        matches_tree->value.SwitchStatement.switch_cases[matches_tree->value.SwitchStatement.switch_cases_size -
                                                         1] = switch_case;

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
    alias_ast->value.Alias.alias_name = parser_advance(parser, T_IDENTIFIER)->value;
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
    switch (parser->current->type) {
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_CHAR:
        case T_HEX:
        case T_OCTAL:
        case T_IDENTIFIER:
        case T_K_SELF:
            return parser_parse_expression_literal(parser);
        case T_PARENS_OPEN:
            return parser_parse_expression_grouping(parser);
        case T_NOT:
            parser_advance(parser, T_NOT);
            AST *not_ast = ast_init_with_type(AST_TYPE_NOT);
            not_ast->value.Not.expression = parser_parse_expression(parser);
            return not_ast;
        case T_K_MATCHES:
            return parser_parse_matches_statement(parser);
        case T_K_TRY:
            return parser_parse_expression_try(parser);
        case T_PERIOD:
            parser_advance(parser, T_PERIOD);
            AST *init_ast = ast_init_with_type(AST_TYPE_STRUCT_INITIALIZER);
            init_ast->value.StructInitializer.function_call = parser_parse_expression(parser);
            return init_ast;
        default:
            fprintf(stderr, "Parser: Expression is cannot be parsed");
            parser_show_error(parser);
    }
}

AST *parser_parse_expression_grouping(Parser *parser) {
    parser_advance(parser, T_PARENS_OPEN);
    AST *grouping_ast = ast_init_with_type(AST_TYPE_GROUPING);

    grouping_ast->value.Grouping.expression = parser_parse_expression(parser);

    parser_advance(parser, T_PARENS_CLOSE);

    if (parser_is_expression(parser->current)) {
        AST *binary_ast = ast_init_with_type(AST_TYPE_BINARY);
        binary_ast->value.Binary.left = grouping_ast;
        binary_ast->value.Binary.operator = parser_advance_without_check(parser);
        binary_ast->value.Binary.right = parser_parse_expression(parser);

        return binary_ast;
    } else {
        return grouping_ast;
    }
}

AST *parser_parse_expression_literal(Parser *parser) {
    if (parser->current->type == T_IDENTIFIER)
        return parser_parse_expression_identifier(parser);
    else {
        Token *literal_value = parser_advance_without_check(parser);
        AST *ast = ast_init_with_type(AST_TYPE_LITERAL);
        ast->value.Literal.literal_value = literal_value;

        if (parser_is_expression(parser->current)) {
            AST *binary_ast = ast_init_with_type(AST_TYPE_BINARY);
            binary_ast->value.Binary.left = ast;
            binary_ast->value.Binary.operator = parser_advance_without_check(parser);
            binary_ast->value.Binary.right = parser_parse_expression(parser);

            return binary_ast;
        } else {
            return ast;
        }
    }
}

AST *parser_parse_expression_identifier(Parser *parser) {
    Token *identifier_name = parser_advance(parser, T_IDENTIFIER);
    AST *ast;

    if (parser->current->type == T_PARENS_OPEN) {
        ast = parser_parse_function_call(parser, identifier_name->value);
    } else {
        ast = ast_init_with_type(AST_TYPE_LITERAL);
        ast->value.Literal.literal_value = identifier_name;
    }

    if (parser_is_expression(parser->current)) {
        AST *binary_ast = ast_init_with_type(AST_TYPE_BINARY);
        binary_ast->value.Binary.left = ast;
        binary_ast->value.Binary.operator = parser_advance_without_check(parser);
        binary_ast->value.Binary.right = parser_parse_expression(parser);

        return binary_ast;
    } else {
        return ast;
    }
}

bool parser_is_expression(Token *token) {
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
        type->value.ValueKeyword.token = parser_advance_without_check(parser);
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

Token *parser_advance(Parser *parser, int token_type) {
    if (parser->current->type != token_type) {
        fprintf(stderr, "Parser: Expected `%s`, instead got `%s`", token_print(token_type),
                token_print(parser->current->type));
        parser_show_error(parser);
    }
    parser->previous = parser->current;
    parser->current = lexer_get_next_token(parser->lexer);

    return parser->previous;
}

Token *parser_advance_without_check(Parser *parser) {
    parser->previous = parser->current;
    parser->current = lexer_get_next_token(parser->lexer);

    return parser->previous;
}