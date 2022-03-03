//
//  semantic_analyzer.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "semantic_analyzer.h"

void semantic_analyzer_error(void) {
    fprintf(stderr, " || No available information\n");
    exit(-4);
}

void semantic_analyzer_run_analysis(AST **ast_items, uintptr_t ast_items_size) {
    UNMap *table = semantic_analyzer_create_symbol_table(ast_items, ast_items_size);

    semantic_analyzer_check_duplicate_symbols(table);

    for (int i = 0; i < table->items; i++) {
        SemanticAnalyzerSymbol *symbol_struct = (SemanticAnalyzerSymbol *) table->pair_values[i]->value;

        semantic_analyzer_check_statement(table, symbol_struct);
    }

    unmap_destroy(table);
}

void semantic_analyzer_check_statement(UNMap *table, SemanticAnalyzerSymbol *symbol_struct) {
    switch (symbol_struct->symbol_type) {
        case AST_TYPE_FUNCTION_DECLARATION:
            semantic_analyzer_check_function_declaration(table, symbol_struct->symbol_ast, false, NULL);
            break;
        case AST_TYPE_STRUCT_OR_UNION_DECLARATION:
            semantic_analyzer_check_struct_or_union_declaration(table, symbol_struct);
            break;
        default:
//                fprintf(stderr, "Semantic Analyzer: Cannot check type `%d`", symbol_struct->symbol_type);
//                semantic_analyzer_error();
            break;
    }
}

void
semantic_analyzer_check_inner_statement(UNMap *table, AST **body, uintptr_t body_size, AST *body_ast, int body_position,
                                        AST *function_ast, bool is_struct_member, AST *struct_ast) {
    switch (body_ast->type) {
        case AST_TYPE_VARIABLE_DEFINITION:
            semantic_analyzer_check_variable_definition(table, body_ast,
                                                        body,
                                                        function_ast->value.FunctionDeclaration.body->size,
                                                        function_ast, is_struct_member,
                                                        struct_ast);
            break;
        case AST_TYPE_BINARY:
            semantic_analyzer_check_expression(table, body_ast, body_position, body,
                                               body_size,
                                               function_ast,
                                               is_struct_member, struct_ast);
            break;
        case AST_TYPE_RETURN:
            semantic_analyzer_check_expression(table, body_ast->value.Return.return_expression, body_position,
                                               body,
                                               body_size,
                                               function_ast,
                                               is_struct_member, struct_ast);
            break;
        case AST_TYPE_FUNCTION_CALL:
            semantic_analyzer_check_expression_function_call(table, body_ast, body_position, body,
                                                             body_size,
                                                             function_ast,
                                                             struct_ast);
            break;
        default:
//                fprintf(stderr, "Semantic Analyzer: Cannot check type `%d`", body->value.Body.body[i]->type);
//                semantic_analyzer_error();
            break;
    }
}

void semantic_analyzer_check_struct_statement(UNMap *table, AST **body, uintptr_t body_size, AST *body_ast,
                                              int body_position, AST *struct_ast) {
    switch (body_ast->type) {
        case AST_TYPE_VARIABLE_DEFINITION:
            semantic_analyzer_check_variable_definition(table, body_ast,
                                                        body,
                                                        body_size,
                                                        struct_ast, true,
                                                        struct_ast);
            break;
        case AST_TYPE_FUNCTION_DECLARATION:
            semantic_analyzer_check_function_declaration(table, body_ast, true, struct_ast);
            break;
        case AST_TYPE_STRUCT_INITIALIZER:
            semantic_analyzer_check_struct_initializer(table, body_ast, struct_ast);
            break;
        default:
            fprintf(stderr, "Semantic Analyzer: Struct cannot have type `%d`", body_ast->type);
            semantic_analyzer_error();
            break;
    }
}

void semantic_analyzer_check_struct_or_union_declaration(UNMap *table, SemanticAnalyzerSymbol *symbol_struct) {
    // Check for duplicate members
    UNMap *new_map = semantic_analyzer_create_symbol_table(
            (AST **) symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members->data,
            symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members->size);
    semantic_analyzer_check_duplicate_symbols(new_map);

    free(new_map);

    for (int i = 0; i < symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members->size; i++) {
        AST *ast_member = mxDynamicArrayGet(symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members, i);

        semantic_analyzer_check_struct_statement(table,
                                                 (AST **) symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members->data,
                                                 symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members->size,
                                                 ast_member, i, symbol_struct->symbol_ast);
    }
}

