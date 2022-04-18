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
    FUNC,
    VAR,
    LET,
    TYPEALIAS,
    RETURN,
    IF,
    ELSE,
    IMPORT,
    ASM,
    VOLATILE,
    CAST,
    EXTERN,
    OPERATOR,
    TRUE,
    FALSE,
    NIL,

    INT_8,
    INT_16,
    INT_32,
    INT_64,
    INT_SIZE,
    UINT_8,
    UINT_16,
    UINT_32,
    UINT_64,
    UINT_SIZE,
    FLOAT_32,
    FLOAT_64,
    VOID,
    STRING,
    CHAR,
    BOOL,
    ANY,

    SWITCH,
    CASE,
    BREAK,
    DEFAULT,
    WHILE,
    FOR,
    CONTINUE,
    UNION,
    TYPEOF, // compile time constant
    IN,

    GOTO,
    PRIVATE,
    DO,
    TRY,
    CATCH,
    THROW,

    // Values
    V_INT,
    V_FLOAT,
    V_STRING,
    V_CHAR,
    V_MULTILINE_STRING,
    V_HEX,
    V_OCTAL,
    V_BINARY,
    IDENTIFIER,

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
    BACKSLASH,    // \

    // Other
    NEW_LINE, // \n
    T_EOF,    // \0
};

std::string tokenTypeAsLiteral(TokenType type);

constexpr bool isOperatorOverloadType(TokenType type) {
    switch (type) {
    case TokenType::LESS_THAN:
    case TokenType::LESS_THAN_EQUAL:
    case TokenType::GREATER_THAN:
    case TokenType::GREATER_THAN_EQUAL:
    case TokenType::EQUAL:
    case TokenType::EQUAL_EQUAL:
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
    case TokenType::SQUARE_OPEN:
    case TokenType::SQUARE_CLOSE:
        return true;

    default:
        return false;
    }
}

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
    case TokenType::INT_8:
    case TokenType::INT_16:
    case TokenType::INT_32:
    case TokenType::INT_64:
    case TokenType::INT_SIZE:
    case TokenType::UINT_8:
    case TokenType::UINT_16:
    case TokenType::UINT_32:
    case TokenType::UINT_64:
    case TokenType::UINT_SIZE:
    case TokenType::FLOAT_32:
    case TokenType::FLOAT_64:
    case TokenType::VOID:
    case TokenType::STRING:
    case TokenType::CHAR:
    case TokenType::BOOL:
    case TokenType::ANY:
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
    case TokenType::INT_8:
    case TokenType::INT_16:
    case TokenType::INT_32:
    case TokenType::INT_64:
    case TokenType::INT_SIZE:
    case TokenType::UINT_8:
    case TokenType::UINT_16:
    case TokenType::UINT_32:
    case TokenType::UINT_64:
    case TokenType::UINT_SIZE:
    case TokenType::FLOAT_32:
    case TokenType::FLOAT_64:
    case TokenType::VOID:
    case TokenType::STRING:
    case TokenType::CHAR:
    case TokenType::BOOL:
    case TokenType::ANY:
        return true;
    default:
        return false;
    }
}

constexpr bool isRegularType(TokenType type) {
    switch (type) {
    case TokenType::INT_8:
    case TokenType::INT_16:
    case TokenType::INT_32:
    case TokenType::INT_64:
    case TokenType::INT_SIZE:
    case TokenType::UINT_8:
    case TokenType::UINT_16:
    case TokenType::UINT_32:
    case TokenType::UINT_64:
    case TokenType::UINT_SIZE:
    case TokenType::FLOAT_32:
    case TokenType::FLOAT_64:
    case TokenType::VOID:
    case TokenType::STRING:
    case TokenType::CHAR:
    case TokenType::BOOL:
    case TokenType::ANY:
    case TokenType::IDENTIFIER:
    case TokenType::DOLLAR:
        return true;
    default:
        return false;
    }
}

constexpr bool isRegularValue(TokenType type) {
    switch (type) {
    case TokenType::V_INT:
    case TokenType::V_FLOAT:
    case TokenType::V_CHAR:
    case TokenType::V_MULTILINE_STRING:
    case TokenType::V_HEX:
    case TokenType::V_OCTAL:
    case TokenType::V_BINARY:
    case TokenType::V_STRING:
    case TokenType::TRUE:
    case TokenType::FALSE:
    case TokenType::NIL:
    case TokenType::IDENTIFIER:
    case TokenType::SELF:
    case TokenType::OPERATOR_SUB:
    case TokenType::TYPEOF:
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
    case TokenType::AT:
        return true;
    default:
        return false;
    }
}

constexpr bool isUnaryOperator(TokenType type) {
    switch (type) {
    case TokenType::OPERATOR_ADD:
    case TokenType::OPERATOR_SUB:
    case TokenType::AT:
    case TokenType::BITWISE_NOT:
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

    explicit Token(TokenType type)
        : type(type), start(0), length(0), location(0, 0) {}

    static TokenType isKeyword(std::string_view string);

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
