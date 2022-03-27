//
// Created by Ashwin Paudel on 2022-03-20.
//

#ifndef DRAST_TOKEN_H
#define DRAST_TOKEN_H

#include "Types.h"
#include <iostream>
#include <sstream>
#include <unordered_map>

enum class TokenType : uint8_t {
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
    NIL,

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

constexpr bool isOperatorType(TokenType type) {
    switch (type) {
    case TokenType::QUESTION:
    case TokenType::LESS_THAN:
    case TokenType::LESS_THAN_EQUAL:
    case TokenType::GREATER_THAN:
    case TokenType::GREATER_THAN_EQUAL:
    case TokenType::EQUAL:
    case TokenType::EQUAL_EQUAL:
    case TokenType::NOT:
    case TokenType::NOT_EQUAL:
    case TokenType::OPERATOR_ADD:
    case TokenType::OPERATOR_ADD_EQUAL:
    case TokenType::OPERATOR_SUB:
    case TokenType::OPERATOR_SUB_EQUAL:
    case TokenType::OPERATOR_MUL:
    case TokenType::OPERATOR_MUL_EQUAL:
    case TokenType::OPERATOR_DIV:
    case TokenType::OPERATOR_DIV_EQUAL:
    case TokenType::OPERATOR_MOD:
    case TokenType::OPERATOR_MOD_EQUAL:
    case TokenType::BITWISE_AND:
    case TokenType::BITWISE_AND_EQUAL:
    case TokenType::BITWISE_AND_AND:
    case TokenType::BITWISE_AND_AND_EQUAL:
    case TokenType::BITWISE_PIPE:
    case TokenType::BITWISE_PIPE_EQUAL:
    case TokenType::BITWISE_PIPE_PIPE:
    case TokenType::BITWISE_PIPE_PIPE_EQUAL:
    case TokenType::BITWISE_SHIFT_LEFT:
    case TokenType::BITWISE_SHIFT_LEFT_EQUAL:
    case TokenType::BITWISE_SHIFT_RIGHT:
    case TokenType::BITWISE_SHIFT_RIGHT_EQUAL:
    case TokenType::BITWISE_POWER:
    case TokenType::BITWISE_POWER_EQUAL:
    case TokenType::BITWISE_NOT:
    case TokenType::COLON: // goto labels
    case TokenType::PARENS_OPEN:
    case TokenType::PERIOD:
        return true;

    default:
        return false;
    }
}

constexpr bool isTemplateKeyword(TokenType type) {
    switch (type) {
    case TokenType::INT:
    case TokenType::CHAR:
    case TokenType::VOID:
    case TokenType::BOOL:
    case TokenType::FLOAT:
    case TokenType::STRING:
    case TokenType::STRUCT:
    case TokenType::ENUM:
    case TokenType::UNION:
        return true;
    default:
        return false;
    }
}

constexpr bool isKeywordType(TokenType type) {
    switch (type) {
    case TokenType::INT:
    case TokenType::CHAR:
    case TokenType::VOID:
    case TokenType::BOOL:
    case TokenType::FLOAT:
    case TokenType::STRING:
        return true;
    default:
        return false;
    }
}

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
    case TokenType::NIL:
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
    case TokenType::AT:
        return true;
    default:
        return false;
    }
}

constexpr bool isEqualityOperator(TokenType type) {
    switch (type) {
    case TokenType::EQUAL:
    case TokenType::EQUAL_EQUAL:
    case TokenType::NOT_EQUAL:
    case TokenType::LESS_THAN:
    case TokenType::LESS_THAN_EQUAL:
    case TokenType::GREATER_THAN:
    case TokenType::GREATER_THAN_EQUAL:
    case TokenType::OPERATOR_ADD_EQUAL:
    case TokenType::OPERATOR_SUB_EQUAL:
    case TokenType::OPERATOR_MUL_EQUAL:
    case TokenType::OPERATOR_DIV_EQUAL:
    case TokenType::OPERATOR_MOD_EQUAL:
    case TokenType::BITWISE_AND_EQUAL:
    case TokenType::BITWISE_PIPE_EQUAL:
    case TokenType::BITWISE_SHIFT_LEFT_EQUAL:
    case TokenType::BITWISE_SHIFT_RIGHT_EQUAL:
    case TokenType::BITWISE_POWER_EQUAL:
    case TokenType::BITWISE_AND_AND:
    case TokenType::BITWISE_PIPE_PIPE:
        return true;
    default:
        return false;
    }
}

constexpr bool isComparativeOperator(TokenType type) {
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
    Location location;
    uint32_t start;
    uint32_t length;
    TokenType type;

  public:
    Token(TokenType type, uint32_t start, uint32_t length, Location location)
        : type(type), start(start), length(length), location(location) {}

    static TokenType is_keyword(std::string_view string);

    [[nodiscard]] std::string toString(std::string_view source) const {
        std::stringstream ss;
        ss << tokenTypeAsLiteral(this->type) << " :: `"
           << source.substr(this->start, this->length)
           << "` :: " << location.toString();

        return ss.str();
    }

    [[nodiscard]] std::string_view value(std::string_view source) const {
        return source.substr(this->start, this->length);
    }
};

#endif // DRAST_TOKEN_H
