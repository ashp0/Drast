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

static inline void ast_print_return(AST *ast);

static inline void ast_print_function_call(AST *ast);

static inline void ast_print_variable_call(AST *ast);

static inline void ast_print_if_else_statement(AST *ast);

static inline void ast_print_while_statement(AST *ast);

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
        case AST_TYPE_STRUCT_OR_UNION_DECLARATION:
            ast_print_struct_declaration(ast);
            break;
        case AST_TYPE_ENUM_DECLARATION:
            ast_print_enum_declaration(ast);
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
        case AST_TYPE_FUNCTION_CALL:
            ast_print_function_call(ast);
            break;
        case AST_TYPE_VARIABLE_CALL:
            ast_print_variable_call(ast);
            break;
        case AST_TYPE_IF_ELSE_STATEMENT:
            ast_print_if_else_statement(ast);
            break;
        case AST_TYPE_WHILE_STATEMENT:
            ast_print_while_statement(ast);
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
    printf("(var/let) %s", ast->value.VariableDeclaration.identifier);

    if (ast->value.VariableDeclaration.is_initialized) {
        printf(" = ");
        ast_print(ast->value.VariableDeclaration.value);
        printf("\n");
    } else {
        printf(": ");
        ast_print_type_name(ast->value.VariableDeclaration.value);
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

    printf("}\n\n");
}

static inline void ast_print_function_arguments(AST *ast) {
    for (int i = 0; i < ast->value.FunctionDeclaration.argument_size; i++) {
        ast_print_type_name(
                ast->value.FunctionDeclaration.arguments[i]->value.FunctionDeclarationArgument.argument_type);
        printf(" %s, ", ast->value.FunctionDeclaration.arguments[i]->value.FunctionDeclarationArgument.argument_name);
    }

    printf("\b\b)");
}

static inline void ast_print_function_call(AST *ast) {
    printf("%s(", ast->value.FunctionCall.function_call_name);
    for (int i = 0; i < ast->value.FunctionCall.arguments_size; i++) {
        ast_print(ast->value.FunctionCall.arguments[i]);
        printf(", ");
    }
    printf("\b\b)\n");
}

static inline void ast_print_struct_declaration(AST *ast) {
    if (ast->value.StructOrUnionDeclaration.is_union)
        printf("union %s {\n", ast->value.StructOrUnionDeclaration.name);
    else
        printf("struct %s {\n", ast->value.StructOrUnionDeclaration.name);

    for (int i = 0; i < ast->value.StructOrUnionDeclaration.member_size; ++i) {
        printf("\t");
        ast_print_variable(ast->value.StructOrUnionDeclaration.members[i]);
    }

    printf("}\n\n");
}

static inline void ast_print_enum_declaration(AST *ast) {
    printf("enum %s {\n", ast->value.EnumDeclaration.enum_name);

    for (int i = 0; i < ast->value.EnumDeclaration.case_size; ++i) {
        printf("\tcase %s = %d\n", ast->value.EnumDeclaration.cases[i]->value.EnumItem.case_name,
               ast->value.EnumDeclaration.cases[i]->value.EnumItem.case_value);
    }

    printf("}\n\n");
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

    if (ast->value.ValueKeyword.is_optional)
        printf("?");
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

static inline void ast_print_variable_call(AST *ast) {
    printf("%s\n", ast->value.VariableCall.variable_name);
}

static inline void ast_print_if_else_statement(AST *ast) {
    if (ast->value.IfElseStatement.is_else_if_statement)
        printf("else if (");
    else if (ast->value.IfElseStatement.is_else_statement) {
        printf("else");
    } else {
        printf("if (");
    }
    if (ast->value.IfElseStatement.expression)
        ast_print(ast->value.IfElseStatement.expression);
    ast->value.IfElseStatement.is_else_statement ? printf(" {\n") : printf(") {\n");
    for (int i = 0; i < ast->value.IfElseStatement.body_size; i++) {
        printf("\t\t");
        ast_print(ast->value.IfElseStatement.body[i]);
    }
    printf("\t}\n");
}

static inline void ast_print_while_statement(AST *ast) {
    printf("while (");
    ast_print(ast->value.WhileStatement.expression);
    printf(") {\n");
    for (int i = 0; i < ast->value.WhileStatement.body_size; i++) {
        printf("\t\t");
        ast_print(ast->value.WhileStatement.body[i]);
    }
    printf("\t}\n");
}