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

void semantic_analyzer_check_function_declarations(UNMap *table) {
    for (int i = 0; i < table->items; i++) {
        SemanticAnalyzerDeclarationTable *declaration_table = (SemanticAnalyzerDeclarationTable *) table->pair_values[i]->value;
        if (declaration_table->declaration_type == AST_TYPE_FUNCTION_DECLARATION) {
            // Duplicate Function Names were already checked, now we just have to check the parameters
            // We need to check the parameters
            semantic_analyzer_check_function_declaration_argument(table,
                                                                  declaration_table->declaration_value->value.FunctionDeclaration.arguments,
                                                                  declaration_table->declaration_value->value.FunctionDeclaration.argument_size);
        }
    }
}

void semantic_analyzer_check_function_declaration_argument(UNMap *table, AST **arguments, uintptr_t argument_size) {
    // Duplicate Function Argument Names
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
            // We need to integrate through the map to check if the identifier value is equal to `declaration_name`, if not, we will display an error message
            for (int j = 0; j < table->items; ++j) {
                char *table_item_name = table->pair_values[j]->key;

                if (strcmp(table_item_name, argument_type->value.ValueKeyword.token->value) == 0 &&
                    table_item_name[0] != '\0') {
                    break;
                } else {
                    if ((j + 1) >= table->items) {
                        fprintf(stderr, "Semantic Analyzer: %s doesn't exist",
                                argument_type->value.ValueKeyword.token->value);
                        exit(-3);
                    }
                }
            }
        }

        printf("FUNCTION ARGUMENT: ");
        ast_print(argument_type);
        printf("\n");
    }
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