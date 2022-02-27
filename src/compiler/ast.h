//
//  ast.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef __DRAST_COMPILER_AST_H__
#define __DRAST_COMPILER_AST_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "token.h"

typedef enum {
    AST_TYPE_IMPORT,

    AST_TYPE_FUNCTION_DECLARATION,
    AST_TYPE_FUNCTION_ARGUMENT,

    AST_TYPE_FUNCTION_CALL,

    AST_TYPE_VARIABLE_DEFINITION,
    AST_TYPE_VARIABLE_CALL,

    AST_TYPE_VALUE_KEYWORD,

    AST_TYPE_STRUCT_OR_UNION_DECLARATION,
    AST_TYPE_STRUCT_OR_UNION_MEMBER_CALL,
    AST_TYPE_STRUCT_INITIALIZER,

    AST_TYPE_ENUM_DECLARATION,
    AST_TYPE_ENUM_ITEM,

    AST_TYPE_BINARY,
    AST_TYPE_LITERAL,
    AST_TYPE_GROUPING,
    AST_TYPE_NOT,

    AST_TYPE_RETURN,

    AST_TYPE_IF_ELSE_STATEMENT,

    AST_TYPE_WHILE_STATEMENT,
    AST_TYPE_FOR_LOOP,

    AST_TYPE_INLINE_ASSEMBLY,

    AST_TYPE_SWITCH_STATEMENT,
    AST_TYPE_SWITCH_CASE,

    AST_TYPE_DO_CATCH,
    AST_TYPE_TRY_STATEMENT,

    AST_TYPE_BODY,

    AST_TYPE_ALIAS,
} ASTType;

typedef union {
    struct {
        char *file;
        bool is_library;
    } Import;

    struct {
        char *identifier;
        bool is_initialized;
        bool is_volatile;
        bool is_private;
        struct AST *value;
        struct AST *type;
    } VariableDeclaration;

    struct {
        Token *token;
        bool is_array;
        bool is_optional;
        bool is_pointer;
    } ValueKeyword;

    struct {
        char *function_name;

        struct AST *return_type;

        bool is_private;
        bool is_struct_initializer;

        struct AST **arguments;
        uintptr_t argument_size;

        struct AST *body;
    } FunctionDeclaration;

    struct {
        char *argument_name;
        struct AST *argument_type;
    } FunctionDeclarationArgument;

    struct {
        char *function_call_name;
        struct AST **arguments; // Literal Values
        uintptr_t arguments_size;
    } FunctionCall;

    struct {
        char *name;
        bool is_union;

        struct AST **members;
        uintptr_t member_size;

        bool is_private;
    } StructOrUnionDeclaration;

    struct {
        struct AST *struct_or_variable_name;
        struct AST *expression;
    } StructOrUnionMemberCall;

    struct {
        struct AST *function_call;
    } StructInitializer;

    struct {
        char *enum_name;
        struct AST **cases;
        uintptr_t case_size;
        bool is_private;
    } EnumDeclaration;

    struct {
        char *case_name;
        int case_value;
    } EnumItem;

    //
    struct {
        struct AST *left;
        Token *operator;
        struct AST *right;
        char *from;
    } Binary;

    struct {
        Token *literal_value;
    } Literal;

    struct {
        struct AST *expression;
    } Grouping;

    struct {
        struct AST *expression;
    } Not;

    struct {
        struct AST *return_expression;
    } Return;

    struct {
        char *variable_name;
        struct AST *expression;
        bool is_expression;
        Token *operator;


        bool is_cast;
        struct AST *cast_value;
    } VariableCall;

    struct {
        struct AST *expression;
        bool is_else_if_statement;
        bool is_else_statement;

        struct AST *body;
    } IfElseStatement;

    struct {
        struct AST *expression;
        struct AST *body;
    } WhileStatement;

    struct {
        struct AST *variable;
        struct AST *condition;
        struct AST *condition2;

        struct AST **body;
        uintptr_t body_size;
    } ForLoop;

    struct {
        char **instruction;
        uintptr_t instructions_size;
    } InlineAssembly;

    struct {
        struct AST *expression;

        struct AST **switch_cases;
        uintptr_t switch_cases_size;

        bool is_matches_statement;
    } SwitchStatement;

    struct {
        bool is_default;

        struct AST *expression;
        struct AST **body;
        uintptr_t body_size;
    } SwitchCase;

    struct {
        // Do Statement
        struct AST **do_body;
        uintptr_t do_body_size;

        // Catch Statement
        struct AST **second_body;
        uintptr_t second_body_size;

        bool is_while_statement;
        struct AST *while_statement_expression;
    } DoCatchOrWhileStatement;

    struct {
        struct AST *expression;
    } TryStatement;

    struct {
        struct AST **body;
        uintptr_t body_size;
    } Body;

    struct {
        char *alias_name;
        struct AST *alias_value;
    } Alias;
} ASTValue;

typedef struct AST {
    ASTType type;
    ASTValue value;
} AST;

AST *ast_init(void);

AST *ast_init_with_type(ASTType type);

#endif // __DRAST_COMPILER_AST_H__
