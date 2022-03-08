//
//  semantic_analyzer.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "semantic_analyzer.h"

void semantic_analyzer_error(SemanticAnalyzer *analyzer, char *message) {
    fprintf(stderr, "%s || No available information\n", message);
    exit(-4);
}

void semantic_analyzer_warning(SemanticAnalyzer *analyzer, char *message) {
    fprintf(stderr, "%s || No available information\n", message);
}

void semantic_analyzer_run_analysis(mxDynamicArray *ast_items) {
    // Create a declaration table
    mxDynamicArray *declarations = mxDynamicArrayCreate(sizeof(SemanticAnalyzerDeclarations *));

    for (int i = 0; i < ast_items->size; i++) {
        AST *declaration_item = mxDynamicArrayGet(ast_items, i);
        mxDynamicArrayAdd(declarations, semantic_analyzer_get_declaration_item(declaration_item, false));
    }

    semantic_analyzer_check_duplicate_declaration(declarations);

    // Create a SemanticAnalyzer table
    SemanticAnalyzer *analyzer = malloc(sizeof(SemanticAnalyzer));
    analyzer->declarations = declarations;
    analyzer->ast_items = ast_items;

    for (int i = 0; i < analyzer->declarations->size; ++i) {
        semantic_analyzer_update_scope(analyzer, i);
        semantic_analyzer_check_scope(analyzer);
    }
}

void semantic_analyzer_check_scope(SemanticAnalyzer *analyzer) {
    switch (analyzer->current_ast_scope_declaration->type) {
        case AST_TYPE_FUNCTION_DECLARATION:
            semantic_analyzer_check_function_declaration(analyzer);
            break;
        case AST_TYPE_STRUCT_OR_UNION_DECLARATION:
            semantic_analyzer_check_struct_declaration(analyzer);
            break;
        default:
            // Show error message
            break;
    }
}

void semantic_analyzer_check_struct_declaration(SemanticAnalyzer *analyzer) {
    // Duplicate Struct Members
    semantic_analyzer_check_duplicate_struct_members(analyzer);

    // Check the members
    for (int i = 0; i < analyzer->current_ast_scope_declaration->value.StructOrUnionDeclaration.members->size; ++i) {
        AST *member = mxDynamicArrayGet(analyzer->current_ast_scope_declaration->value.StructOrUnionDeclaration.members,
                                        i);
        analyzer->current_ast_scope_inner_declaration = member;

        if (member->type == AST_TYPE_FUNCTION_DECLARATION) {
            analyzer->current_ast_scope_body = (AST **) member->value.FunctionDeclaration.body->data;
            analyzer->current_ast_scope_body_size = member->value.FunctionDeclaration.body->size;

            semantic_analyzer_check_function_declaration(analyzer);
        } else if (member->type == AST_TYPE_STRUCT_INITIALIZER) {
            semantic_analyzer_check_struct_initializer(analyzer, member);
        }
    }
}

void semantic_analyzer_check_struct_initializer(SemanticAnalyzer *analyzer, AST *struct_initializer) {
    semantic_analyzer_check_function_declaration_arguments(analyzer, true);
}

void semantic_analyzer_check_duplicate_struct_members(SemanticAnalyzer *analyzer) {
    // Check for duplicate functions and variable definitions
    mxDynamicArray *variable_names = mxDynamicArrayCreate(sizeof(char *));
    mxDynamicArray *function_names = mxDynamicArrayCreate(sizeof(char *));

    for (int i = 0; i < analyzer->current_ast_scope_body_size; ++i) {
        if (analyzer->current_ast_scope_body[i]->type == AST_TYPE_VARIABLE_DEFINITION) {
            char *variable_name = analyzer->current_ast_scope_body[i]->value.VariableDeclaration.identifier;
            mxDynamicArrayAdd(variable_names, variable_name);
        } else if (analyzer->current_ast_scope_body[i]->type == AST_TYPE_FUNCTION_DECLARATION) {
            char *function_name = analyzer->current_ast_scope_body[i]->value.FunctionDeclaration.function_name;
            mxDynamicArrayAdd(function_names, function_name);
        }
    }

    // Check for duplicate variable names
    for (int i = 0; i < variable_names->size; ++i) {
        for (int j = i + 1; j < variable_names->size; ++j) {
            if (strcmp(mxDynamicArrayGet(variable_names, i), mxDynamicArrayGet(variable_names, j)) == 0) {
                semantic_analyzer_error(analyzer, "Duplicate variable name in struct");
            }
        }
    }

    // Check for duplicate function names
    for (int i = 0; i < function_names->size; ++i) {
        for (int j = i + 1; j < function_names->size; ++j) {
            if (strcmp(mxDynamicArrayGet(function_names, i), mxDynamicArrayGet(function_names, j)) == 0) {
                semantic_analyzer_error(analyzer, "Duplicate function name in struct");
            }
        }
    }
}

