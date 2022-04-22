//
// Token.cpp
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

#include "Token.h"

namespace drast::lexer {

const LookupTable<std::string_view, TokenType> keywords = {
    {"struct", TokenType::STRUCT},
    {"self", TokenType::SELF},
    {"enum", TokenType::ENUM},
    {"func", TokenType::FUNC},
    {"var", TokenType::VAR},
    {"let", TokenType::LET},
    {"typealias", TokenType::TYPEALIAS},
    {"return", TokenType::RETURN},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"import", TokenType::IMPORT},
    {"asm", TokenType::ASM},
    {"volatile", TokenType::VOLATILE},
    {"cast", TokenType::CAST},
    {"extern", TokenType::EXTERN},
    {"operator", TokenType::OPERATOR},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"nil", TokenType::NIL},

    {"i8", TokenType::INT_8},
    {"i16", TokenType::INT_16},
    {"i32", TokenType::INT_32},
    {"i64", TokenType::INT_64},
    {"isize", TokenType::INT_SIZE},
    {"u8", TokenType::UINT_8},
    {"u16", TokenType::UINT_16},
    {"u32", TokenType::UINT_32},
    {"u64", TokenType::UINT_64},
    {"usize", TokenType::UINT_SIZE},
    {"f32", TokenType::FLOAT_32},
    {"f64", TokenType::FLOAT_64},
    {"void", TokenType::VOID},
    {"string", TokenType::STRING},
    {"char", TokenType::CHAR},
    {"bool", TokenType::BOOL},
    {"any", TokenType::ANY},

    {"switch", TokenType::SWITCH},
    {"case", TokenType::CASE},
    {"break", TokenType::BREAK},
    {"default", TokenType::DEFAULT},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"continue", TokenType::CONTINUE},
    {"union", TokenType::UNION},
    {"typeof", TokenType::TYPEOF},
    {"in", TokenType::IN},

    {"goto", TokenType::GOTO},
    {"private", TokenType::PRIVATE},
    {"do", TokenType::DO},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"throw", TokenType::THROW},
};

