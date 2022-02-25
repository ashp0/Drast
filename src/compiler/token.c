//
//  token.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "token.h"

char *token_print(int type) {
    switch (type) {
        // Keywords
        case T_K_STRUCT:
            return "T_K_STRUCT";
        case T_K_ENUM:
            return "T_K_ENUM";
        case T_K_ALIAS:
            return "T_K_ALIAS";
        case T_K_RETURN:
            return "T_K_RETURN";
        case T_K_IF:
            return "T_K_IF";
        case T_K_ELSE:
            return "T_K_ELSE";
        case T_K_IMPORT:
            return "T_K_IMPORT";
        case T_K_ASM:
            return "T_K_ASM";
        case T_K_VOLATILE:
            return "T_K_VOLATILE";
        case T_K_CAST:
            return "T_K_CAST";

        case T_K_SWITCH:
            return "T_K_SWITCH";
        case T_K_MATCHES:
            return "T_K_MATCHES";
        case T_K_CASE:
            return "T_K_CASE";
        case T_K_BREAK:
            return "T_K_BREAK";
        case T_K_DEFAULT:
            return "T_K_DEFAULT";
        case T_K_WHILE:
            return "T_K_WHILE";
        case T_K_FOR:
            return "T_K_FOR";
        case T_K_CONTINUE:
            return "T_K_CONTINUE";
        case T_K_UNION:
            return "T_K_UNION";

        case T_K_FALSE:
            return "T_K_FALSE";
        case T_K_TRUE:
            return "T_K_TRUE";
        case T_K_BOOL:
            return "T_K_BOOL";
        case T_K_INT:
            return "T_K_INT";
        case T_K_FLOAT:
            return "T_K_FLOAT";
        case T_K_VOID:
            return "T_K_VOID";
        case T_K_STRING:
            return "T_K_STRING";
        case T_K_CHAR:
            return "T_K_CHAR";
        case T_IDENTIFIER:
            return "T_IDENTIFIER";

        case T_INT:
            return "T_INT";
        case T_FLOAT:
            return "T_FLOAT";
        case T_STRING:
            return "T_STRING";

        case T_K_GOTO:
            return "T_K_GOTO";
        case T_K_PRIVATE:
            return "T_K_PRIVATE";

        case T_K_DO:
            return "T_K_DO";
        case T_K_TRY:
            return "T_K_TRY";
        case T_K_CATCH:
            return "T_K_CATCH";

        case T_QUESTION:
            return "T_QUESTION";

        case T_LESS_THAN:
            return "T_LESS_THAN";
        case T_LESS_THAN_EQUAL:
            return "T_LESS_THAN_EQUAL";

        case T_GREATER_THAN:
            return "T_GREATER_THAN";
        case T_GREATER_THAN_EQUAL:
            return "T_GREATER_THAN_EQUAL";

        case T_EQUAL:
            return "T_EQUAL";
        case T_EQUAL_EQUAL:
            return "T_EQUAL_EQUAL";

        case T_NOT:
            return "T_NOT";
        case T_NOT_EQUAL:
            return "T_NOT_EQUAL";

        case T_OPERATOR_ADD:
            return "T_OPERATOR_ADD";
        case T_OPERATOR_ADD_EQUAL:
            return "T_OPERATOR_ADD_EQUAL";

        case T_OPERATOR_SUB:
            return "T_OPERATOR_SUB";
        case T_OPERATOR_SUB_EQUAL:
            return "T_OPERATOR_SUB_EQUAL";

        case T_OPERATOR_MUL:
            return "T_OPERATOR_MUL";
        case T_OPERATOR_MUL_EQUAL:
            return "T_OPERATOR_MUL_EQUAL";

        case T_OPERATOR_DIV:
            return "T_OPERATOR_DIV";
        case T_OPERATOR_DIV_EQUAL:
            return "T_OPERATOR_DIV_EQUAL";

        case T_OPERATOR_MOD:
            return "T_OPERATOR_MOD";
        case T_OPERATOR_MOD_EQUAL:
            return "T_OPERATOR_MOD_EQUAL";

        case T_BITWISE_AND:
            return "T_BITWISE_AND";
        case T_BITWISE_AND_EQUAL:
            return "T_BITWISE_AND_EQUAL";
        case T_BITWISE_AND_AND:
            return "T_BITWISE_AND_AND";
        case T_BITWISE_AND_AND_EQUAL:
            return "T_BITWISE_AND_AND_EQUAL";

        case T_BITWISE_PIPE:
            return "T_BITWISE_PIPE";
        case T_BITWISE_PIPE_EQUAL:
            return "T_BITWISE_PIPE_EQUAL";
        case T_BITWISE_PIPE_PIPE:
            return "T_BITWISE_PIPE_PIPE";
        case T_BITWISE_PIPE_PIPE_EQUAL:
            return "T_BITWISE_PIPE_PIPE_EQUAL";

        case T_BITWISE_SHIFT_LEFT:
            return "T_BITWISE_SHIFT_LEFT";
        case T_BITWISE_SHIFT_LEFT_EQUAL:
            return "T_BITWISE_SHIFT_LEFT_EQUAL";

        case T_BITWISE_SHIFT_RIGHT:
            return "T_BITWISE_SHIFT_RIGHT";
        case T_BITWISE_SHIFT_RIGHT_EQUAL:
            return "T_BITWISE_SHIFT_RIGHT_EQUAL";

        case T_BITWISE_POWER:
            return "T_BITWISE_POWER";
        case T_BITWISE_POWER_EQUAL:
            return "T_BITWISE_POWER_EQUAL";
        case T_BITWISE_NOT:
            return "T_BITWISE_NOT";

        case T_ARROW:
            return "T_ARROW";
        case T_COLON:
            return "T_COLON";
        case T_DOUBLE_COLON:
            return "T_DOUBLE_COLON";
        case T_SEMICOLON:
            return "T_SEMICOLON";
        case T_PARENS_OPEN:
            return "T_PARENS_OPEN";
        case T_PARENS_CLOSE:
            return "T_PARENS_CLOSE";
        case T_BRACE_OPEN:
            return "T_BRACE_OPEN";
        case T_BRACE_CLOSE:
            return "T_BRACE_CLOSE";
        case T_SQUARE_OPEN:
            return "T_SQUARE_OPEN";
        case T_SQUARE_CLOSE:
            return "T_SQUARE_CLOSE";
        case T_COMMA:
            return "T_COMMA";
        case T_PERIOD:
            return "T_PERIOD";
        case T_DOLLAR:
            return "T_DOLLAR";
        case T_HASHTAG:
            return "T_HASHTAG";
        case T_AT:
            return "T_AT";
        case T_BACKSLASH:
            return "T_BACKSLASH";

        case T_EOF:
            return "T_EOF";
    }

    printf("%d ", type);
    return "UNKNOWN TOKEN";
}
