//
//  ast.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef DRAST_AST_H
#define DRAST_AST_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "token.h"
#include "../utils/mxDynamicArray.h"
#include "../utils/mxBitmap.h"
#include "../utils/mxHashmap.h"

typedef enum {
    AST_TYPE_COMPOUND_STATEMENT, // { ... }

    AST_TYPE_IMPORT, // import io

    AST_TYPE_FUNCTION_DECLARATION, // int :: test(int a, int b) { ... }
    AST_TYPE_FUNCTION_ARGUMENT, // int a, int b
    AST_TYPE_FUNCTION_CALL, // test(1, 2)

    AST_TYPE_TYPE, // int, string, float, bool, etc.

    AST_TYPE_STRUCT_DECLARATION, // struct Test { ... }
    AST_TYPE_STRUCT_INITIALIZER_CALL, // .init(1, 2)

    AST_TYPE_ENUM_DECLARATION, // enum Test { ... }
    AST_TYPE_ENUM_CASE, // case A = 50, B = 100 etc.

    AST_TYPE_VARIABLE_DECLARATION, // int a = 1

    AST_TYPE_WHILE, // while (a == 1) { ... }
    AST_TYPE_FOR, // for (int i = 0; i < 10; i++) { ... }

    AST_TYPE_SWITCH, // switch (a) { case 1: ... }
    AST_TYPE_SWITCH_CASE, // case 1: ...

    AST_TYPE_DO, // do { ... }
    AST_TYPE_CATCH, // catch (...) { ... }
    AST_TYPE_TRY, // try myVariable = myFunction()


    AST_TYPE_RETURN, // return 1
    AST_TYPE_IF, // if (a == 1) { ... } else { ... }
    AST_TYPE_ASM, // asm("mov rax, 1")
    AST_TYPE_ALIAS, // alias Test = int

    AST_TYPE_BINARY_EXPRESSION, // 5 + 6;
    AST_TYPE_UNARY_EXPRESSION, // -5;
    AST_TYPE_GROUPING_EXPRESSION, // (5 + 6)
    AST_TYPE_LITERAL, // 5;
    AST_TYPE_CAST, // cast(5.50, int);
} ASTType;

typedef union {
    struct {
        mxDynamicArray *statements;
        mxHashmap *declarations; // declaration.collided, for duplicate declarations
    } CompoundStatement;

    struct {
        Token *import;
        // Just check the token type, if it's a string, then it's a file import
        // if it's a keyword, then it's a library import
        // No need to make a new variable for this
    } Import;

    struct {
        mxBitmap *modifiers;
        struct AST *return_type; // AST_TYPE_TYPE
        Token *name;
        mxDynamicArray *arguments; // struct AST *
        struct AST *body; // AST_TYPE_COMPOUND_STATEMENT
        bool is_extern_or_header; // C functions, Assembly functions, etc.
    } FunctionDeclaration;

    struct {
        struct AST *type; // AST_TYPE_TYPE
        Token *name;
        bool is_vaarg;
    } FunctionArgument;

    struct {
        Token *name;
        mxDynamicArray *arguments; // struct AST *
    } FunctionCall;

    struct {
        Token *token_type;
        bool is_optional;
        bool is_pointer;
        bool is_array;
    } Type;

    struct {
        Token *name;
        mxDynamicArray *members; // struct AST *
    } StructDeclaration;

    struct {
        mxDynamicArray *arguments; // struct AST *
    } StructInitializerCall;

    struct {
        Token *name;
        mxDynamicArray *cases; // struct AST *
    } EnumDeclaration;

    struct {
        Token *name;
        int index;
    } EnumCase;

    struct {
        mxBitmap *modifiers;
        struct AST *type; // AST_TYPE_TYPE
        Token *name;
        struct AST *initializer; // AST_TYPE_LITERAL
        bool is_initialized;
    } VariableDeclaration;

    struct {
        struct AST *condition; // AST_TYPE_BINARY_EXPRESSION
        struct AST *body; // AST_TYPE_COMPOUND_STATEMENT
    } While;

    struct {
        struct AST *condition; // AST_TYPE_BINARY_EXPRESSION
        struct AST *condition1; // AST_TYPE_BINARY_EXPRESSION
        struct AST *variable_declaration; // AST_TYPE_VARIABLE_DECLARATION
        struct AST *body; // AST_TYPE_COMPOUND_STATEMENT
    } For;

    struct {
        struct AST *condition; // AST_TYPE_BINARY_EXPRESSION
        mxDynamicArray *cases; // struct AST *
    } Switch;

    struct {
        struct AST *condition; // AST_TYPE_BINARY_EXPRESSION
        struct AST *body; // AST_TYPE_COMPOUND_STATEMENT
    } SwitchCase;

    // TODO:
    struct {
        struct AST *body; // AST_TYPE_COMPOUND_STATEMENT
    } Do;

    struct {
        struct AST *body; // AST_TYPE_COMPOUND_STATEMENT
    } Catch;

    struct {
        struct AST *expression; // AST_TYPE_COMPOUND_STATEMENT, should be function call?
    } Try;
    // @end todo

    struct {
        struct AST *expression; // AST_TYPE_LITERAL
        bool has_return;
        // Return call also be empty
    } Return;

    struct {
        struct AST *if_condition; // AST_TYPE_BINARY_EXPRESSION
        struct AST *if_body; // AST_TYPE_COMPOUND_STATEMENT

        mxDynamicArray *else_if_conditions; // AST_TYPE_BINARY_EXPRESSION
        mxDynamicArray *else_if_bodies; // AST_TYPE_COMPOUND_STATEMENT

        struct AST *else_body; // AST_TYPE_COMPOUND_STATEMENT
        bool has_else;
    } If;

    struct {
        struct AST *asm_string; // AST_TYPE_LITERAL
    } Asm;

    struct {
        Token *name;
        struct AST *type; // AST_TYPE_TYPE
    } Alias;

    struct {
        struct AST *left; // AST_TYPE_VARIABLE_DECLARATION
        Token *operator;
        struct AST *right; // AST_TYPE_LITERAL
    } BinaryExpression;

    struct {
        Token *operator;
        struct AST *right; // AST_TYPE_LITERAL
    } UnaryExpression;

    struct {
        struct AST *expression; // AST_TYPE_COMPOUND_STATEMENT
    } Grouping;

    struct {
        Token *token;
    } Literal;

    struct {
        struct AST *to_type; // AST_TYPE_TYPE
        struct AST *expression; // AST_TYPE_COMPOUND_STATEMENT
    } Cast;
} ASTValue;

typedef struct AST {
    ASTType type;
    ASTValue value;
} AST;

#endif /* DRAST_AST_H */