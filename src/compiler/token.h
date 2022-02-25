//
//  token.h
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#ifndef __DRAST_COMPILER_TOKEN_H__
#define __DRAST_COMPILER_TOKEN_H__

#include "stdio.h"

typedef struct {
    char *value;
    enum {
        // Keywords
        T_K_STRUCT,
        T_K_ENUM,
        T_K_ALIAS,
        T_K_RETURN,
        T_K_IF,
        T_K_ELSE,
        T_K_IMPORT,
        T_K_PRINT,
        T_K_ASM,
        T_K_VOLATILE,
        T_K_CAST,
        T_K_INT,
        T_K_FLOAT,
        T_K_VOID,
        T_K_STRING,
        T_K_CHAR,
        T_K_BOOL,
        T_K_FALSE,
        T_K_TRUE,

        T_K_SWITCH,
        T_K_MATCHES,
        T_K_CASE,
        T_K_BREAK,
        T_K_DEFAULT,
        T_K_WHILE,
        T_K_FOR,
        T_K_CONTINUE,
        T_K_UNION,

        // Values
        T_INT,
        T_FLOAT,
        T_STRING,
        T_IDENTIFIER,

        T_K_GOTO,
        T_K_PRIVATE,

        T_K_DO,
        T_K_TRY,
        T_K_CATCH,

        // Operators
        T_QUESTION, // ?

        T_LESS_THAN,       // <
        T_LESS_THAN_EQUAL, // <=

        T_GREATER_THAN,       // >
        T_GREATER_THAN_EQUAL, // >=

        T_EQUAL,       // =
        T_EQUAL_EQUAL, // ==

        T_NOT,       // !
        T_NOT_EQUAL, // !=

        T_OPERATOR_ADD,       // +
        T_OPERATOR_ADD_EQUAL, // +=

        T_OPERATOR_SUB,       // -
        T_OPERATOR_SUB_EQUAL, // -=

        T_OPERATOR_MUL,       // *
        T_OPERATOR_MUL_EQUAL, // *=

        T_OPERATOR_DIV,       // /
        T_OPERATOR_DIV_EQUAL, // /=

        T_OPERATOR_MOD,       // %
        T_OPERATOR_MOD_EQUAL, // %=

        // Bitwise Operators
        T_BITWISE_AND,           // &
        T_BITWISE_AND_EQUAL,     // &=
        T_BITWISE_AND_AND,       // &&
        T_BITWISE_AND_AND_EQUAL, // &&=

        T_BITWISE_PIPE,            // |
        T_BITWISE_PIPE_EQUAL,      // |=
        T_BITWISE_PIPE_PIPE,       // ||
        T_BITWISE_PIPE_PIPE_EQUAL, // ||=

        T_BITWISE_SHIFT_LEFT,       // <<
        T_BITWISE_SHIFT_LEFT_EQUAL, // <<=

        T_BITWISE_SHIFT_RIGHT,       // >>
        T_BITWISE_SHIFT_RIGHT_EQUAL, // >>=

        T_BITWISE_POWER,       // ^
        T_BITWISE_POWER_EQUAL, // ^=

        T_BITWISE_NOT, // ~

        // Symbols
        T_ARROW,        // ->
        T_COLON,        // :
        T_DOUBLE_COLON, // ::
        T_SEMICOLON,    // ;
        T_PARENS_OPEN,  // (
        T_PARENS_CLOSE, // )
        T_BRACE_OPEN,   // {
        T_BRACE_CLOSE,  // }
        T_SQUARE_OPEN,  // [
        T_SQUARE_CLOSE, // ]
        T_COMMA,        // ,
        T_PERIOD,       // .
        T_DOLLAR,       // $
        T_HASHTAG,      // #
        T_AT,           // @
        T_BACKSLASH,

        // Other
        T_EOF,
    } type;
} Token;

char *token_print(int type);

#endif // __DRAST_COMPILER_TOKEN_H__
