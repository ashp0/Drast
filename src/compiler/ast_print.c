//
//  ast_print.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "ast_print.h"

static inline void ast_print_import(AST *ast);

static inline void ast_print_variable(AST *ast);

void ast_print(AST *ast) {
    switch (ast->type) {
        case AST_TYPE_IMPORT:
            ast_print_import(ast);
            break;
        case AST_TYPE_VARIABLE_DEFINITION:
        case AST_TYPE_LET_DEFINITION:
            ast_print_variable(ast);
            break;
    }
}

static inline void ast_print_import(AST *ast) {
//    printf("import %s, is library: %d", ast->value.Import.file, ast->value.Import.is_library);
    printf("import %s\n", ast->value.Import.file);
}

static inline void ast_print_variable(AST *ast) {
    printf("(var/let) %s\n", ast->value.Variable.identifier);
}