//
//  semantic_analyzer.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "ast.h"
#include <stdlib.h>

#pragma once

typedef struct {
    char *symbol_name;
    ASTType symbol_type;
    AST *symbol_ast;
} SemanticAnalyzerDeclarations;

typedef struct {
    AST **body;
    uintptr_t body_size;
} SemanticAnalyzerInnerBodies;

typedef struct {
    mxDynamicArray *declarations;
    mxDynamicArray *ast_items;

    // The current scope, function body, struct body etc.
    AST **current_ast_scope_body;
    uintptr_t current_ast_scope_body_size;
    AST *current_ast_scope_body_item;
    uintptr_t current_ast_scope_body_item_position;
    AST *current_ast_scope_inner_declaration; // If the current scope is a struct, this is the inner declaration, which will be a function.
    AST *current_ast_scope_declaration; // Function Declaration, Struct Declaration, etc.

    // Inner Statements: for, if, while, else, else if, switch, etc.
    // If Statement, For Statement, etc.
    mxDynamicArray *inner_ast_scope_bodies; // Array of `SemanticAnalyzerInnerBodies`
    int inner_ast_scope_body_index;
    bool is_inner_body;

    uintptr_t inner_ast_scope_declaration; // The declaration of the if statement or the for statement.

    // To show error messages, eg: "Undeclared variable in scope: _____"
    char *current_declaration_name; // For debugging purposes
} SemanticAnalyzer;

void semantic_analyzer_run_analysis(mxDynamicArray *ast_items);

void semantic_analyzer_check_scope(SemanticAnalyzer *analyzer);

void semantic_analyzer_check_struct_declaration(SemanticAnalyzer *analyzer);

void semantic_analyzer_check_duplicate_struct_members(SemanticAnalyzer *analyzer);

void semantic_analyzer_check_struct_initializer(SemanticAnalyzer *analyzer, AST *struct_initializer);

void semantic_analyzer_check_function_declaration(SemanticAnalyzer *analyzer);

void semantic_analyzer_check_function_declaration_arguments(SemanticAnalyzer *analyzer, bool is_struct);

void semantic_analyzer_check_body(SemanticAnalyzer *analyzer);

void semantic_analyzer_check_inner_body(SemanticAnalyzer *analyzer, AST **body, uintptr_t body_size);

void semantic_analyzer_check_variable_declaration(SemanticAnalyzer *analyzer);

void semantic_analyzer_check_if_else_statement(SemanticAnalyzer *analyzer);

void semantic_analyzer_check_return(SemanticAnalyzer *analyzer);

void semantic_analyzer_check_duplicate_variable_definitions(SemanticAnalyzer *analyzer);

int semantic_analyzer_check_expression(SemanticAnalyzer *analyzer, AST *expression);

int semantic_analyzer_check_expression_binary(SemanticAnalyzer *analyzer, AST *expression, bool is_equality);

int semantic_analyzer_check_expression_self(SemanticAnalyzer *analyzer, AST *expression);

int semantic_analyzer_check_expression_literal(SemanticAnalyzer *analyzer, AST *expression);

int semantic_analyzer_check_expression_function_call(SemanticAnalyzer *analyzer, AST *expression);

AST *semantic_analyzer_check_function_exists(SemanticAnalyzer *analyzer, char *identifier);

int semantic_analyzer_check_type_name_exists(SemanticAnalyzer *analyzer, char *type_name, bool is_value_keyword);

void semantic_analyzer_check_duplicate_declaration(mxDynamicArray *declarations);

bool semantic_analyzer_check_types_valid(int type1, int type2);

void semantic_analyzer_update_scope(SemanticAnalyzer *analyzer, uintptr_t position);

SemanticAnalyzerDeclarations *semantic_analyzer_get_declaration_item(AST *declaration_item, bool is_struct);