void semantic_analyzer_check_function_declaration(SemanticAnalyzer *analyzer) {
    semantic_analyzer_check_function_declaration_arguments(analyzer, false);
    semantic_analyzer_check_body(analyzer);
}

void semantic_analyzer_check_function_declaration_arguments(SemanticAnalyzer *analyzer, bool is_struct) {
    int argument_size = 0;
    mxDynamicArray *arguments;

    if (is_struct) {
        argument_size = analyzer->current_ast_scope_inner_declaration->value.StructInitializer.arguments->size;
        arguments = analyzer->current_ast_scope_inner_declaration->value.StructInitializer.arguments;
    } else {
        argument_size = analyzer->current_ast_scope_inner_declaration->value.FunctionDeclaration.arguments->size;
        arguments = analyzer->current_ast_scope_inner_declaration->value.FunctionDeclaration.arguments;
    }

    for (int i = 0; i < argument_size; ++i) {
        AST *argument = mxDynamicArrayGet(arguments, i);

        if (argument->value.FunctionDeclarationArgument.argument_type->type != AST_TYPE_VALUE_KEYWORD) {
            semantic_analyzer_error(analyzer, "Semantic Analyzer: Function declaration argument type is not a keyword");
            return;
        }

        if (argument->value.FunctionDeclarationArgument.argument_type->value.ValueKeyword.token->type == T_IDENTIFIER) {
            semantic_analyzer_check_type_name_exists(analyzer,
                                                     argument->value.FunctionDeclarationArgument.argument_type->value.ValueKeyword.token->value,
                                                     true);
            return;
        }
    }
}

void semantic_analyzer_check_body(SemanticAnalyzer *analyzer) {
    for (int i = 0; i < analyzer->current_ast_scope_body_size; ++i) {
        analyzer->current_ast_scope_body_item_position = i;
        analyzer->current_ast_scope_body_item = analyzer->current_ast_scope_body[i];

        switch (analyzer->current_ast_scope_body_item->type) {
            case AST_TYPE_VARIABLE_DEFINITION:
                semantic_analyzer_check_variable_declaration(analyzer);
                break;
            case AST_TYPE_FUNCTION_CALL:
                semantic_analyzer_check_expression_function_call(analyzer, analyzer->current_ast_scope_body_item);
                break;
            default:
                break;
        }
    }
}

void semantic_analyzer_check_variable_declaration(SemanticAnalyzer *analyzer) {
    if (analyzer->current_ast_scope_body_item->value.VariableDeclaration.type->value.ValueKeyword.token->type ==
        T_IDENTIFIER) {
        semantic_analyzer_check_type_name_exists(analyzer,
                                                 analyzer->current_ast_scope_body_item->value.VariableDeclaration.type->value.ValueKeyword.token->value,
                                                 true);
    }

    int variable_type = analyzer->current_ast_scope_body_item->value.VariableDeclaration.type->value.ValueKeyword.token->type;

    // Check the expression
    int type = semantic_analyzer_check_expression(analyzer,
                                                  analyzer->current_ast_scope_body_item->value.VariableDeclaration.value);

    if (!semantic_analyzer_check_types_valid(variable_type, type)) {
        semantic_analyzer_warning(analyzer, "Semantic Analyzer: Variable type is not valid to expression type");
    }

    // Check if the variable is already declared
    semantic_analyzer_check_duplicate_variable_definitions(analyzer);
}

