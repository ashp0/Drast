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

        switch (symbol_struct->symbol_type) {
            case AST_TYPE_FUNCTION_DECLARATION:
                semantic_analyzer_check_function_declaration(table, symbol_struct->symbol_ast, false);
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

    unmap_destroy(table);
}

void semantic_analyzer_check_struct_or_union_declaration(UNMap *table, SemanticAnalyzerSymbol *symbol_struct) {
    // Create a new map, so we will iterate through the members of the struct and append them and their types to the new map

    // Check for duplicate members
    UNMap *new_map = semantic_analyzer_create_symbol_table(
            symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members,
            symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members_size);
    semantic_analyzer_check_duplicate_symbols(new_map);

    free(new_map);

    for (int i = 0; i < symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members_size; i++) {
        AST *ast_member = symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members[i];
        // Check if the member is a function
        if (ast_member->type == AST_TYPE_FUNCTION_DECLARATION) {
            semantic_analyzer_check_function_declaration(table, ast_member, true);
        } else if (ast_member->type == AST_TYPE_VARIABLE_DEFINITION) {
            semantic_analyzer_check_variable_definition(table, ast_member,
                                                        symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members,
                                                        symbol_struct->symbol_ast->value.StructOrUnionDeclaration.members_size,
                                                        symbol_struct->symbol_ast, true);
        }
    }
}

void semantic_analyzer_check_function_declaration(UNMap *table, AST *function_declaration,
                                                  bool is_struct_member) {
    semantic_analyzer_check_function_declaration_argument(table, function_declaration,
                                                          function_declaration->value.FunctionDeclaration.arguments,
                                                          function_declaration->value.FunctionDeclaration.arguments_size,
                                                          is_struct_member);
    semantic_analyzer_check_function_declaration_body(table, function_declaration, false);
}

void semantic_analyzer_check_function_declaration_argument(UNMap *table, AST *function_declaration_ast,
                                                           AST **arguments, uintptr_t argument_size,
                                                           bool is_struct_member) {
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

void semantic_analyzer_check_function_declaration_body(UNMap *table, AST *function_declaration_ast,
                                                       bool is_struct_member) {
    for (int i = 0; i < function_declaration_ast->value.FunctionDeclaration.body->value.Body.body_size; ++i) {
        AST *body = function_declaration_ast->value.FunctionDeclaration.body;

        switch (body->value.Body.body[i]->type) {
            case AST_TYPE_VARIABLE_DEFINITION:
                semantic_analyzer_check_variable_definition(table, body->value.Body.body[i],
                                                            body->value.Body.body, body->value.Body.body_size,
                                                            function_declaration_ast, is_struct_member);
            default:
//                fprintf(stderr, "Semantic Analyzer: Cannot check type `%d`", symbol_struct->symbol_type);
//                semantic_analyzer_error();
                break;
        }
    }
}

void
semantic_analyzer_check_variable_definition(UNMap *symbol_table, AST *variable_ast, AST **body, uintptr_t body_size,
                                            AST *function_declaration_ast, bool is_struct_member) {
    // If `is_struct_member` we will check the variables in the struct, in the last though, if it doesn't match any other variables,
    // or we could show an error saying re-defined variable?
    // or we can force the user to type self (choses)

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
                                               body, body_size, function_declaration_ast);
        }
    } else {
        // Struct member variable
        if (variable_ast->value.VariableDeclaration.is_initialized) {
            fprintf(stderr, "Semantic Analyzer: Variable `%s`, must not be initialized inside a struct",
                    variable_ast->value.VariableDeclaration.identifier);
            semantic_analyzer_error();
        }
    }
}

int semantic_analyzer_check_expression(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                       uintptr_t body_size,
                                       AST *function_declaration) {
    switch (expression->type) {
        case AST_TYPE_BINARY:
            return semantic_analyzer_check_expression_binary(table, expression, position_inside_body, body, body_size,
                                                             function_declaration);
        case AST_TYPE_LITERAL:
            return semantic_analyzer_check_expression_literal(table, expression, position_inside_body, body, body_size,
                                                              function_declaration);
        case AST_TYPE_FUNCTION_CALL:
            return semantic_analyzer_check_expression_function_call(table, expression, position_inside_body, body,
                                                                    body_size,
                                                                    function_declaration);
        case AST_TYPE_GROUPING:
            return semantic_analyzer_check_expression_grouping(table, expression, position_inside_body, body, body_size,
                                                               function_declaration);
        default:
            fprintf(stderr, "Semantic Analyzer: Expression cannot be checked");
            semantic_analyzer_error();
    }

    return 0;
}

