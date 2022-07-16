//
// Created by Ashwin Paudel on 2022-06-01.
//

#include "Token.h"

const std::unordered_map<std::string, TokenType> keywords = {
        {"struct",   TokenType::STRUCT},
        {"self",     TokenType::SELF},
        {"enum",     TokenType::ENUM},
        {"fn",       TokenType::FN},
        {"let",      TokenType::LET},
        {"type",     TokenType::TYPE},
        {"return",   TokenType::RETURN},
        {"if",       TokenType::IF},
        {"elif",     TokenType::ELIF},
        {"else",     TokenType::ELSE},
        {"while",    TokenType::WHILE},
        {"for",      TokenType::FOR},
        {"in",       TokenType::IN},
        {"break",    TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
        {"import",   TokenType::IMPORT},
        {"true",     TokenType::LV_TRUE},
        {"false",    TokenType::LV_FALSE},

        {"int",      TokenType::INT},
        {"float",    TokenType::FLOAT},
        {"string",   TokenType::STRING},
        {"char",     TokenType::CHAR},
        {"bool",     TokenType::BOOL},
};

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::STRUCT:
            return "STRUCT";
        case TokenType::SELF:
            return "SELF";
        case TokenType::ENUM:
            return "ENUM";
        case TokenType::FN:
            return "FN";
        case TokenType::LET:
            return "LET";
        case TokenType::TYPE:
            return "TYPE";
        case TokenType::RETURN:
            return "RETURN";
        case TokenType::IF:
            return "IF";
        case TokenType::ELIF:
            return "ELIF";
        case TokenType::ELSE:
            return "ELSE";
        case TokenType::WHILE:
            return "WHILE";
        case TokenType::FOR:
            return "FOR";
        case TokenType::IN:
            return "IN";
        case TokenType::BREAK:
            return "BREAK";
        case TokenType::CONTINUE:
            return "CONTINUE";
        case TokenType::IMPORT:
            return "IMPORT";
        case TokenType::INT:
            return "INT";
        case TokenType::FLOAT:
            return "FLOAT";
        case TokenType::STRING:
            return "STRING";
        case TokenType::CHAR:
            return "CHAR";
        case TokenType::BOOL:
            return "BOOL";
        case TokenType::LV_INT:
            return "LV_INT";
        case TokenType::LV_FLOAT:
            return "LV_FLOAT";
        case TokenType::LV_STRING:
            return "LV_STRING";
        case TokenType::LV_CHAR:
            return "LV_CHAR";
        case TokenType::LV_TRUE:
            return "LV_TRUE";
        case TokenType::LV_FALSE:
            return "LV_FALSE";
        case TokenType::LV_IDENTIFIER:
            return "LV_IDENTIFIER";
        case TokenType::QUESTION:
            return "QUESTION";
        case TokenType::ARROW:
            return "ARROW";
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
        case TokenType::DECLARE_EQUAL:
            return "DECLARE_EQUAL";
        case TokenType::SEMICOLON:
            return "SEMICOLON";
        case TokenType::PARENS_OPEN:
            return "PARENS_OPEN";
        case TokenType::PARENS_CLOSE:
            return "PARENS_CLOSE";
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
        case TokenType::TAB:
            return "TAB";
        case TokenType::T_EOF:
            return "T_EOF";
        default:
            return "UNKNOWN";
    }
}