void semantic_analyzer_check_duplicate_variable_definitions(SemanticAnalyzer *analyzer) {
    for (uintptr_t i = analyzer->current_ast_scope_body_size - 1; i > 0; i--) {
        AST *item = analyzer->current_ast_scope_body[i];
        if (item->type == AST_TYPE_VARIABLE_DEFINITION && item != analyzer->current_ast_scope_body_item) {
            if (strcmp(item->value.VariableDeclaration.identifier,
                       analyzer->current_ast_scope_body_item->value.VariableDeclaration.identifier) == 0) {
                semantic_analyzer_error(analyzer, "Variable already declared");
            }
        }
    }
}

int semantic_analyzer_check_expression(SemanticAnalyzer *analyzer, AST *expression) {
    switch (expression->type) {
        case AST_TYPE_BINARY:
            if (expression->value.Binary.left->type == AST_TYPE_LITERAL) {
                if (expression->value.Binary.left->value.Literal.literal_value->type == T_K_SELF) {
                    return semantic_analyzer_check_expression_self(analyzer, expression);
                }
            }

            return semantic_analyzer_check_expression_binary(analyzer, expression);
        case AST_TYPE_LITERAL:
            return semantic_analyzer_check_expression_literal(analyzer, expression);
        case AST_TYPE_UNARY:
            return semantic_analyzer_check_expression(analyzer, expression->value.Unary.right);
        case AST_TYPE_GROUPING:
            return semantic_analyzer_check_expression(analyzer, expression->value.Grouping.expression);
        case AST_TYPE_FUNCTION_CALL:
            return semantic_analyzer_check_expression_function_call(analyzer, expression);
        default:
            semantic_analyzer_error(analyzer, "Semantic Analyzer: Expression type not supported");
            return -1;
    }
}

int semantic_analyzer_check_expression_binary(SemanticAnalyzer *analyzer, AST *expression) {
    int left_type = semantic_analyzer_check_expression(analyzer, expression->value.Binary.left);
    int right_type = semantic_analyzer_check_expression(analyzer, expression->value.Binary.right);

    if (semantic_analyzer_check_types_valid(left_type, right_type) == false) {
        semantic_analyzer_warning(analyzer, "Semantic Analyzer: Types are not correct");
        return -1;
    }

    return left_type;
}

int semantic_analyzer_check_expression_self(SemanticAnalyzer *analyzer, AST *expression) {
    if (analyzer->current_ast_scope_declaration->type != AST_TYPE_STRUCT_OR_UNION_DECLARATION) {
        semantic_analyzer_error(analyzer, "Semantic Analyzer: Self can only be used in struct declarations");
    }

    for (int i = 0; i < analyzer->current_ast_scope_declaration->value.StructOrUnionDeclaration.members->size; ++i) {
        AST *member = mxDynamicArrayGet(
            analyzer->current_ast_scope_declaration->value.StructOrUnionDeclaration.members, i);

        if (member->type == AST_TYPE_VARIABLE_DEFINITION) {
            if (strcmp(member->value.VariableDeclaration.identifier,
                       expression->value.Binary.right->value.Literal.literal_value->value) == 0) {
                return member->value.VariableDeclaration.type->value.ValueKeyword.token->type;
            }
        }
    }

    semantic_analyzer_error(analyzer, "Semantic Analyzer: Variable not found");
    return -1;
}

int semantic_analyzer_check_expression_literal(SemanticAnalyzer *analyzer, AST *expression) {
    int literal_type = expression->value.Literal.literal_value->type;

    if (literal_type == T_IDENTIFIER) {
        return semantic_analyzer_check_type_name_exists(analyzer, expression->value.Literal.literal_value->value,
                                                        false);
    }

    return literal_type;
}

int semantic_analyzer_check_expression_function_call(SemanticAnalyzer *analyzer, AST *expression) {
    // Check if the function exists
    AST *function_declaration = semantic_analyzer_check_function_exists(analyzer,
                                                                        expression->value.FunctionCall.function_call_name);

    // Check the arguments
    if (function_declaration->value.FunctionDeclaration.arguments->size !=
        expression->value.FunctionCall.arguments->size) {
        semantic_analyzer_error(analyzer, "Invalid Number of Arguments");
        return -1;
    }

    // Check the argument type
    for (int i = 0; i < expression->value.FunctionCall.arguments->size; ++i) {
        AST *argument_first = mxDynamicArrayGet(function_declaration->value.FunctionDeclaration.arguments, i);
        AST *argument_second = mxDynamicArrayGet(expression->value.FunctionCall.arguments, i);

        int type1 = argument_first->value.FunctionDeclarationArgument.argument_type->value.ValueKeyword.token->type;
        int type2 = semantic_analyzer_check_expression(analyzer, argument_second);

        if (!semantic_analyzer_check_types_valid(type1, type2)) {
            semantic_analyzer_error(analyzer, "Function call invalid types");
            return -1;
        }
    }


    return function_declaration->value.FunctionDeclaration.return_type->value.ValueKeyword.token->type;
}

