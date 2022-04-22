//
// Token.h
// Created by Ashwin Paudel on 2022-03-20.
//
// =============================================================================
//
// Contributed by:
//  - Ashwin Paudel <ashwonixer123@gmail.com>
//
// =============================================================================
///
/// \file
/// This file contains the declaration of the Token, which are used
/// in the Drast programming language.
///
// =============================================================================
//
// Copyright (c) 2022, Drast Programming Language Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
//
// =============================================================================

#ifndef DRAST_TOKEN_H
#define DRAST_TOKEN_H

#include "../Common/LookupTable.h"
#include "../Common/Types.h"
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace drast::lexer {

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

} // namespace drast::lexer

#endif // DRAST_TOKEN_H
