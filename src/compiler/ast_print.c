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
    if (ast->value.Variable.is_initialized) {

    } else {
        printf("(var/let) %s: ", ast->value.Variable.identifier);
        ast_print_type_name(ast->value.Variable.value);
        printf("\n");
    }
}

static inline void ast_print_function_declaration(AST *ast) {
    printf("func %s(", ast->value.FunctionDeclaration.function_name);
    ast_print_function_arguments(ast);
    printf(" -> ");
    printf("%s {\n", token_print(ast->value.FunctionDeclaration.return_type->value.ValueKeyword.token->type));

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

static inline void ast_print_type_name(AST *ast) {
    if (ast->value.ValueKeyword.is_array) {
        printf("%s[]", token_print(ast->value.ValueKeyword.token->type));
    } else {
        printf("%s", token_print(ast->value.ValueKeyword.token->type));
    }
}