AST *semantic_analyzer_check_function_exists(SemanticAnalyzer *analyzer, char *identifier) {
    for (int i = 0; i < analyzer->declarations->size; ++i) {
        SemanticAnalyzerDeclarations *declarations = (SemanticAnalyzerDeclarations *) mxDynamicArrayGet(
            analyzer->declarations, i);
        if (declarations->symbol_type == AST_TYPE_FUNCTION_DECLARATION) {
            if (strcmp(declarations->symbol_name, identifier) == 0) {
                return declarations->symbol_ast;
            }
        }
    }

    semantic_analyzer_error(analyzer, "Semantic Analyzer: Function not found");
    return NULL;
}

int semantic_analyzer_check_type_name_exists(SemanticAnalyzer *analyzer, char *type_name, bool is_value_keyword) {
    if (!is_value_keyword) {
        for (int i = (int) analyzer->current_ast_scope_body_item_position; i > 0; i--) {
            if (analyzer->current_ast_scope_body[i - 1]->type == AST_TYPE_VARIABLE_DEFINITION) {
                if (strcmp(analyzer->current_ast_scope_body[i - 1]->value.VariableDeclaration.identifier, type_name) ==
                    0) {
                    return analyzer->current_ast_scope_body[i -
                                                            1]->value.VariableDeclaration.type->value.ValueKeyword.token->type;
                }
            }
        }

        if (analyzer->current_ast_scope_declaration->type == AST_TYPE_FUNCTION_DECLARATION) {
            for (int i = 0;
                 i < analyzer->current_ast_scope_declaration->value.FunctionDeclaration.arguments->size; ++i) {
                AST *argument = mxDynamicArrayGet(
                    analyzer->current_ast_scope_declaration->value.FunctionDeclaration.arguments,
                    i);
                if (strcmp(argument->value.FunctionDeclarationArgument.argument_name, type_name) == 0) {
                    return argument->value.FunctionDeclarationArgument.argument_type->value.ValueKeyword.token->type;
                }
            }
        }

        if (analyzer->current_ast_scope_declaration->type == AST_TYPE_STRUCT_OR_UNION_DECLARATION) {
            // Struct members
            for (int i = 0;
                 i < analyzer->current_ast_scope_declaration->value.StructOrUnionDeclaration.members->size; ++i) {
                if (((AST *) analyzer->current_ast_scope_declaration->value.StructOrUnionDeclaration.members->data[i])->type ==
                    AST_TYPE_VARIABLE_DEFINITION) {
                    if (strcmp(
                        ((AST *) analyzer->current_ast_scope_declaration->value.StructOrUnionDeclaration.members->data[i])->value.VariableDeclaration.identifier,
                        type_name) == 0) {
                        return ((AST *) analyzer->current_ast_scope_declaration->value.StructOrUnionDeclaration.members->data[i])->value.VariableDeclaration.type->value.ValueKeyword.token->type;
                    }
                }
            }
        }
    }

    for (int i = 0; i < analyzer->declarations->size; i++) {
        SemanticAnalyzerDeclarations *declaration_item = mxDynamicArrayGet(analyzer->declarations, i);
        if (strcmp(declaration_item->symbol_name, type_name) == 0) {
            if (declaration_item->symbol_type == AST_TYPE_FUNCTION_DECLARATION) {
                semantic_analyzer_error(analyzer, "Cannot use function as type");
            }
            // TODO: Create a new function to convert ast type into token type
            return declaration_item->symbol_ast->type;
        }
    }

    semantic_analyzer_error(analyzer, "Type not found");
    return -1;
}

