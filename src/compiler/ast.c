//
//  ast.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "ast.h"

AST *ast_init(void) {
    AST *ast = calloc(1, sizeof(AST));

    return ast;
}

AST *ast_init_with_type(ASTType type) {
    AST *ast = ast_init();
    ast->type = type;

    return ast;
}