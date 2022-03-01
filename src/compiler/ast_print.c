//
//  ast_print.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "ast_print.h"

void ast_print(AST *ast) {
    _ast_print(ast, 0);
}

void _ast_print(AST *ast, uintptr_t indent) {
    switch (ast->type) {
        case AST_TYPE_DO_CATCH:
            ast_print_do_catch_statement(ast, indent);
            break;
        case AST_TYPE_IMPORT:
            ast_print_import(ast, indent);
            break;
        case AST_TYPE_VARIABLE_DEFINITION:
            ast_print_variable(ast, true, indent);
            break;
        case AST_TYPE_FUNCTION_DECLARATION:
            ast_print_function_declaration(ast, indent);
            break;
        case AST_TYPE_STRUCT_OR_UNION_DECLARATION:
            ast_print_struct_declaration(ast, indent);
            break;
        case AST_TYPE_ENUM_DECLARATION:
            ast_print_enum_declaration(ast, indent);
            break;
        case AST_TYPE_BINARY:
            ast_print_binary(ast, indent);
            break;
        case AST_TYPE_LITERAL:
            ast_print_literal(ast, indent);
            break;
        case AST_TYPE_GROUPING:
            ast_print_grouping(ast, indent);
            break;
        case AST_TYPE_VALUE_KEYWORD:
            ast_print_type_name(ast, indent);
            break;
        case AST_TYPE_RETURN:
            ast_print_return(ast, indent);
            break;
        case AST_TYPE_FUNCTION_CALL:
            ast_print_function_call(ast, indent);
            break;
        case AST_TYPE_VARIABLE_CALL:
            ast_print_variable_call(ast, indent);
            break;
        case AST_TYPE_IF_ELSE_STATEMENT:
            ast_print_if_else_statement(ast, indent);
            break;
        case AST_TYPE_WHILE_STATEMENT:
            ast_print_while_statement(ast, indent);
            break;
        case AST_TYPE_FOR_LOOP:
            ast_print_for_loop(ast, indent);
            break;
        case AST_TYPE_INLINE_ASSEMBLY:
            ast_print_inline_assembly(ast, indent);
            break;
        case AST_TYPE_SWITCH_STATEMENT:
            ast_print_switch_statement(ast, indent);
            break;
        case AST_TYPE_TRY_STATEMENT:
            ast_print_try_statement(ast, indent);
            break;
        case AST_TYPE_STRUCT_OR_UNION_MEMBER_CALL:
            ast_print_struct_member_call(ast, indent);
            break;
        case AST_TYPE_BODY:
            ast_print_body(ast, indent);
            break;
        case AST_TYPE_NOT:
            ast_print_not(ast, indent);
            break;
        case AST_TYPE_STRUCT_INITIALIZER:
            ast_print_struct_initializer(ast, indent);
            break;
        case AST_TYPE_ALIAS:
            ast_print_alias(ast);
            break;
        default:
            printf("Cannot Print AST %d\n", ast->type);
            break;
    }
}

void ast_print_body(AST *ast, uintptr_t indent) {
    for (int i = 0; i < ast->value.Body.body_size; ++i) {
        _ast_print(ast->value.Body.body[i], indent + 1);
    }
}

void ast_print_struct_member_call(AST *ast, uintptr_t indent) {
//    for (int i = 0; i < indent; ++i)
//        printf("\t");
    _ast_print(ast->value.StructOrUnionMemberCall.struct_or_variable_name, indent);
    printf(".");
    _ast_print(ast->value.StructOrUnionMemberCall.expression, 0);
    printf("\n");
}

void ast_print_try_statement(AST *ast, uintptr_t indent) {
    printf("try ");
    ast_print(ast->value.TryStatement.expression);
}

void ast_print_do_catch_statement(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");

    printf("do {\n");

    for (int i = 0; i < ast->value.DoCatchOrWhileStatement.do_body_size; ++i) {
        _ast_print(ast->value.DoCatchOrWhileStatement.do_body[i], indent + 1);
    }

    for (int i = 0; i < indent; ++i)
        printf("\t");
    if (ast->value.DoCatchOrWhileStatement.is_while_statement) {
        printf("} while (");
        ast_print(ast->value.DoCatchOrWhileStatement.while_statement_expression);
        printf(")\n\n");
    } else {
        printf("} catch {\n");

        for (int i = 0; i < ast->value.DoCatchOrWhileStatement.second_body_size; ++i) {
            _ast_print(ast->value.DoCatchOrWhileStatement.second_body[i], indent + 1);
        }

        for (int i = 0; i < indent; ++i)
            printf("\t");
        printf("}\n");
    }
}

