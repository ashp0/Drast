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
} SemanticAnalyzerSymbol;

void semantic_analyzer_run_analysis(AST **ast_items, uintptr_t ast_items_size);

void semantic_analyzer_check_struct_or_union_declaration(UNMap *table, SemanticAnalyzerSymbol *symbol_struct);

void semantic_analyzer_check_function_declaration(UNMap *table, AST *function_declaration, bool is_struct_member,
                                                  AST *struct_declaration);

void semantic_analyzer_check_function_declaration_argument(UNMap *table,
                                                           __attribute__((unused)) AST *function_declaration_ast,
                                                           AST **arguments, uintptr_t argument_size,
                                                           __attribute__((unused)) bool is_struct_member);

void
semantic_analyzer_check_function_declaration_body(UNMap *table, AST *function_declaration_ast, bool is_struct_member,
                                                  AST *struct_declaration);

void
semantic_analyzer_check_variable_definition(UNMap *symbol_table, AST *variable_ast, AST **body, uintptr_t body_size,
                                            AST *function_declaration_ast, bool is_struct_member,
                                            AST *struct_declaration);

int semantic_analyzer_check_expression(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                       uintptr_t body_size, AST *function_declaration, bool is_struct_member,
                                       AST *struct_declaration);

int semantic_analyzer_check_expression_grouping(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                                uintptr_t body_size, AST *function_declaration, bool is_struct_member,
                                                AST *struct_declaration);

int semantic_analyzer_check_expression_binary(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                              uintptr_t body_size, AST *function_declaration, bool is_struct_member,
                                              AST *struct_declaration);

int semantic_analyzer_check_expression_literal(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                               __attribute__((unused)) uintptr_t body_size, AST *function_declaration,
                                               bool is_struct_member,
                                               __attribute__((unused)) AST *struct_declaration);

int
semantic_analyzer_check_expression_function_call(UNMap *table, AST *expression,
                                                 __attribute__((unused)) int position_inside_body,
                                                 __attribute__((unused)) AST **body,
                                                 __attribute__((unused)) uintptr_t body_size,
                                                 __attribute__((unused)) AST *function_declaration,
                                                 __attribute__((unused)) AST *struct_declaration);

int semantic_analyzer_check_struct_self(UNMap *table, AST *expression, int position_inside_body, AST **body,
                                        uintptr_t body_size, AST *function_declaration, bool is_struct_member,
                                        AST *struct_declaration);

void semantic_analyzer_check_if_type_exists(UNMap *symbol_table, char *type_name);

bool semantic_analyzer_types_are_allowed(int type1, int type2);

void semantic_analyzer_compare_types(AST *type1, AST *type2);

void semantic_analyzer_check_duplicate_symbols(UNMap *table);

UNMap *semantic_analyzer_create_symbol_table(AST **ast_items, uintptr_t ast_items_size);