void semantic_analyzer_check_struct_initializer(UNMap *table, AST *initializer_ast, AST *struct_declaration) {
    // Check the variable definitions in the struct, and check if they are initialized
    for (int i = 0; i < initializer_ast->value.StructInitializer.body->size; i++) {
        AST *ast_member = mxDynamicArrayGet(initializer_ast->value.StructInitializer.body, i);

        // Check if the member is a self statement, and check which members weren't initialized
        semantic_analyzer_check_expression(table, ast_member, i,
                                           (AST **) initializer_ast->value.StructInitializer.body->data,
                                           initializer_ast->value.StructInitializer.body->size,
                                           struct_declaration,
                                           true, struct_declaration);
    }
}

int
semantic_analyzer_check_expression_function_call(UNMap *table, AST *expression,
                                                 __attribute__((unused)) int position_inside_body,
                                                 __attribute__((unused)) AST **body,
                                                 __attribute__((unused)) uintptr_t body_size,
                                                 __attribute__((unused)) AST *function_declaration,
                                                 __attribute__((unused)) AST *struct_declaration) {
//    if (strcmp(expression->value.FunctionCall.function_call_name, "print") == 0) {
//        return T_K_VOID;
//    }
    for (int j = 0; j < table->items; j++) {
        char *table_item_name = table->pair_values[j]->key;

        if (strcmp(table_item_name, expression->value.FunctionCall.function_call_name) == 0 &&
            table_item_name[0] != '\0') {
            if (((SemanticAnalyzerSymbol *) table->pair_values[j]->value)->symbol_type ==
                AST_TYPE_FUNCTION_DECLARATION) {
                // Check the arguments
                AST *function = ((SemanticAnalyzerSymbol *) table->pair_values[j]->value)->symbol_ast;
                if (expression->value.FunctionCall.arguments->size !=
                    function->value.FunctionDeclaration.arguments->size) {
                    fprintf(stderr, "Semantic Analyzer: `%s` invalid number of arguments, expected %d, got %d",
                            expression->value.FunctionCall.function_call_name,
                            function->value.FunctionDeclaration.arguments->size,
                            expression->value.FunctionCall.arguments->size);
                    semantic_analyzer_error();
                }
                // Check the arguments types
                for (int i = 0; i < expression->value.FunctionCall.arguments->size; ++i) {
                    AST *argument1 = mxDynamicArrayGet(expression->value.FunctionCall.arguments, i);
                    AST *argument2 = ((AST *) mxDynamicArrayGet(function->value.FunctionDeclaration.arguments,
                                                                i))->value.FunctionDeclarationArgument.argument_type;
                    semantic_analyzer_compare_types(argument1, argument2);
                }
                return function->value.FunctionDeclaration.return_type->value.ValueKeyword.token->type;
            } else {
                fprintf(stderr, "Semantic Analyzer: `%s` is not a valid function",
                        expression->value.FunctionCall.function_call_name);
                semantic_analyzer_error();
            }
        } else {
            if ((j + 1) >= table->items) {
                fprintf(stderr, "Semantic Analyzer: `%s` is not a valid function",
                        expression->value.FunctionCall.function_call_name);
                semantic_analyzer_error();
            }
        }
    }

    return 0;
}

void semantic_analyzer_check_function_declaration(UNMap *table, AST *function_declaration, bool is_struct_member,
                                                  AST *struct_declaration) {
    semantic_analyzer_check_function_declaration_argument(table, function_declaration,
                                                          ((AST **) function_declaration->value.FunctionDeclaration.arguments->data),
                                                          function_declaration->value.FunctionDeclaration.arguments->size,
                                                          is_struct_member);
    semantic_analyzer_check_function_declaration_body(table, function_declaration, is_struct_member,
                                                      struct_declaration);
}