int semantic_analyzer_check_expression_grouping(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                                uintptr_t body_size,
                                                AST *function_declaration) {
    return semantic_analyzer_check_expression(table, expression->value.Grouping.expression, position_inside_body, body,
                                              body_size,
                                              function_declaration);
}

int semantic_analyzer_check_expression_binary(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                              uintptr_t body_size,
                                              AST *function_declaration) {
    // Check the left side
    int left_type = semantic_analyzer_check_expression(table, expression->value.Binary.left, position_inside_body, body,
                                                       body_size,
                                                       function_declaration);
    // The token is already validated by the parser
    // Check if the left side and right side are valid.

    int right_type = semantic_analyzer_check_expression(table, expression->value.Binary.right, position_inside_body,
                                                        body, body_size,
                                                        function_declaration);

    if (!semantic_analyzer_types_are_allowed(left_type, right_type)) {
        fprintf(stderr, "Semantic Analyzer: expressions have different types: %s :: %s", token_print(left_type),
                token_print(right_type));
        semantic_analyzer_error();
    }

    return left_type;
}

int semantic_analyzer_check_expression_literal(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                               uintptr_t body_size,
                                               AST *function_declaration) {
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
                if (body[i]->type == AST_TYPE_VARIABLE_DEFINITION) {
                    if (strcmp(body[i - 1]->value.VariableDeclaration.identifier,
                               expression->value.Literal.literal_value->value) == 0) {
                        return body[i - 1]->value.VariableDeclaration.type->value.ValueKeyword.token->type;
                    } else if (i >= position_inside_body) {
                        // Error, not a variable
                    }
                }
            }

            // Check for function arguments
            for (int j = 0; j < function_declaration->value.FunctionDeclaration.arguments_size; j++) {
                AST *argument_name = function_declaration->value.FunctionDeclaration.arguments[j];
                if (strcmp(expression->value.Literal.literal_value->value,
                           argument_name->value.FunctionDeclarationArgument.argument_name) == 0) {
                    return argument_name->value.FunctionDeclarationArgument.argument_type->value.ValueKeyword.token->type;
                } else if (j >= function_declaration->value.FunctionDeclaration.arguments_size) {
                    // Error, not a function argument
                }
            }

            semantic_analyzer_check_if_type_exists(table, expression->value.Literal.literal_value->value);
            break;
        default:
            fprintf(stderr, "Semantic Analyzer: `%s` isn't a valid type",
                    expression->value.Literal.literal_value->value);
            semantic_analyzer_error();
    }

    return 0;
}

int
semantic_analyzer_check_expression_function_call(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                                 uintptr_t body_size, AST *function_declaration) {
    for (int j = 0; j < table->items; j++) {
        char *table_item_name = table->pair_values[j]->key;

        if (strcmp(table_item_name, expression->value.FunctionCall.function_call_name) == 0 &&
            table_item_name[0] != '\0') {
            if (((SemanticAnalyzerSymbol *) table->pair_values[j]->value)->symbol_type ==
                AST_TYPE_FUNCTION_DECLARATION) {
                // Check the arguments
                // TODO: Check the argument types

                AST *function = ((SemanticAnalyzerSymbol *) table->pair_values[j]->value)->symbol_ast;
                if (expression->value.FunctionCall.arguments_size !=
                    function->value.FunctionDeclaration.arguments_size) {
                    fprintf(stderr, "Semantic Analyzer: `%s` invalid number of arguments",
                            expression->value.FunctionCall.function_call_name);
                    semantic_analyzer_error();
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
            default:
                fprintf(stderr, "Semantic Analyzer: Cannot Check Type `%d`", ast_items[i]->type);
//                semantic_analyzer_error();
        }

        UNMapPairValue *pair = malloc(sizeof(UNMapPairValue));
        pair->key = symbol_struct->symbol_name;
        pair->value = symbol_struct;

        unmap_push_back(symbol_map, pair);
    }

    return symbol_map;
}
