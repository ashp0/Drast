//
//  semantic_analyzer.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "semantic_analyzer.h"

void semantic_analyzer_run_analysis(UNMap *table) {
    semantic_analyzer_check_for_duplicate_declarations(table);
    semantic_analyzer_check_function_declarations(table);
    semantic_analyzer_check_struct_declarations(table);
}

void semantic_analyzer_check_for_duplicate_declarations(UNMap *table) {
    for (int i = 0; i < table->items - 1; i++) {
        for (int j = i + 1; j < table->items; j++) {
            if (table->pair_values[i]->key != NULL) {
                if (strcmp(table->pair_values[i]->key, table->pair_values[j]->key) == 0 &&
                    table->pair_values[i]->key[0] != '\0') {
                    fprintf(stderr, "Semantic Analyzer: Found Duplicate Member: `%s`", table->pair_values[i]->key);
                    exit(-3);
                }
            }
        }
    }
}

void semantic_analyzer_check_struct_declarations(UNMap *table) {
    for (int i = 0; i < table->items; ++i) {
        SemanticAnalyzerDeclarationTable *declaration_table = (SemanticAnalyzerDeclarationTable *) table->pair_values[i]->value;
        if (declaration_table->declaration_type == AST_TYPE_STRUCT_OR_UNION_DECLARATION) {
            printf("FOUND STRUCT\n");
            // Create a new struct table

            SemanticAnalyzerASTItems *ast_items = calloc(1, sizeof(SemanticAnalyzerASTItems));

            for (int j = 0; j < declaration_table->declaration_value->value.StructOrUnionDeclaration.member_size; ++j) {
                ast_items->item_size += 1;
                ast_items->items = realloc(ast_items->items, ast_items->item_size * sizeof(AST *));

                ast_items->items[ast_items->item_size -
                                 1] = declaration_table->declaration_value->value.StructOrUnionDeclaration.members[j];
            }

            UNMap *map = semantic_analyzer_create_declaration_table(ast_items);

            for (int l = 0; l < map->items; ++l) {
                SemanticAnalyzerDeclarationTable *declaration_table2 = (SemanticAnalyzerDeclarationTable *) map->pair_values[l]->value;
                switch (declaration_table2->declaration_type) {
                    case AST_TYPE_VARIABLE_DEFINITION:
                        semantic_analyzer_check_variable_definition(map,
                                                                    declaration_table->declaration_value->value.StructOrUnionDeclaration.members,
                                                                    declaration_table->declaration_value->value.StructOrUnionDeclaration.member_size,
                                                                    declaration_table->declaration_value,
                                                                    declaration_table->declaration_value->value.StructOrUnionDeclaration.members[l]);
                        break;
                    case AST_TYPE_FUNCTION_DECLARATION:
                        break;
                    default:
                        printf("Semantic Analyzer: Struct cannot be checked\n");
                        exit(-4);
                }
            }

            semantic_analyzer_run_analysis(map);
        }
    }
}

void semantic_analyzer_check_function_declarations(UNMap *table) {
    for (int i = 0; i < table->items; i++) {
        SemanticAnalyzerDeclarationTable *declaration_table = (SemanticAnalyzerDeclarationTable *) table->pair_values[i]->value;
        if (declaration_table->declaration_type == AST_TYPE_FUNCTION_DECLARATION) {
            // Duplicate Function Names were already checked, now we just have to check the parameters
            // We need to check the parameters
            semantic_analyzer_check_function_declaration_argument(table,
                                                                  declaration_table->declaration_value->value.FunctionDeclaration.arguments,
                                                                  declaration_table->declaration_value->value.FunctionDeclaration.argument_size);
            semantic_analyzer_check_function_declaration_body(table, declaration_table->declaration_value);
        }
    }
}

void semantic_analyzer_check_function_declaration_argument(UNMap *table, AST **arguments, uintptr_t argument_size) {
    // Duplicate Function Argument Names
    if (argument_size == 0)
        return;
    for (int i = 0; i < argument_size - 1; i++) {
        char *argument_i = arguments[i]->value.FunctionDeclarationArgument.argument_name;
        for (int j = i + 1; j < argument_size; j++) {
            char *argument_j = arguments[j]->value.FunctionDeclarationArgument.argument_name;
            if (strcmp(argument_i, argument_j) == 0) {
                fprintf(stderr, "Semantic Analyzer: Found Duplicate Function Parameter: `%s`", argument_i);
                exit(-3);
            }
        }
    }

    // If the function type exits
    for (int i = 0; i < argument_size; ++i) {
        AST *argument_type = arguments[i]->value.FunctionDeclarationArgument.argument_type;
        if (argument_type->type != AST_TYPE_VALUE_KEYWORD) {
            fprintf(stderr, "Semantic Analyzer: Function Arguments must be a type %d", argument_type->type);
            exit(-3);
        }

        if (argument_type->value.ValueKeyword.token->type == T_IDENTIFIER) {
            semantic_analyzer_check_if_identifier_is_valid_type(table, argument_type->value.ValueKeyword.token->value,
                                                                true);
        }
    }
}

