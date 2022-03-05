//
//  ast_print.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "ast.h"

void ast_print(AST *ast);

void ast_print_import(AST *ast, uintptr_t indent);

void ast_print_variable(AST *ast, bool new_line, uintptr_t indent);

void ast_print_function_declaration(AST *ast, uintptr_t indent);

void ast_print_function_arguments(AST *ast, uintptr_t indent);

void ast_print_type_name(AST *ast, uintptr_t indent);

void ast_print_struct_declaration(AST *ast, uintptr_t indent);

void ast_print_enum_declaration(AST *ast, uintptr_t indent);

void ast_print_binary(AST *ast, uintptr_t indent);

void ast_print_literal(AST *ast, uintptr_t indent);

void ast_print_grouping(AST *ast, uintptr_t indent);

void ast_print_return(AST *ast, uintptr_t indent);

void ast_print_function_call(AST *ast, uintptr_t indent);

void ast_print_if_else_statement(AST *ast, uintptr_t indent);

void ast_print_while_statement(AST *ast, uintptr_t indent);

void ast_print_inline_assembly(AST *ast, uintptr_t indent);

void ast_print_switch_statement(AST *ast, uintptr_t indent);

void ast_print_switch_case_statement(AST *ast, uintptr_t indent);

void ast_print_for_loop(AST *ast, uintptr_t indent);

void _ast_print(AST *ast, uintptr_t indent);

void ast_print_do_catch_statement(AST *ast, uintptr_t indent);

void ast_print_try_statement(AST *ast, __attribute__((unused)) uintptr_t indent);

void ast_print_struct_member_call(AST *ast, uintptr_t indent);

void ast_print_body(AST *ast, uintptr_t indent);

void ast_print_unary(AST *ast, uintptr_t indent);

void ast_print_struct_initializer(AST *ast, uintptr_t indent);

void ast_print_alias(AST *ast);