void semantic_analyzer_check_function_declaration_argument(UNMap *table,
                                                           __attribute__((unused)) AST *function_declaration_ast,
                                                           AST **arguments, uintptr_t argument_size,
                                                           __attribute__((unused)) bool is_struct_member) {
    if (argument_size == 0)
        return;

    for (int i = 0; i < argument_size; i++) {
        char *argument_i = arguments[i]->value.FunctionDeclarationArgument.argument_name;
        AST *argument_type_i = arguments[i]->value.FunctionDeclarationArgument.argument_type;

        if (argument_type_i->type != AST_TYPE_VALUE_KEYWORD) {
            fprintf(stderr, "Semantic Analyzer: Function Arguments must be a type `%d`", argument_type_i->type);
            semantic_analyzer_error();
        }

        if (argument_type_i->value.ValueKeyword.token->type == T_IDENTIFIER) {
            semantic_analyzer_check_if_type_exists(table, argument_type_i->value.ValueKeyword.token->value);
        }

        for (int j = i + 1; j < argument_size; j++) {
            char *argument_j = arguments[j]->value.FunctionDeclarationArgument.argument_name;

            AST *argument_type_j = arguments[i]->value.FunctionDeclarationArgument.argument_type;

            if (argument_type_j->type != AST_TYPE_VALUE_KEYWORD) {
                fprintf(stderr, "Semantic Analyzer: Function Arguments must be a type `%d`", argument_type_j->type);
                semantic_analyzer_error();
            }

            if (argument_type_j->value.ValueKeyword.token->type == T_IDENTIFIER) {
                semantic_analyzer_check_if_type_exists(table, argument_type_j->value.ValueKeyword.token->value);
            }

            if (strcmp(argument_i, argument_j) == 0) {
                fprintf(stderr, "Semantic Analyzer: Found Duplicate Function Parameter: `%s`", argument_j);
                semantic_analyzer_error();

            }
        }
    }
}

void
semantic_analyzer_check_function_declaration_body(UNMap *table, AST *function_declaration_ast, bool is_struct_member,
                                                  AST *struct_declaration) {

    AST **body = (AST **) function_declaration_ast->value.FunctionDeclaration.body->data;
    for (int i = 0; i < function_declaration_ast->value.FunctionDeclaration.body->size; ++i) {
        AST *body_i = mxDynamicArrayGet(function_declaration_ast->value.FunctionDeclaration.body, i);

        semantic_analyzer_check_inner_statement(table, body,
                                                function_declaration_ast->value.FunctionDeclaration.body->size, body_i,
                                                i, function_declaration_ast, is_struct_member, struct_declaration);
    }

    free(body);
}

void
semantic_analyzer_check_variable_definition(UNMap *symbol_table, AST *variable_ast, AST **body, uintptr_t body_size,
                                            AST *function_declaration_ast, bool is_struct_member,
                                            AST *struct_declaration) {
    // If `is_struct_member` we will check the variables in the struct, in the last though, if it doesn't match any other variables,
    // or we could show an error saying re-defined variable?
    // or we can force the user to type self (chooses)

    // Check the variable type
    if (variable_ast->value.VariableDeclaration.type->value.ValueKeyword.token->type == T_IDENTIFIER) {
        semantic_analyzer_check_if_type_exists(symbol_table,
                                               variable_ast->value.VariableDeclaration.type->value.ValueKeyword.token->value);
    }

    if (!is_struct_member) {
        // Check if the variable is defined elsewhere in the body
        // We will increment up the body
        int position_inside_body = 0;
        for (int i = 0; i < body_size; ++i) {
            if (body[i]->type == AST_TYPE_VARIABLE_DEFINITION) {
                if (body[i] == variable_ast) {
                    position_inside_body = i;
                }
                if ((strcmp(variable_ast->value.VariableDeclaration.identifier,
                            body[i]->value.VariableDeclaration.identifier) == 0) &&
                    body[i] != variable_ast) {
                    fprintf(stderr, "Semantic Analyzer: Variable `%s`, has been defined more than once",
                            variable_ast->value.VariableDeclaration.identifier);
                    semantic_analyzer_error();

                }
            }
        }

        if (variable_ast->value.VariableDeclaration.is_initialized) {
            semantic_analyzer_check_expression(symbol_table, variable_ast->value.VariableDeclaration.value,
                                               position_inside_body,
                                               body, body_size, function_declaration_ast, is_struct_member,
                                               struct_declaration);
        }
    } else {
        // Struct member variable
        if (function_declaration_ast->type == AST_TYPE_STRUCT_OR_UNION_DECLARATION) {
            if (variable_ast->value.VariableDeclaration.is_initialized) {
                fprintf(stderr, "Semantic Analyzer: Variable `%s`, must not be initialized inside a struct",
                        variable_ast->value.VariableDeclaration.identifier);
                semantic_analyzer_error();
            }
        }
    }
}

