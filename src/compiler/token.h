//
//  token.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef __DRAST_COMPILER_TOKEN_H__
#define __DRAST_COMPILER_TOKEN_H__

typedef struct {
    char *value;
    enum {
        // Keywords ( 0-6 )
        T_FUNC,
        T_LET,
        T_VAR,
        T_RETURN,
        T_PRINT,
        T_IF,
        T_IMPORT,

        // Data Types ( 7-12 )
        T_INT,
        T_FLOAT,
        T_VOID,
        T_STRING,
        T_BOOL,
        T_IDENTIFIER,

        // Operators ( 12-14 )
        T_EQUAL,
        T_OPERATOR_ADD,
        T_OPERATOR_SUB,
        T_OPERATOR_MUL,
        T_OPERATOR_DIV,

        // Symbols ( 15-25 )
        T_ARROW,
        T_COLON,
        T_SEMICOLON,
        T_PARENS_OPEN,
        T_PARENS_CLOSE,
        T_BRACE_OPEN,
        T_BRACE_CLOSE,
        T_SQUARE_OPEN,
        T_SQUARE_CLOSE,
        T_COMMA,
        T_PERIOD,
        T_LESS_THAN,
        T_GREATER_THAN,
        T_ADDRESS, // &

        // Other ( 26 )
        T_EOF,
    } type;
} Token;

char *token_print(int type);

#endif // __DRAST_COMPILER_TOKEN_H__
