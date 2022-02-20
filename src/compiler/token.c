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
        case T_FUNC:
            return "T_FUNC";
        case T_LET:
            return "T_LET";
        case T_VAR:
            return "T_VAR";
        case T_STRUCT:
            return "T_STRUCT";
        case T_ENUM:
            return "T_ENUM";
        case T_ALIAS:
            return "T_ALIAS";
        case T_RETURN:
            return "T_RETURN";
        case T_IF:
            return "T_IF";
        case T_ELSE:
            return "T_ELSE";
        case T_IMPORT:
            return "T_IMPORT";
        case T_PRINT:
            return "T_PRINT";
        case T_ASM:
            return "T_ASM";
        case T_VOLATILE:
            return "T_VOLATILE";
        case T_CAST:
            return "T_CAST";

        case T_SWITCH:
            return "T_SWITCH";
        case T_CASE:
            return "T_CASE";
        case T_BREAK:
            return "T_BREAK";
        case T_DEFAULT:
            return "T_DEFAULT";
        case T_WHILE:
            return "T_WHILE";
        case T_FOR:
            return "T_FOR";
        case T_CONTINUE:
            return "T_CONTINUE";
        case T_UNION:
            return "T_UNION";

        case T_FALSE:
            return "T_FALSE";
        case T_TRUE:
            return "T_TRUE";
        case T_BOOL:
            return "T_BOOL";
        case T_INT:
            return "T_INT";
        case T_FLOAT:
            return "T_FLOAT";
        case T_VOID:
            return "T_VOID";
        case T_STRING:
            return "T_STRING";
        case T_IDENTIFIER:
            return "T_IDENTIFIER";

        case T_GOTO:
            return "T_GOTO";
        case T_PRIVATE:
            return "T_STATIC";

        case T_DO:
            return "T_DO";
        case T_TRY:
            return "T_TRY";
        case T_CATCH:
            return "T_CATCH";
        case T_THROW:
            return "T_THROW";
        case T_THROWS:
            return "T_THROWS";

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

    return "UNKNOWN";
}