std::string tokenGenerate(TokenType type) {
    switch (type) {
        case TokenType::STRUCT:
            return "struct";
        case TokenType::SELF:
            return "self";
        case TokenType::ENUM:
            return "enum";
        case TokenType::FN:
            return "fn";
        case TokenType::LET:
            return "let";
        case TokenType::TYPE:
            return "type";
        case TokenType::RETURN:
            return "return";
        case TokenType::IF:
            return "if";
        case TokenType::ELIF:
            return "elif";
        case TokenType::ELSE:
            return "else";
        case TokenType::WHILE:
            return "while";
        case TokenType::FOR:
            return "for";
        case TokenType::IN:
            return "in";
        case TokenType::BREAK:
            return "break";
        case TokenType::CONTINUE:
            return "continue";
        case TokenType::IMPORT:
            return "import";
        case TokenType::INT:
            return "int";
        case TokenType::FLOAT:
            return "float";
        case TokenType::STRING:
            return "char*";
        case TokenType::CHAR:
            return "char";
        case TokenType::BOOL:
            return "int";
        case TokenType::LV_INT:
            return "int";
        case TokenType::LV_FLOAT:
            return "float";
        case TokenType::LV_STRING:
            return "char*";
        case TokenType::LV_CHAR:
            return "char";
        case TokenType::LV_TRUE:
            return "1";
        case TokenType::LV_FALSE:
            return "0";
        case TokenType::LV_IDENTIFIER:
            return "identifier";
        case TokenType::QUESTION:
            return "?";
        case TokenType::ARROW:
            return "->";
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
        case TokenType::DECLARE_EQUAL:
            return ":=";
        case TokenType::SEMICOLON:
            return ";";
        case TokenType::PARENS_OPEN:
            return "(";
        case TokenType::PARENS_CLOSE:
            return ")";
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
            return "\\n";
        case TokenType::TAB:
            return "\\t";
        case TokenType::T_EOF:
            return "\\0";
        default:
            return "unknown";
    }
}

TokenType checkKeyword(const std::string &keyword) {
    return keywords.find(keyword) != keywords.end() ? keywords.at(keyword) : TokenType::LV_IDENTIFIER;
}

bool isAssignmentOp(TokenType type) {
    switch (type) {
        case TokenType::EQUAL:
        case TokenType::OPERATOR_ADD_EQUAL:
        case TokenType::OPERATOR_SUB_EQUAL:
        case TokenType::OPERATOR_MUL_EQUAL:
        case TokenType::OPERATOR_DIV_EQUAL:
        case TokenType::OPERATOR_MOD_EQUAL:
            return true;
        default:
            return false;
    }
}

bool isEqualityOp(TokenType type) {
    switch (type) {
        case TokenType::NOT_EQUAL:
        case TokenType::EQUAL_EQUAL:
        case TokenType::BITWISE_PIPE_PIPE:
            return true;
        default:
            return false;
    }
}

bool isComparisonOp(TokenType type) {
    switch (type) {
        case TokenType::GREATER_THAN:
        case TokenType::GREATER_THAN_EQUAL:
        case TokenType::LESS_THAN:
        case TokenType::LESS_THAN_EQUAL:
            return true;
        default:
            return false;
    }
}

bool isTermOp(TokenType type) {
    switch (type) {
        case TokenType::OPERATOR_ADD:
        case TokenType::OPERATOR_SUB:
            return true;
        default:
            return false;
    }
}

bool isFactorOp(TokenType type) {
    switch (type) {
        case TokenType::OPERATOR_MUL:
        case TokenType::OPERATOR_DIV:
            return true;
        default:
            return false;
    }
}

bool isUnaryOp(TokenType type) {
    switch (type) {
        case TokenType::NOT:
        case TokenType::OPERATOR_SUB:
            return true;
        default:
            return false;
    }
}

bool isPrimaryType(TokenType type) {
    switch (type) {
        case TokenType::LV_INT:
        case TokenType::LV_FLOAT:
        case TokenType::LV_STRING:
        case TokenType::LV_IDENTIFIER:
        case TokenType::LV_CHAR:
        case TokenType::LV_TRUE:
        case TokenType::LV_FALSE:
            return true;
        default:
            return false;
    }
}

Token::Token(TokenType type, const std::string &literal, size_t start, Location loc) : type(type), literal(literal),
                                                                                       start(start), location(loc) {}

Token::Token(TokenType type, size_t start, Location loc) : type(type), start(start), location(loc) {}

Token::Token(TokenType type, size_t start, Location loc, size_t tab_width) : type(type), start(start), location(loc),
                                                                             tab_width(tab_width) {}

std::string Token::toString() const {
    std::stringstream ss;
    ss << tokenTypeToString(type);

    ss << " :: ";
    if (literal) {
        ss << "(" << literal.value() << ")";
        ss << " :: ";
    }
    ss << location.toString();

    return ss.str();
}