void ast_print_import(AST *ast, uintptr_t indent) {
//    printf("import %s, is library: %d", ast->value.Import.file, ast->value.Import.is_library);
    for (int i = 0; i < indent; ++i)
        printf("\t");
    printf("import %s\n", ast->value.Import.file);
}

void ast_print_variable(AST *ast, bool new_line, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");

    if (ast->value.VariableDeclaration.is_private)
        printf("private ");

    if (ast->value.VariableDeclaration.is_volatile) {
        printf("volatile ");
    }

    ast_print_type_name(ast->value.VariableDeclaration.type, 0);

    printf(" %s", ast->value.VariableDeclaration.identifier);

    if (ast->value.VariableDeclaration.is_initialized) {
        printf(" = ");
        ast_print(ast->value.VariableDeclaration.value);
    }
    if (ast->value.VariableDeclaration.type->value.ValueKeyword.is_optional) {
        printf("?");
    }
    if (new_line)
        printf("\n");
}

void ast_print_switch_statement(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");

    if (ast->value.SwitchStatement.is_matches_statement)
        printf("matches (");
    else
        printf("switch (");
    ast_print(ast->value.SwitchStatement.expression);
    printf(") {");

    for (int i = 0; i < ast->value.SwitchStatement.switch_cases_size; ++i) {
        ast_print_switch_case_statement(ast->value.SwitchStatement.switch_cases[i], indent + 1);
    }

    printf("\n\t}\n");
}

void ast_print_switch_case_statement(AST *ast, uintptr_t indent) {
    if (!ast->value.SwitchCase.is_default) {
        printf("\n");
        for (int i = 0; i < indent; ++i)
            printf("\t");
        printf("case (");
        _ast_print(ast->value.SwitchCase.expression, 0);
        printf("): \n");
    } else {
        printf("\n");
        for (int i = 0; i < indent; ++i)
            printf("\t");
        printf("default: \n");
    }

    for (int i = 0; i < ast->value.SwitchCase.body_size; ++i) {
        _ast_print(ast->value.SwitchCase.body[i], indent + 1);
    }
}

void ast_print_function_declaration(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");

    if (ast->value.FunctionDeclaration.is_private)
        printf("private ");

    if (ast->value.FunctionDeclaration.is_struct_initializer) {
        printf("@init(");
    } else {
        ast_print_type_name(ast->value.FunctionDeclaration.return_type, 0);
        printf(" :: %s(", ast->value.FunctionDeclaration.function_name);
    }

    ast_print_function_arguments(ast, 0);
    printf(" {\n");

    _ast_print(ast->value.FunctionDeclaration.body, indent + 1);

    printf("\n");
    for (int i = 0; i < indent; ++i)
        printf("\t");
    printf("}\n\n");
}

void ast_print_function_arguments(AST *ast, uintptr_t indent) {
    if (ast->value.FunctionDeclaration.arguments_size == 0) {
        printf(")");
    } else {
        for (int i = 0; i < ast->value.FunctionDeclaration.arguments_size; i++) {
            ast_print_type_name(
                    ast->value.FunctionDeclaration.arguments[i]->value.FunctionDeclarationArgument.argument_type,
                    indent);
            printf(" %s, ",
                   ast->value.FunctionDeclaration.arguments[i]->value.FunctionDeclarationArgument.argument_name);
        }

        printf("\b\b)");
    }
}

void ast_print_function_call(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");
    printf("%s(", ast->value.FunctionCall.function_call_name);

    for (int i = 0; i < ast->value.FunctionCall.arguments_size; i++) {
        ast_print(ast->value.FunctionCall.arguments[i]);
        printf(", ");
    }

    if (ast->value.FunctionCall.arguments_size == 0)
        printf(")");
    else
        printf("\b\b)");
}

void ast_print_struct_declaration(AST *ast, uintptr_t indent) {
    if (ast->value.StructOrUnionDeclaration.is_private) {
        printf("private ");
    }
    if (ast->value.StructOrUnionDeclaration.is_union)
        printf("union %s {\n", ast->value.StructOrUnionDeclaration.name);
    else
        printf("struct %s {\n", ast->value.StructOrUnionDeclaration.name);

    for (int i = 0; i < ast->value.StructOrUnionDeclaration.members_size; ++i) {
        _ast_print(ast->value.StructOrUnionDeclaration.members[i], indent + 1);
    }

    // Don't need to use indent, because struct will always be outside
    printf("}\n\n");
}

