//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "Token.h"

static const std::unordered_map<std::string_view, TokenType> keywords = {
    {"struct", TokenType::STRUCT},
    {"self", TokenType::SELF},
    {"enum", TokenType::ENUM},
    {"typealias", TokenType::TYPEALIAS},
    {"return", TokenType::RETURN},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"import", TokenType::IMPORT},
    {"asm", TokenType::ASM},
    {"volatile", TokenType::VOLATILE},
    {"cast", TokenType::CAST},
    {"extern", TokenType::EXTERN},

    {"switch", TokenType::SWITCH},
    {"case", TokenType::CASE},
    {"break", TokenType::BREAK},
    {"default", TokenType::DEFAULT},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"continue", TokenType::CONTINUE},
    {"union", TokenType::UNION},

    {"false", TokenType::FALSE},
    {"true", TokenType::TRUE},
    {"bool", TokenType::BOOL},
    {"int", TokenType::INT},
    {"float", TokenType::FLOAT},
    {"void", TokenType::VOID},
    {"string", TokenType::STRING},
    {"char", TokenType::CHAR},
    {"nil", TokenType::NIL},

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
        return "STRUCT";
    case TokenType::SELF:
        return "SELF";
    case TokenType::ENUM:
        return "ENUM";
    case TokenType::TYPEALIAS:
        return "TYPEALIAS";
    case TokenType::RETURN:
        return "RETURN";
    case TokenType::IF:
        return "IF";
    case TokenType::ELSE:
        return "ELSE";
    case TokenType::IMPORT:
        return "IMPORT";
    case TokenType::ASM:
        return "ASM";
    case TokenType::VOLATILE:
        return "VOLATILE";
    case TokenType::CAST:
        return "CAST";
    case TokenType::EXTERN:
        return "EXTERN";

    case TokenType::SWITCH:
        return "SWITCH";
    case TokenType::CASE:
        return "CASE";
    case TokenType::BREAK:
        return "BREAK";
    case TokenType::DEFAULT:
        return "DEFAULT";
    case TokenType::WHILE:
        return "WHILE";
    case TokenType::FOR:
        return "FOR";
    case TokenType::CONTINUE:
        return "CONTINUE";
    case TokenType::UNION:
        return "UNION";

    case TokenType::FALSE:
        return "FALSE";
    case TokenType::TRUE:
        return "TRUE";
    case TokenType::BOOL:
        return "BOOL";
    case TokenType::INT:
        return "INT";
    case TokenType::FLOAT:
        return "FLOAT";
    case TokenType::VOID:
        return "VOID";
    case TokenType::STRING:
        return "STRING";
    case TokenType::CHAR:
        return "CHAR";
    case TokenType::NIL:
        return "NIL";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";

    case TokenType::V_INT:
        return "V_INT";
    case TokenType::V_FLOAT:
        return "V_FLOAT";
    case TokenType::V_CHAR:
        return "V_CHAR";
    case TokenType::V_STRING:
        return "V_STRING";
    case TokenType::V_HEX:
        return "V_HEX";
    case TokenType::V_OCTAL:
        return "V_OCTAL";

    case TokenType::GOTO:
        return "GOTO";
    case TokenType::PRIVATE:
        return "PRIVATE";

    case TokenType::DO:
        return "DO";
    case TokenType::TRY:
        return "TRY";
    case TokenType::CATCH:
        return "CATCH";
    case TokenType::THROW:
        return "THROW";

    case TokenType::QUESTION:
        return "QUESTION";

    case TokenType::LESS_THAN:
        return "LESS_THAN";
    case TokenType::LESS_THAN_EQUAL:
        return "LESS_THAN_EQUAL";

    case TokenType::GREATER_THAN:
        return "GREATER_THAN";
    case TokenType::GREATER_THAN_EQUAL:
        return "GREATER_THAN_EQUAL";

    case TokenType::EQUAL:
        return "EQUAL";
    case TokenType::EQUAL_EQUAL:
        return "EQUAL_EQUAL";

    case TokenType::NOT:
        return "NOT";
    case TokenType::NOT_EQUAL:
        return "NOT_EQUAL";

    case TokenType::OPERATOR_ADD:
        return "OPERATOR_ADD";
    case TokenType::OPERATOR_ADD_EQUAL:
        return "OPERATOR_ADD_EQUAL";

    case TokenType::OPERATOR_SUB:
        return "OPERATOR_SUB";
    case TokenType::OPERATOR_SUB_EQUAL:
        return "OPERATOR_SUB_EQUAL";

    case TokenType::OPERATOR_MUL:
        return "OPERATOR_MUL";
    case TokenType::OPERATOR_MUL_EQUAL:
        return "OPERATOR_MUL_EQUAL";

    case TokenType::OPERATOR_DIV:
        return "OPERATOR_DIV";
    case TokenType::OPERATOR_DIV_EQUAL:
        return "OPERATOR_DIV_EQUAL";

    case TokenType::OPERATOR_MOD:
        return "OPERATOR_MOD";
    case TokenType::OPERATOR_MOD_EQUAL:
        return "OPERATOR_MOD_EQUAL";

    case TokenType::BITWISE_AND:
        return "BITWISE_AND";
    case TokenType::BITWISE_AND_EQUAL:
        return "BITWISE_AND_EQUAL";
    case TokenType::BITWISE_AND_AND:
        return "BITWISE_AND_AND";
    case TokenType::BITWISE_AND_AND_EQUAL:
        return "BITWISE_AND_AND_EQUAL";

    case TokenType::BITWISE_PIPE:
        return "BITWISE_PIPE";
    case TokenType::BITWISE_PIPE_EQUAL:
        return "BITWISE_PIPE_EQUAL";
    case TokenType::BITWISE_PIPE_PIPE:
        return "BITWISE_PIPE_PIPE";
    case TokenType::BITWISE_PIPE_PIPE_EQUAL:
        return "BITWISE_PIPE_PIPE_EQUAL";

    case TokenType::BITWISE_SHIFT_LEFT:
        return "BITWISE_SHIFT_LEFT";
    case TokenType::BITWISE_SHIFT_LEFT_EQUAL:
        return "BITWISE_SHIFT_LEFT_EQUAL";

    case TokenType::BITWISE_SHIFT_RIGHT:
        return "BITWISE_SHIFT_RIGHT";
    case TokenType::BITWISE_SHIFT_RIGHT_EQUAL:
        return "BITWISE_SHIFT_RIGHT_EQUAL";

    case TokenType::BITWISE_POWER:
        return "BITWISE_POWER";
    case TokenType::BITWISE_POWER_EQUAL:
        return "BITWISE_POWER_EQUAL";
    case TokenType::BITWISE_NOT:
        return "BITWISE_NOT";

    case TokenType::COLON:
        return "COLON";
    case TokenType::DOUBLE_COLON:
        return "DOUBLE_COLON";
    case TokenType::SEMICOLON:
        return "SEMICOLON";
    case TokenType::PARENS_OPEN:
        return "PARENS_OPEN";
    case TokenType::PARENS_CLOSE:
        return "PARENS_CLOSE";
    case TokenType::BRACE_OPEN:
        return "BRACE_OPEN";
    case TokenType::BRACE_CLOSE:
        return "BRACE_CLOSE";
    case TokenType::SQUARE_OPEN:
        return "SQUARE_OPEN";
    case TokenType::SQUARE_CLOSE:
        return "SQUARE_CLOSE";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::PERIOD:
        return "PERIOD";
    case TokenType::DOLLAR:
        return "DOLLAR";
    case TokenType::HASHTAG:
        return "HASHTAG";
    case TokenType::AT:
        return "AT";
    case TokenType::BACKSLASH:
        return "BACKSLASH";

    case TokenType::NEW_LINE:
        return "NEW_LINE";
    case TokenType::T_EOF:
        return "T_EOF";

    default:
        return "UNKNOWN TOKEN";
    }
}

TokenType Token::is_keyword(std::string_view string) {
    auto it = keywords.find(string);

    if (it == keywords.end()) {
        return TokenType::IDENTIFIER;
    } else {
        return it->second;
    }
}
