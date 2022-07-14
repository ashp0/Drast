//
// Created by Ashwin Paudel on 2022-06-01.
//

#ifndef DRAST_TOKEN_H
#define DRAST_TOKEN_H

#include <cstdint>
#include <string>
#include <optional>
#include <unordered_map>
#include <sstream>
#include "../Common/Location.h"

enum class TokenType : uint8_t {
    // Keywords
    STRUCT,
    SELF,
    ENUM,
    FN, // Function
    LET, // Constant
    TYPE,
    RETURN,
    IF,
    ELIF,
    ELSE,
    WHILE,
    FOR,
    IN,
    BREAK,
    CONTINUE,
    IMPORT,

    // Values
    INT, // int
    FLOAT, // float
    STRING, // string
    CHAR,  // char
    BOOL,  // bool

    // Literal Values
    LV_INT, // 50
    LV_FLOAT, // 50.2
    LV_STRING, // "Hello, World!"
    LV_CHAR,   // 'c'
    LV_IDENTIFIER, // myVariable
    LV_TRUE, // true
    LV_FALSE, // false

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
    ARROW, // ->

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

    COLON,        // :
    DECLARE_EQUAL, // :=
    SEMICOLON,    // ;
    PARENS_OPEN,  // (
    PARENS_CLOSE, // )
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
    TAB,      // \t
    T_EOF,    // \0
};

// TokenType to string
std::string tokenTypeToString(TokenType type);

std::string tokenGenerate(TokenType type);

// Check if a string is a keyword and get the type
TokenType checkKeyword(const std::string &keyword);

bool isAssignmentOp(TokenType type);

bool isEqualityOp(TokenType type);

bool isComparisonOp(TokenType type);

bool isTermOp(TokenType type);

bool isFactorOp(TokenType type);

bool isUnaryOp(TokenType type);

bool isPrimaryType(TokenType type);

class Token {
public:
    TokenType type;
    std::optional<std::string> literal = std::nullopt;

    size_t start = -1;
    Location location;
    size_t tab_width = 0;
public:
    Token(TokenType type, const std::string &literal, size_t start, Location loc);

    Token(TokenType type, size_t start, Location loc);

    Token(TokenType type, size_t start, Location loc, size_t tab_width);

    [[nodiscard]] std::string toString() const;
};


#endif //DRAST_TOKEN_H