void ast_print_enum_declaration(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");
    if (ast->value.EnumDeclaration.is_private) {
        printf("private ");
    }
    printf("enum %s {\n", ast->value.EnumDeclaration.enum_name);

    for (int i = 0; i < ast->value.EnumDeclaration.case_size; ++i) {
        printf("\tcase %s = %d\n", ast->value.EnumDeclaration.cases[i]->value.EnumItem.case_name,
               ast->value.EnumDeclaration.cases[i]->value.EnumItem.case_value);
    }

    printf("}\n\n");
}

void ast_print_return(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");
    printf("return ");
    ast_print(ast->value.Return.return_expression);
    printf("\n");
}

void ast_print_for_loop(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");
    printf("for (");
    ast_print_variable(ast->value.ForLoop.variable, false, 0);
    printf(", ");
    ast_print(ast->value.ForLoop.condition);
    printf(", ");
    ast_print(ast->value.ForLoop.condition2);
    printf(") {\n");

    for (int i = 0; i < ast->value.ForLoop.body_size; i++) {
        _ast_print(ast->value.ForLoop.body[i], indent + 1);
        printf("\n");
    }

    printf("\n");
    for (int i = 0; i < indent; ++i)
        printf("\t");
    printf("}\n");
}

void ast_print_type_name(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");
    if (ast->value.ValueKeyword.is_array) {
        printf("%s[]", token_print(ast->value.ValueKeyword.token->type));
    } else {
        printf("%s", token_print(ast->value.ValueKeyword.token->type));
    }

    if (ast->value.ValueKeyword.is_pointer)
        printf("*");
}

void ast_print_binary(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");
    ast_print(ast->value.Binary.left);
    printf(" %s ", ast->value.Binary.operator->value);
    ast_print(ast->value.Binary.right);
}

void ast_print_literal(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");
    printf("%s", ast->value.Literal.literal_value->value);
}

void ast_print_grouping(AST *ast, uintptr_t indent) {
    printf("(");
    _ast_print(ast->value.Grouping.expression, indent);
    printf(")");
}

void ast_print_variable_call(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");
    if (ast->value.VariableCall.is_cast) {
        printf("%s ->", ast->value.VariableCall.variable_name);
        ast_print(ast->value.VariableCall.cast_value);
        printf(" \n");
    } else {
        printf("%s %s ", ast->value.VariableCall.variable_name, ast->value.VariableCall.operator->value);
        _ast_print(ast->value.VariableCall.expression, 0);
        printf("\n");
    }
    if (indent != 0)
        printf("\n");
}

void ast_print_if_else_statement(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");

    printf("if (");
    ast_print(ast->value.IfElseStatement.if_condition);
    printf(") {\n");
    _ast_print(ast->value.IfElseStatement.if_body, indent + 1);
    printf("}\n");

    if (ast->value.IfElseStatement.else_if_size != 0) {
        for (int i = 0; i < indent; ++i)
            printf("\t");

        // For each else if statement
        for (int i = 0; i < ast->value.IfElseStatement.else_if_size; ++i) {
            printf("else if (");
            ast_print(ast->value.IfElseStatement.else_if_conditions[i]);
            printf(") {\n");
            _ast_print(ast->value.IfElseStatement.else_if_bodys[i], indent + 1);
            printf("}\n");
        }
    }

    if (ast->value.IfElseStatement.has_else_statement) {
        for (int i = 0; i < indent; ++i)
            printf("\t");
        printf("else {\n");
        _ast_print(ast->value.IfElseStatement.else_body, indent + 1);
        printf("}\n");
    }
}

void ast_print_while_statement(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");
    printf("while (");
    ast_print(ast->value.WhileStatement.expression);
    printf(") {\n");
    _ast_print(ast->value.WhileStatement.body, indent + 1);
    printf("\t}\n");
}

void ast_print_inline_assembly(AST *ast, uintptr_t indent) {
    printf("asm {\n");
    for (int i = 0; i < ast->value.InlineAssembly.instructions_size; ++i) {
        for (int i = 0; i < indent + 1; ++i)
            printf("\t");
        printf("%s\n", ast->value.InlineAssembly.instruction[i]);
    }

    for (int i = 0; i < indent; ++i)
        printf("\t");
    printf("}\n");
}

void ast_print_not(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");
    printf("!");
    ast_print(ast->value.Not.expression);
}

void ast_print_struct_initializer(AST *ast, uintptr_t indent) {
    for (int i = 0; i < indent; ++i)
        printf("\t");
    printf(".");
    _ast_print(ast->value.StructInitializer.function_call, indent);
    printf("\n");
}

void ast_print_alias(AST *ast) {
    printf("alias %s : ", ast->value.Alias.alias_name);
    _ast_print(ast->value.Alias.alias_value, 0);
    printf("\n");
}