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
#include "token.h"

typedef enum {
    AST_TYPE_IMPORT,

    AST_TYPE_FUNCTION_DECLARATION,
    AST_TYPE_FUNCTION_ARGUMENT,
    AST_TYPE_FUNCTION_CALL,

    AST_TYPE_LET_DEFINITION,
    AST_TYPE_VARIABLE_DEFINITION,

    AST_TYPE_VALUE_KEYWORD,

    AST_TYPE_STRUCT_DECLARATION,
    AST_TYPE_STRUCT_ITEM,

    AST_TYPE_ENUM_DECLARATION,
    AST_TYPE_ENUM_ITEM,
} ASTType;

typedef union {
    struct {
        char *file;
        bool is_library;
    } Import;

    struct {
        char *identifier;
        bool is_constant;
        bool is_initialized;
        bool is_volatile;
        struct AST *value;
    } Variable;

    struct {
        Token *token;
        bool is_array;
    } ValueKeyword;

    struct {
        char *function_name;
        struct AST *return_type;

        bool is_private;

        struct AST **arguments;
        uintptr_t argument_size;

        struct AST **body;
        uintptr_t body_size;
    } FunctionDeclaration;

    struct {
        char *argument_name;
        struct AST *argument_type;
    } FunctionArgument;
} ASTValue;

typedef struct AST {
    ASTType type;
    ASTValue value;
} AST;

AST *ast_init(void);

AST *ast_init_with_type(ASTType type);

AST *ast_init_with_type_and_value(ASTType type, ASTValue value);

#endif // __DRAST_COMPILER_AST_H__