void semantic_analyzer_check_duplicate_declaration(mxDynamicArray *declarations) {
    for (int i = 0; i < declarations->size; i++) {
        SemanticAnalyzerDeclarations *declaration = mxDynamicArrayGet(declarations, i);
        for (int j = i + 1; j < declarations->size; j++) {
            SemanticAnalyzerDeclarations *other_declaration = mxDynamicArrayGet(declarations, j);
            if (strcmp(declaration->symbol_name, other_declaration->symbol_name) == 0) {
                semantic_analyzer_error(NULL, "Found Duplicate declaration!");
            }
        }
    }
}

bool semantic_analyzer_check_types_valid(int type1, int type2) {
    if ((type1 == T_INT || type1 == T_K_INT || type1 == T_FLOAT || type1 == T_K_FLOAT) &&
        (type2 == T_INT || type2 == T_K_INT || type2 == T_FLOAT || type2 == T_K_FLOAT)) {
        return true;
    } else if ((type1 == T_CHAR || type1 == T_K_CHAR || type1 == T_STRING || type1 == T_K_STRING) &&
               (type2 == T_CHAR || type2 == T_K_CHAR || type2 == T_STRING || type2 == T_K_STRING)) {
        return true;
    } else {
        return false;
    }
}

void semantic_analyzer_update_scope(SemanticAnalyzer *analyzer, uintptr_t position) {
    SemanticAnalyzerDeclarations *declaration = mxDynamicArrayGet(analyzer->declarations, (int) position);
    analyzer->current_ast_scope_declaration = declaration->symbol_ast;
    analyzer->current_ast_scope_inner_declaration = declaration->symbol_ast;
    analyzer->current_declaration_name = declaration->symbol_name;

    switch (declaration->symbol_type) {
        case AST_TYPE_FUNCTION_DECLARATION:
            analyzer->current_ast_scope_body = (AST **) declaration->symbol_ast->value.FunctionDeclaration.body->data;
            analyzer->current_ast_scope_body_size = declaration->symbol_ast->value.FunctionDeclaration.body->size;
            break;
        case AST_TYPE_STRUCT_OR_UNION_DECLARATION:
            analyzer->current_ast_scope_body = (AST **) declaration->symbol_ast->value.StructOrUnionDeclaration.members->data;
            analyzer->current_ast_scope_body_size = declaration->symbol_ast->value.StructOrUnionDeclaration.members->size;
            break;
        case AST_TYPE_ENUM_DECLARATION:
            analyzer->current_ast_scope_body = (AST **) declaration->symbol_ast->value.EnumDeclaration.cases->data;
            analyzer->current_ast_scope_body_size = declaration->symbol_ast->value.EnumDeclaration.cases->size;
            break;
        default:
            semantic_analyzer_error(NULL, "Unknown declaration type");
            break;
    }
}

SemanticAnalyzerDeclarations *semantic_analyzer_get_declaration_item(AST *declaration_item, bool is_struct) {
    SemanticAnalyzerDeclarations *declaration = malloc(sizeof(SemanticAnalyzerDeclarations));

    switch (declaration_item->type) {
        case AST_TYPE_FUNCTION_DECLARATION:
            declaration->symbol_name = declaration_item->value.FunctionDeclaration.function_name;
            declaration->symbol_ast = declaration_item;
            declaration->symbol_type = declaration_item->type;
            break;
        case AST_TYPE_STRUCT_OR_UNION_DECLARATION:
            declaration->symbol_name = declaration_item->value.StructOrUnionDeclaration.name;
            declaration->symbol_ast = declaration_item;
            declaration->symbol_type = declaration_item->type;
            break;
        case AST_TYPE_ENUM_DECLARATION:
            declaration->symbol_name = declaration_item->value.EnumDeclaration.enum_name;
            declaration->symbol_ast = declaration_item;
            declaration->symbol_type = declaration_item->type;
            break;
        case AST_TYPE_VARIABLE_DEFINITION:
            if (is_struct) {
                declaration->symbol_name = declaration_item->value.VariableDeclaration.identifier;
                declaration->symbol_ast = declaration_item;
                declaration->symbol_type = declaration_item->type;
                break;
            }
        default:
            semantic_analyzer_error(NULL, "Unknown declaration type");
            break;
    }

    return declaration;
}