std::string tokenTypeAsLiteral(TokenType type) {
    switch (type) {
    case TokenType::STRUCT:
        return "struct";
    case TokenType::SELF:
        return "self";
    case TokenType::ENUM:
        return "enum";
    case TokenType::FUNC:
        return "func";
    case TokenType::VAR:
        return "var";
    case TokenType::LET:
        return "let";
    case TokenType::TYPEALIAS:
        return "typealias";
    case TokenType::RETURN:
        return "return";
    case TokenType::IF:
        return "if";
    case TokenType::ELSE:
        return "else";
    case TokenType::IMPORT:
        return "import";
    case TokenType::ASM:
        return "asm";
    case TokenType::VOLATILE:
        return "volatile";
    case TokenType::CAST:
        return "cast";
    case TokenType::EXTERN:
        return "extern";
    case TokenType::OPERATOR:
        return "operator";
    case TokenType::TRUE:
        return "true";
    case TokenType::FALSE:
        return "false";
    case TokenType::NIL:
        return "nil";

    case TokenType::INT_8:
        return "i8";
    case TokenType::INT_16:
        return "i16";
    case TokenType::INT_32:
        return "i32";
    case TokenType::INT_64:
        return "i64";
    case TokenType::INT_SIZE:
        return "isize";
    case TokenType::UINT_8:
        return "u8";
    case TokenType::UINT_16:
        return "u16";
    case TokenType::UINT_32:
        return "u32";
    case TokenType::UINT_64:
        return "u64";
    case TokenType::UINT_SIZE:
        return "usize";
    case TokenType::FLOAT_32:
        return "f32";
    case TokenType::FLOAT_64:
        return "f64";
    case TokenType::VOID:
        return "void";
    case TokenType::STRING:
        return "string";
    case TokenType::CHAR:
        return "char";
    case TokenType::BOOL:
        return "bool";
    case TokenType::ANY:
        return "any";

    case TokenType::SWITCH:
        return "switch";
    case TokenType::CASE:
        return "case";
    case TokenType::BREAK:
        return "break";
    case TokenType::DEFAULT:
        return "default";
    case TokenType::WHILE:
        return "while";
    case TokenType::FOR:
        return "for";
    case TokenType::CONTINUE:
        return "continue";
    case TokenType::UNION:
        return "union";
    case TokenType::TYPEOF:
        return "typeof";
    case TokenType::IN:
        return "in";

    case TokenType::GOTO:
        return "goto";
    case TokenType::PRIVATE:
        return "private";
    case TokenType::DO:
        return "do";
    case TokenType::TRY:
        return "try";
    case TokenType::CATCH:
        return "catch";
    case TokenType::THROW:
        return "throw";

    case TokenType::V_INT:
        return "v_int";
    case TokenType::V_FLOAT:
        return "v_float";
    case TokenType::V_STRING:
        return "v_string";
    case TokenType::V_CHAR:
        return "v_char";
    case TokenType::V_MULTILINE_STRING:
        return "v_multiline_string";
    case TokenType::V_HEX:
        return "v_hex";
    case TokenType::V_OCTAL:
        return "v_octal";
    case TokenType::V_BINARY:
        return "v_binary";
    case TokenType::IDENTIFIER:
        return "identifier";

    case TokenType::QUESTION:
        return "?";

    case TokenType::LESS_THAN:
        return "<";
    case TokenType::LESS_THAN_EQUAL:
        return "<=";

    case TokenType::GREATER_THAN:
        return ">";
    case TokenType::GREATER_THAN_EQUAL:
        return ">=";

    case TokenType::EQUAL:
        return "=";
    case TokenType::EQUAL_EQUAL:
        return "==";

    case TokenType::NOT:
        return "!";
    case TokenType::NOT_EQUAL:
        return "!=";

    case TokenType::OPERATOR_ADD:
        return "+";
    case TokenType::OPERATOR_ADD_EQUAL:
        return "+=";

    case TokenType::OPERATOR_SUB:
        return "-";
    case TokenType::OPERATOR_SUB_EQUAL:
        return "-=";

    case TokenType::OPERATOR_MUL:
        return "*";
    case TokenType::OPERATOR_MUL_EQUAL:
        return "*=";

    case TokenType::OPERATOR_DIV:
        return "/";
    case TokenType::OPERATOR_DIV_EQUAL:
        return "/=";

    case TokenType::OPERATOR_MOD:
        return "%";
    case TokenType::OPERATOR_MOD_EQUAL:
        return "%=";

    case TokenType::BITWISE_AND:
        return "&";
    case TokenType::BITWISE_AND_EQUAL:
        return "&=";
    case TokenType::BITWISE_AND_AND:
        return "&&";
    case TokenType::BITWISE_AND_AND_EQUAL:
        return "&&=";

    case TokenType::BITWISE_PIPE:
        return "|";
    case TokenType::BITWISE_PIPE_EQUAL:
        return "|=";
    case TokenType::BITWISE_PIPE_PIPE:
        return "||";
    case TokenType::BITWISE_PIPE_PIPE_EQUAL:
        return "||=";

    case TokenType::BITWISE_SHIFT_LEFT:
        return "<<";
    case TokenType::BITWISE_SHIFT_LEFT_EQUAL:
        return "<<=";

    case TokenType::BITWISE_SHIFT_RIGHT:
        return ">>";
    case TokenType::BITWISE_SHIFT_RIGHT_EQUAL:
        return ">>=";

    case TokenType::BITWISE_POWER:
        return "^";
    case TokenType::BITWISE_POWER_EQUAL:
        return "^=";
    case TokenType::BITWISE_NOT:
        return "~";

    case TokenType::COLON:
        return ":";
    case TokenType::DOUBLE_COLON:
        return "::";
    case TokenType::SEMICOLON:
        return ";";
    case TokenType::PARENS_OPEN:
        return "(";
    case TokenType::PARENS_CLOSE:
        return ")";
    case TokenType::BRACE_OPEN:
        return "{";
    case TokenType::BRACE_CLOSE:
        return "}";
    case TokenType::SQUARE_OPEN:
        return "[";
    case TokenType::SQUARE_CLOSE:
        return "]";
    case TokenType::COMMA:
        return ",";
    case TokenType::PERIOD:
        return ".";
    case TokenType::DOLLAR:
        return "$";
    case TokenType::HASHTAG:
        return "#";
    case TokenType::AT:
        return "@";
    case TokenType::BACKSLASH:
        return "\\";

    case TokenType::NEW_LINE:
        return "\n";
    case TokenType::T_EOF:
        return "T_EOF";

    default:
        return "UNKNOWN TOKEN";
    }
}

TokenType Token::isKeyword(std::string_view string) {
    auto it = keywords.find(string);

    return it == keywords.end() ? TokenType::IDENTIFIER : it->second;
}

} // namespace drast::lexer