int semantic_analyzer_check_expression(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                       uintptr_t body_size, AST *function_declaration, bool is_struct_member,
                                       AST *struct_declaration) {
    switch (expression->type) {
        case AST_TYPE_BINARY:
            if (is_struct_member && expression->value.Binary.left->value.Literal.literal_value->type == T_K_SELF) {
                return semantic_analyzer_check_struct_self(table, expression, position_inside_body, body, body_size,
                                                           function_declaration, is_struct_member, struct_declaration);
            }
            return semantic_analyzer_check_expression_binary(table, expression, position_inside_body, body, body_size,
                                                             function_declaration, is_struct_member,
                                                             struct_declaration);
        case AST_TYPE_LITERAL:
            return semantic_analyzer_check_expression_literal(table, expression, position_inside_body, body, body_size,
                                                              function_declaration, is_struct_member,
                                                              struct_declaration);
        case AST_TYPE_FUNCTION_CALL:
            return semantic_analyzer_check_expression_function_call(table, expression, position_inside_body, body,
                                                                    body_size,
                                                                    function_declaration, struct_declaration);
        case AST_TYPE_GROUPING:
            return semantic_analyzer_check_expression_grouping(table, expression, position_inside_body, body, body_size,
                                                               function_declaration, is_struct_member,
                                                               struct_declaration);
        default:
            fprintf(stderr, "Semantic Analyzer: Expression cannot be checked %d", expression->type);
            semantic_analyzer_error();
    }

    return 0;
}

int semantic_analyzer_check_expression_grouping(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                                uintptr_t body_size, AST *function_declaration, bool is_struct_member,
                                                AST *struct_declaration) {
    return semantic_analyzer_check_expression(table, expression->value.Grouping.expression, position_inside_body, body,
                                              body_size,
                                              function_declaration, is_struct_member, struct_declaration);
}

int semantic_analyzer_check_expression_binary(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                              uintptr_t body_size, AST *function_declaration, bool is_struct_member,
                                              AST *struct_declaration) {
    // Check the left side
    int left_type = semantic_analyzer_check_expression(table, expression->value.Binary.left, position_inside_body, body,
                                                       body_size,
                                                       function_declaration, is_struct_member, struct_declaration);
    // The token is already validated by the parser
    // Check if the left side and right side are valid.

    int right_type = semantic_analyzer_check_expression(table, expression->value.Binary.right, position_inside_body,
                                                        body, body_size,
                                                        function_declaration, is_struct_member, struct_declaration);

    if (!semantic_analyzer_types_are_allowed(left_type, right_type)) {
        fprintf(stderr, "Semantic Analyzer: expressions have different types: %s :: %s", token_print(left_type),
                token_print(right_type));
        semantic_analyzer_error();
    }

    return left_type;
}

