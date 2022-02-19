//
//  token.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "token.h"

char *token_print(int type) {
    switch (type) {
        case T_FUNC:
            return "T_FUNC";
        case T_LET:
            return "T_LET";
        case T_VAR:
            return "T_VAR";
        case T_RETURN:
            return "T_RETURN";
        case T_PRINT:
            return "T_PRINT";
        case T_IF:
            return "T_IF";
        case T_IMPORT:
            return "T_IMPORT";
        case T_INT:
            return "T_INT";
        case T_FLOAT:
            return "T_FLOAT";
        case T_VOID:
            return "T_VOID";
        case T_STRING:
            return "T_STRING";
        case T_BOOL:
            return "T_BOOL";
        case T_IDENTIFIER:
            return "T_IDENTIFIER";
        case T_EQUAL:
            return "T_EQUAL";
        case T_OPERATOR_ADD:
            return "T_OPERATOR_ADD";
        case T_OPERATOR_SUB:
            return "T_OPERATOR_ADD";
        case T_OPERATOR_MUL:
            return "T_OPERATOR_MUL";
        case T_OPERATOR_DIV:
            return "T_OPERATOR_DIV";
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
        case T_LESS_THAN:
            return "T_LESS_THAN";
        case T_GREATER_THAN:
            return "T_GREATER_THAN";
        case T_ADDRESS:
            return "T_ADDRESS";
        case T_EOF:
            return "T_EOF";
    }

    return "UNKNOWN";
}