void semantic_analyzer_check_function_declaration_body(UNMap *table, AST *function_declaration) {
    for (int i = 0; i < function_declaration->value.FunctionDeclaration.body->value.Body.body_size; ++i) {
        switch (function_declaration->value.FunctionDeclaration.body->value.Body.body[i]->type) {
            case AST_TYPE_VARIABLE_DEFINITION:
                semantic_analyzer_check_variable_definition(table,
                                                            function_declaration->value.FunctionDeclaration.body->value.Body.body,
                                                            function_declaration->value.FunctionDeclaration.body->value.Body.body_size,
                                                            function_declaration,
                                                            function_declaration->value.FunctionDeclaration.body->value.Body.body[i]);
                break;
            default:
                printf("Function body cannot be checked by semantic analyzer %d\n",
                       function_declaration->value.FunctionDeclaration.body->value.Body.body[i]->type);
        }
    }
}

void
semantic_analyzer_check_variable_definition(UNMap *table, AST **body, uintptr_t body_size, AST *function_declaration,
                                            AST *variable_definition_ast) {
    // Variable Type
    if (variable_definition_ast->value.VariableDeclaration.type->value.ValueKeyword.token->type == T_IDENTIFIER) {
        semantic_analyzer_check_if_identifier_is_valid_type(table,
                                                            variable_definition_ast->value.VariableDeclaration.type->value.ValueKeyword.token->value,
                                                            true);
    }

    // Check if the variable name is used elsewhere in the body
    int position_inside_body = 0;
    for (int i = 0; i < body_size; ++i) {
        if (body[i]->type == AST_TYPE_VARIABLE_DEFINITION) {
            if (body[i] == variable_definition_ast) {
                position_inside_body = i;
            }
            if ((strcmp(variable_definition_ast->value.VariableDeclaration.identifier,
                        body[i]->value.VariableDeclaration.identifier) == 0) &&
                body[i] != variable_definition_ast) {
                fprintf(stderr, "Semantic Analyzer: Variable `%s`, has been defined more than once\n",
                        variable_definition_ast->value.VariableDeclaration.identifier);
                exit(-3);
            }
        }
    }

    if (variable_definition_ast->value.VariableDeclaration.is_initialized) {
        int variable_type = variable_definition_ast->value.VariableDeclaration.type->value.ValueKeyword.token->type;
        int expression_type = semantic_analyzer_check_expression(table,
                                                                 variable_definition_ast->value.VariableDeclaration.value,
                                                                 position_inside_body, body, body_size,
                                                                 function_declaration);
        if (!semantic_analyzer_types_are_allowed(variable_type, expression_type)) {
            fprintf(stderr, "Semantic Analyzer: Variable `%s`, has a type of `%s`, but was asigned type of `%s`\n",
                    variable_definition_ast->value.VariableDeclaration.identifier, token_print(variable_type),
                    token_print(expression_type));
            exit(-3);
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
            printf("Cannot parse binary %d\n", expression->type);
            break;
    }
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
        fprintf(stderr, "Semantic Analyzer: expressions have different types: %s :: %s\n", token_print(left_type),
                token_print(right_type));
        exit(-3);
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
            for (int j = 0; j < function_declaration->value.FunctionDeclaration.argument_size; j++) {
                AST *argument_name = function_declaration->value.FunctionDeclaration.arguments[j];
                if (strcmp(expression->value.Literal.literal_value->value,
                           argument_name->value.FunctionDeclarationArgument.argument_name) == 0) {
                    return argument_name->value.FunctionDeclarationArgument.argument_type->value.ValueKeyword.token->type;
                } else if (j >= function_declaration->value.FunctionDeclaration.argument_size) {
                    // Error, not a function argument
                }
            }

            semantic_analyzer_check_if_identifier_is_valid_type(table, expression->value.Literal.literal_value->value,
                                                                true);
            break;
        default:
            fprintf(stderr, "Semantic Analyzer: `%s` isn't a valid type\n",
                    expression->value.Literal.literal_value->value);
            exit(-3);
    }
}

