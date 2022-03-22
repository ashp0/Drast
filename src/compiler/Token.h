//
// Created by Ashwin Paudel on 2022-03-20.
//

#ifndef DRAST_TOKEN_H
#define DRAST_TOKEN_H

#include <iostream>
#include <unordered_map>

enum class TokenType {
    // Keywords
    STRUCT,
    SELF,
    ENUM,
    TYPEALIAS,
    RETURN,
    IF,
    ELSE,
    IMPORT,
    ASM,
    VOLATILE,
    CAST,
    EXTERN,

    INT,
    FLOAT,
    VOID,
    STRING,
    CHAR,
    BOOL,
    FALSE,
    TRUE,

    SWITCH,
    CASE,
    BREAK,
    DEFAULT,
    WHILE,
    FOR,
    CONTINUE,
    UNION,

    // Values
    V_NUMBER,
    V_FLOAT,
    V_STRING,
    V_CHAR,
    V_HEX,
    V_OCTAL,
    IDENTIFIER,

    GOTO,
    PRIVATE,

    DO,
    TRY,
    CATCH,

    // Operators
    QUESTION, // ?

    LESS_THAN,       // <
    LESS_THAN_EQUAL, // <=

    GREATER_THAN,       // >
    GREATER_THAN_EQUAL, // >=

    EQUAL,       // =
    EQUAL_EQUAL, // ==

    NOT,       // !
    NOT_EQUAL, // !=

    OPERATOR_ADD,       // +
    OPERATOR_ADD_EQUAL, // +=

    OPERATOR_SUB,       // -
    OPERATOR_SUB_EQUAL, // -=

    OPERATOR_MUL,       // *
    OPERATOR_MUL_EQUAL, // *=

    OPERATOR_DIV,       // /
    OPERATOR_DIV_EQUAL, // /=

    OPERATOR_MOD,       // %
    OPERATOR_MOD_EQUAL, // %=

    // Bitwise Operators
    BITWISE_AND,           // &
    BITWISE_AND_EQUAL,     // &=
    BITWISE_AND_AND,       // &&
    BITWISE_AND_AND_EQUAL, // &&=

    BITWISE_PIPE,            // |
    BITWISE_PIPE_EQUAL,      // |=
    BITWISE_PIPE_PIPE,       // ||
    BITWISE_PIPE_PIPE_EQUAL, // ||=

    BITWISE_SHIFT_LEFT,       // <<
    BITWISE_SHIFT_LEFT_EQUAL, // <<=

    BITWISE_SHIFT_RIGHT,       // >>
    BITWISE_SHIFT_RIGHT_EQUAL, // >>=

    BITWISE_POWER,       // ^
    BITWISE_POWER_EQUAL, // ^=

    BITWISE_NOT, // ~

    // Symbols
    COLON,        // :
    DOUBLE_COLON, // ::
    SEMICOLON,    // ;
    PARENS_OPEN,  // (
    PARENS_CLOSE, // )
    BRACE_OPEN,   // {
    BRACE_CLOSE,  // }
    SQUARE_OPEN,  // [
    SQUARE_CLOSE, // ]
    COMMA,        // ,
    PERIOD,       // .
    DOLLAR,       // $
    HASHTAG,      // #
    AT,           // @
    BACKSLASH,

    // Other
    T_EOF,
};

std::string tokenTypeAsLiteral(TokenType type);

constexpr bool isRegularType(TokenType type) {
    switch (type) {
    case TokenType::INT:
    case TokenType::CHAR:
    case TokenType::VOID:
    case TokenType::BOOL:
    case TokenType::FLOAT:
    case TokenType::STRING:
    case TokenType::IDENTIFIER:
        return true;
    default:
        return false;
    }
}

constexpr bool isRegularValue(TokenType type) {
    switch (type) {
    case TokenType::V_NUMBER:
    case TokenType::V_CHAR:
    case TokenType::V_STRING:
    case TokenType::TRUE:
    case TokenType::FALSE:
    case TokenType::IDENTIFIER:
        return true;
    default:
        return false;
    }
}

constexpr bool isMultiplicativeOperator(TokenType type) {
    switch (type) {
    case TokenType::OPERATOR_MUL:
    case TokenType::OPERATOR_DIV:
    case TokenType::OPERATOR_MOD:
        return true;
    default:
        return false;
    }
}

constexpr bool isAdditiveOperator(TokenType type) {
    switch (type) {
    case TokenType::OPERATOR_ADD:
    case TokenType::OPERATOR_SUB:
    case TokenType::PERIOD:
        return true;
    default:
        return false;
    }
}

constexpr bool isEqualitiveOperator(TokenType type) {
    switch (type) {
    case TokenType::EQUAL_EQUAL:
    case TokenType::NOT_EQUAL:
        return true;
    default:
        return false;
    }
}

constexpr bool isComparitiveOperator(TokenType type) {
    switch (type) {
    case TokenType::LESS_THAN:
    case TokenType::LESS_THAN_EQUAL:
    case TokenType::GREATER_THAN:
    case TokenType::GREATER_THAN_EQUAL:
        return true;
    default:
        return false;
    }
}

class Token {
  public:
    std::string value;
    size_t line;
    size_t column;
    TokenType type;

  public:
    Token(std::string &value, TokenType type, size_t line, size_t column)
        : value(value), type(type), line(line), column(column) {}

    static TokenType is_keyword(const std::string &string, size_t length);

    friend std::ostream &operator<<(std::ostream &out, Token const &token) {
        out << "Token: `" << token.value << "` "
            << tokenTypeAsLiteral(token.type) << " L`" << token.line << "` C`"
            << token.column << '`';
        return out;
    }
};

#endif // DRAST_TOKEN_H