int semantic_analyzer_check_expression_literal(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                               __attribute__((unused)) uintptr_t body_size, AST *function_declaration,
                                               bool is_struct_member,
                                               __attribute__((unused)) AST *struct_declaration) {
    switch (expression->value.Literal.literal_value->type) {
        case T_INT:
        case T_FLOAT:
        case T_STRING:
        case T_CHAR:
        case T_HEX:
        case T_OCTAL:
            return expression->value.Literal.literal_value->type;
        case T_IDENTIFIER:
            // Check for variable names before the variable definition or a struct name
            for (int i = position_inside_body; i > 0; i--) {
                if (body[i]->type == AST_TYPE_VARIABLE_DEFINITION ||
                    (body[i]->type == AST_TYPE_BINARY /* && !is_struct_member */)) {
                    if (strcmp(body[i - 1]->value.VariableDeclaration.identifier,
                               expression->value.Literal.literal_value->value) == 0) {
                        return body[i - 1]->value.VariableDeclaration.type->value.ValueKeyword.token->type;
                    } else if (i >= position_inside_body) {
                        // Error, not a variable
                    }
                }
            }

            // Check for function arguments
            for (int j = 0; j < function_declaration->value.FunctionDeclaration.arguments->size; j++) {
                AST *argument_name = mxDynamicArrayGet(function_declaration->value.FunctionDeclaration.arguments, j);
                if (strcmp(expression->value.Literal.literal_value->value,
                           argument_name->value.FunctionDeclarationArgument.argument_name) == 0) {
                    return argument_name->value.FunctionDeclarationArgument.argument_type->value.ValueKeyword.token->type;
                } else if (j >= function_declaration->value.FunctionDeclaration.arguments->size) {
                    // Error, not a function argument
                }
            }


            semantic_analyzer_check_if_type_exists(table, expression->value.Literal.literal_value->value);
            break;
        case T_K_SELF:
            if (!is_struct_member) {
                fprintf(stderr, "Semantic Analyzer: self must be used inside a struct member");
                semantic_analyzer_error();
            }
            return T_K_SELF;
        default:
            fprintf(stderr, "Semantic Analyzer: `%s` isn't a valid type",
                    expression->value.Literal.literal_value->value);
            semantic_analyzer_error();
    }

    return 0;
}

int semantic_analyzer_check_struct_self(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                        uintptr_t body_size, AST *function_declaration, bool is_struct_member,
                                        AST *struct_declaration) {
    if (expression->value.Binary.right->type != AST_TYPE_BINARY) {
        fprintf(stderr, "Semantic Analyzer: `%s` must have an expression",
                expression->value.Binary.left->value.Literal.literal_value->value);
        semantic_analyzer_error();
    }

    for (int j = 0; j < struct_declaration->value.StructOrUnionDeclaration.members->size; j++) {
        // Compare the values
        AST *member = mxDynamicArrayGet(struct_declaration->value.StructOrUnionDeclaration.members, j);
        if (member->type == AST_TYPE_VARIABLE_DEFINITION) {
            if (strcmp(member->value.VariableDeclaration.identifier,
                       expression->value.Binary.right->value.Binary.left->value.Literal.literal_value->value) == 0) {
                int expression_type = semantic_analyzer_check_expression(table,
                                                                         expression->value.Binary.right->value.Binary.right,
                                                                         position_inside_body, body, body_size,
                                                                         function_declaration,
                                                                         is_struct_member,
                                                                         struct_declaration);
                if (!semantic_analyzer_types_are_allowed(
                        member->value.VariableDeclaration.type->value.ValueKeyword.token->type, expression_type)) {
                    fprintf(stderr, "Semantic Analyzer: `%s` has the type of `%s` but was assigned type `%s`",
                            member->value.VariableDeclaration.identifier, token_print(
                                    member->value.VariableDeclaration.type->value.ValueKeyword.token->type),
                            token_print(expression_type));
                    semantic_analyzer_error();
                }

                return expression_type;
            } else {
                if ((j + 1) >= struct_declaration->value.StructOrUnionDeclaration.members->size) {
                    fprintf(stderr, "Semantic Analyzer: `%s` is not a valid struct member",
                            expression->value.Binary.right->value.Binary.left->value.Literal.literal_value->value);
                    semantic_analyzer_error();
                }
            }
        } else {
            if ((j + 1) >= struct_declaration->value.StructOrUnionDeclaration.members->size) {
                fprintf(stderr, "Semantic Analyzer: `%s` is not a valid struct member",
                        expression->value.Binary.right->value.Binary.left->value.Literal.literal_value->value);
                semantic_analyzer_error();
            }
        }
    }

    return -1;
}

void semantic_analyzer_check_if_type_exists(UNMap *symbol_table, char *type_name) {
    for (int j = 0; j < symbol_table->items; j++) {
        char *table_item_name = symbol_table->pair_values[j]->key;

        if (strcmp(table_item_name, type_name) == 0 &&
            table_item_name[0] != '\0') {
            if (((SemanticAnalyzerSymbol *) symbol_table->pair_values[j]->value)->symbol_type ==
                AST_TYPE_FUNCTION_DECLARATION) {
                fprintf(stderr, "Semantic Analyzer: `%s` is a function name, which cannot be used as a type",
                        type_name);
                semantic_analyzer_error();
            }
            return;
        } else {
            if ((j + 1) >= symbol_table->items) {
                fprintf(stderr, "Semantic Analyzer: `%s` isn't an existing type", type_name);
                semantic_analyzer_error();
            }
        }
    }
}

