//
// Created by Ashwin Paudel on 2022-04-20.
//

#ifndef DRAST_TOKENUTILS_H
#define DRAST_TOKENUTILS_H

#include "Token.h"

namespace drast::lexer {
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

} // namespace drast::lexer

#endif // DRAST_TOKENUTILS_H
