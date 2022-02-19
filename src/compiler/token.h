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
        // Keywords
        T_FUNC,
        T_LET,
        T_VAR,
        T_STRUCT,
        T_ENUM,
        T_ALIAS,
        T_RETURN,
        T_IF,
        T_ELSE,
        T_IMPORT,
        T_PRINT,
        T_ASM,
        T_VOLATILE,
        T_CAST,

        T_SWITCH,
        T_CASE,
        T_BREAK,
        T_DEFAULT,
        T_WHILE,
        T_FOR,
        T_CONTINUE,
        T_TYPEDEF,
        T_UNION,

        T_FALSE,
        T_TRUE,
        T_BOOL,
        T_INT,
        T_FLOAT,
        T_VOID,
        T_STRING,
        T_IDENTIFIER,

        T_GOTO,
        T_PUBLIC,
        T_PRIVATE,

        T_DO,
        T_TRY,
        T_CATCH,
        T_THROW,
        T_THROWS,

        // Operators
        T_QUESTION, // ?

        T_LESS_THAN, // <
        T_LESS_THAN_EQUAL, // <=

        T_GREATER_THAN, // >
        T_GREATER_THAN_EQUAL, // >=

        T_EQUAL, // =
        T_EQUAL_EQUAL, // ==

        T_NOT, // !
        T_NOT_EQUAL, // !=

        T_OPERATOR_ADD, // +
        T_OPERATOR_ADD_EQUAL, // +=

        T_OPERATOR_SUB, // -
        T_OPERATOR_SUB_EQUAL, // -=

        T_OPERATOR_MUL, // *
        T_OPERATOR_MUL_EQUAL, // *=

        T_OPERATOR_DIV, // /
        T_OPERATOR_DIV_EQUAL, // /=

        T_OPERATOR_MOD, // %
        T_OPERATOR_MOD_EQUAL, // %=

        // Bitwise Operators
        T_BITWISE_AND, // &
        T_BITWISE_AND_EQUAL, // &=
        T_BITWISE_AND_AND, // &&
        T_BITWISE_AND_AND_EQUAL, // &&=

        T_BITWISE_PIPE, // |
        T_BITWISE_PIPE_EQUAL, // |=
        T_BITWISE_PIPE_PIPE, // ||
        T_BITWISE_PIPE_PIPE_EQUAL, // ||=

        T_BITWISE_SHIFT_LEFT, // <<
        T_BITWISE_SHIFT_LEFT_EQUAL, // <<=

        T_BITWISE_SHIFT_RIGHT, // >>
        T_BITWISE_SHIFT_RIGHT_EQUAL, // >>=

        T_BITWISE_POWER, // ^
        T_BITWISE_POWER_EQUAL, // ^=

        T_BITWISE_NOT, // ~

        // Symbols
        T_ARROW, // ->
        T_COLON, // :
        T_SEMICOLON, // ;
        T_PARENS_OPEN, // (
        T_PARENS_CLOSE, // )
        T_BRACE_OPEN, // {
        T_BRACE_CLOSE, // }
        T_SQUARE_OPEN, // [
        T_SQUARE_CLOSE, // ]
        T_COMMA, // ,
        T_PERIOD, // .
        T_DOLLAR, // $
        T_HASHTAG, // #
        T_AT, // @
        T_BACKSLASH,

        // Other
        T_EOF,
    } type;
} Token;

char *token_print(int type);

#endif // __DRAST_COMPILER_TOKEN_H__