bool semantic_analyzer_types_are_allowed(int type1, int type2) {
    if ((type1 == T_INT || type1 == T_FLOAT || type1 == T_K_INT || type1 == T_K_FLOAT) &&
        (type2 == T_INT || type2 == T_FLOAT || type2 == T_K_INT || type2 == T_K_FLOAT))
        return true;
    else if ((type1 == T_STRING || type1 == T_K_STRING) && (type2 == T_STRING || type2 == T_K_STRING))
        return true;
    else
        return false;
}

void semantic_analyzer_compare_types(AST *type1, AST *type2) {
    // Compare the value types
    if (!semantic_analyzer_types_are_allowed(type1->value.ValueKeyword.token->type,
                                             type2->value.ValueKeyword.token->type)) {
        fprintf(stderr, "Semantic Analyzer: `%s` and `%s` types are not compatible",
                token_print(type1->value.ValueKeyword.token->type),
                token_print(type2->value.ValueKeyword.token->type));
        semantic_analyzer_error();
    }
}

void semantic_analyzer_check_duplicate_symbols(UNMap *table) {
    for (int i = 0; i < table->items - 1; i++) {
        for (int j = i + 1; j < table->items; j++) {
            if (table->pair_values[i]->key != NULL) {
                if (strcmp(table->pair_values[i]->key, table->pair_values[j]->key) == 0 &&
                    table->pair_values[i]->key[0] != '\0') {
                    fprintf(stderr, "Semantic Analyzer: Found Duplicate Symbol `%s`", table->pair_values[i]->key);
                    unmap_destroy(table);
                    semantic_analyzer_error();
                }
            }
        }
    }
}

UNMap *semantic_analyzer_create_symbol_table(AST **ast_items, uintptr_t ast_items_size) {
    UNMap *symbol_map = unmap_init();
    // Create a symbol table
    for (int i = 0; i < ast_items_size; i++) {
        SemanticAnalyzerSymbol *symbol_struct = malloc(sizeof(SemanticAnalyzerSymbol));
        switch (ast_items[i]->type) {
            case AST_TYPE_FUNCTION_DECLARATION:
                symbol_struct->symbol_name = ast_items[i]->value.FunctionDeclaration.function_name;
                symbol_struct->symbol_type = ast_items[i]->type;
                symbol_struct->symbol_ast = ast_items[i];
                break;
            case AST_TYPE_STRUCT_OR_UNION_DECLARATION:
                symbol_struct->symbol_name = ast_items[i]->value.StructOrUnionDeclaration.name;
                symbol_struct->symbol_type = ast_items[i]->type;
                symbol_struct->symbol_ast = ast_items[i];
                break;
            case AST_TYPE_ALIAS:
                symbol_struct->symbol_name = ast_items[i]->value.Alias.alias_name;
                symbol_struct->symbol_type = ast_items[i]->type;
                symbol_struct->symbol_ast = ast_items[i];
                break;
            case AST_TYPE_VARIABLE_DEFINITION:
                symbol_struct->symbol_name = ast_items[i]->value.VariableDeclaration.identifier;
                symbol_struct->symbol_type = ast_items[i]->type;
                symbol_struct->symbol_ast = ast_items[i];
                break;
            case AST_TYPE_STRUCT_INITIALIZER:
                symbol_struct->symbol_name = NULL;
                symbol_struct->symbol_ast = NULL;
                symbol_struct->symbol_type = -1;
                break;
            default:
                fprintf(stderr, "Semantic Analyzer: Cannot Check Type `%d`", ast_items[i]->type);
                semantic_analyzer_error();
        }

        UNMapPairValue *pair = malloc(sizeof(UNMapPairValue));

        if (symbol_struct->symbol_name != NULL) {
            pair->key = symbol_struct->symbol_name;
            pair->value = symbol_struct;
            unmap_push_back(symbol_map, pair);
        }
    }

    return symbol_map;
}