int
semantic_analyzer_check_expression_function_call(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                                 uintptr_t body_size, AST *function_declaration) {
    for (int j = 0; j < table->items; j++) {
        char *table_item_name = table->pair_values[j]->key;

        if (strcmp(table_item_name, expression->value.FunctionCall.function_call_name) == 0 &&
            table_item_name[0] != '\0') {
            if (((SemanticAnalyzerDeclarationTable *) table->pair_values[j]->value)->declaration_type ==
                AST_TYPE_FUNCTION_DECLARATION) {
                // Check the arguments
                // TODO: Check the argument types

                AST *function = ((SemanticAnalyzerDeclarationTable *) table->pair_values[j]->value)->declaration_value;
                if (expression->value.FunctionCall.arguments_size !=
                    function->value.FunctionDeclaration.argument_size) {
                    fprintf(stderr, "Semantic Analyzer: `%s` invalid number of arguments",
                            expression->value.FunctionCall.function_call_name);
                    exit(-3);
                }
                return function->value.FunctionDeclaration.return_type->value.ValueKeyword.token->type;
            } else {
                fprintf(stderr, "Semantic Analyzer: `%s` is not a valid function",
                        expression->value.FunctionCall.function_call_name);
                exit(-3);
            }
        } else {
            if ((j + 1) >= table->items) {
                fprintf(stderr, "Semantic Analyzer: `%s` is not a valid function",
                        expression->value.FunctionCall.function_call_name);
                exit(-3);
            }
        }
    }
}

bool semantic_analyzer_check_if_identifier_is_valid_type(UNMap *table, char *identifier, bool displays_error) {
    for (int j = 0; j < table->items; j++) {
        char *table_item_name = table->pair_values[j]->key;

        if (strcmp(table_item_name, identifier) == 0 &&
            table_item_name[0] != '\0') {

            // Check if it's a function definition
            if (((SemanticAnalyzerDeclarationTable *) table->pair_values[j]->value)->declaration_type ==
                AST_TYPE_FUNCTION_DECLARATION) {
                fprintf(stderr, "Semantic Analyzer: `%s` is a function name, which cannot be used as a type",
                        identifier);
                exit(-3);
            }

            return true;
        } else {
            if ((j + 1) >= table->items && displays_error) {
                fprintf(stderr, "Semantic Analyzer: `%s` isn't a valid type", identifier);
                exit(-3);
            }
        }
    }

    return false;
}

UNMap *semantic_analyzer_create_declaration_table(SemanticAnalyzerASTItems *ast_items) {
    UNMap *map = unmap_init();

    for (int i = 0; i < ast_items->item_size; ++i) {
        // Create a pair
        UNMapPairValue *pair = calloc(1, sizeof(UNMapPairValue));
        pair->key = semantic_analyzer_declaration_name(ast_items->items[i]);

        // Create a declaration table
        SemanticAnalyzerDeclarationTable *declaration_table = calloc(1, sizeof(SemanticAnalyzerDeclarationTable));
        declaration_table->declaration_name = pair->key;
        declaration_table->declaration_type = ast_items->items[i]->type;
        declaration_table->declaration_value = ast_items->items[i];

        // Add the item
        pair->value = declaration_table;
        unmap_push_back(map, pair);
    }

    return map;
}

bool semantic_analyzer_types_are_allowed(int type1, int type2) {
    if ((type1 == T_INT || type1 == T_FLOAT || type1 == T_K_INT || type1 == T_K_FLOAT) &&
        (type2 == T_INT || type2 == T_FLOAT || type2 == T_K_INT || type2 == T_K_FLOAT))
        return true;
    else
        return false;
}

char *semantic_analyzer_declaration_name(AST *ast) {
    switch (ast->type) {
        case AST_TYPE_FUNCTION_DECLARATION:
            return ast->value.FunctionDeclaration.function_name;
        case AST_TYPE_STRUCT_OR_UNION_DECLARATION:
            return ast->value.StructOrUnionDeclaration.name;
        case AST_TYPE_ENUM_DECLARATION:
            return ast->value.EnumDeclaration.enum_name;
        case AST_TYPE_ALIAS:
            return ast->value.Alias.alias_name;
        default:
            return "";
    }
}