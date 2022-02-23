//
//  ast_print.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "ast_print.h"

static inline void ast_print_import(AST *ast);

static inline void ast_print_variable(AST *ast);

static inline void ast_print_function_declaration(AST *ast);

static inline void ast_print_function_arguments(AST *ast);

static inline void ast_print_type_name(AST *ast);

static inline void ast_print_struct_declaration(AST *ast);

static inline void ast_print_enum_declaration(AST *ast);

static inline void ast_print_binary(AST *ast);

static inline void ast_print_unary(AST *ast);

static inline void ast_print_literal(AST *ast);

static inline void ast_print_grouping(AST *ast);

static inline void ast_print_type_name(AST *ast);

static inline void ast_print_return(AST *ast);

static inline void ast_print_expressions(AST *ast);

void ast_print(AST *ast) {
    switch (ast->type) {
        case AST_TYPE_IMPORT:
            ast_print_import(ast);
            break;
        case AST_TYPE_VARIABLE_DEFINITION:
        case AST_TYPE_LET_DEFINITION:
            ast_print_variable(ast);
            break;
        case AST_TYPE_FUNCTION_DECLARATION:
            ast_print_function_declaration(ast);
            break;
        case AST_TYPE_STRUCT_DECLARATION:
            ast_print_struct_declaration(ast);
            break;
        case AST_TYPE_ENUM_DECLARATION:
            ast_print_enum_declaration(ast);
            break;
        case AST_TYPE_EXPRESSIONS:
            ast_print_expressions(ast);
            break;
        case AST_TYPE_BINARY:
            ast_print_binary(ast);
            break;
        case AST_TYPE_UNARY:
            ast_print_unary(ast);
            break;
        case AST_TYPE_LITERAL:
            ast_print_literal(ast);
            break;
        case AST_TYPE_GROUPING:
            ast_print_grouping(ast);
            break;
        case AST_TYPE_VALUE_KEYWORD:
            ast_print_type_name(ast);
            break;
        case AST_TYPE_RETURN:
            ast_print_return(ast);
            break;
        default:
            printf("Cannot Print AST %d\n", ast->type);
            break;
    }
}

static inline void ast_print_import(AST *ast) {
//    printf("import %s, is library: %d", ast->value.Import.file, ast->value.Import.is_library);
    printf("import %s\n", ast->value.Import.file);
}

static inline void ast_print_variable(AST *ast) {
    printf("(var/let) %s", ast->value.Variable.identifier);

    if (ast->value.Variable.is_initialized) {
        printf(" = ");
        ast_print(ast->value.Variable.value);
        printf("\n");
    } else {
        printf(": ");
        ast_print_type_name(ast->value.Variable.value);
        printf("\n");
    }
}

static inline void ast_print_function_declaration(AST *ast) {
    printf("func %s(", ast->value.FunctionDeclaration.function_name);
    ast_print_function_arguments(ast);
    if (ast->value.FunctionDeclaration.has_return_type) {
        printf(" -> ");
        ast_print(ast->value.FunctionDeclaration.return_type);
    }
    printf(" {\n");

    for (int i = 0; i < ast->value.FunctionDeclaration.body_size; i++) {
        printf("\t");
        ast_print(ast->value.FunctionDeclaration.body[i]);
    }

    printf("}\n");
}

static inline void ast_print_function_arguments(AST *ast) {
    for (int i = 0; i < ast->value.FunctionDeclaration.argument_size; i++) {
        ast_print_type_name(ast->value.FunctionDeclaration.arguments[i]->value.FunctionArgument.argument_type);
        printf(" %s, ", ast->value.FunctionDeclaration.arguments[i]->value.FunctionArgument.argument_name);
    }

    printf("\b\b)");
}

static inline void ast_print_struct_declaration(AST *ast) {
    printf("struct %s {\n", ast->value.StructDeclaration.struct_name);

    for (int i = 0; i < ast->value.StructDeclaration.member_size; ++i) {
        printf("\t");
        ast_print_variable(ast->value.StructDeclaration.members[i]);
    }

    printf("}\n");
}

static inline void ast_print_enum_declaration(AST *ast) {
    printf("enum %s {\n", ast->value.EnumDeclaration.enum_name);

    for (int i = 0; i < ast->value.EnumDeclaration.case_size; ++i) {
        printf("\tcase %s = %d\n", ast->value.EnumDeclaration.cases[i]->value.EnumItem.case_name,
               ast->value.EnumDeclaration.cases[i]->value.EnumItem.case_value);
    }

    printf("}\n");
}

static inline void ast_print_return(AST *ast) {
    printf("return ");
    ast_print(ast->value.Return.return_expression);
    printf("\n");
}

static inline void ast_print_type_name(AST *ast) {
    if (ast->value.ValueKeyword.is_array) {
        printf("%s[]", token_print(ast->value.ValueKeyword.token->type));
    } else {
        printf("%s", token_print(ast->value.ValueKeyword.token->type));
    }
}

static inline void ast_print_expressions(AST *ast) {
    for (int i = 0; i < ast->value.Expressions.expressions_size; i++) {
        ast_print(ast->value.Expressions.expressions[i]);
    }
}

static inline void ast_print_binary(AST *ast) {
    ast_print(ast->value.Binary.left);
    printf(" %s ", ast->value.Binary.operator->value);
    ast_print(ast->value.Binary.right);
}

static inline void ast_print_unary(AST *ast) {
    printf("ast_print_unary");
}

static inline void ast_print_literal(AST *ast) {
    printf("%s", ast->value.Literal.literal_value->value);
}

static inline void ast_print_grouping(AST *ast) {
    printf("(");
    ast_print(ast->value.Grouping.expression);
    printf(")");
}
