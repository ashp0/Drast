//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "Token.h"

std::string tokenTypeAsLiteral(TokenType type) {
    switch (type) {
    case TokenType::STRUCT:
        return "STRUCT";
    case TokenType::SELF:
        return "SELF";
    case TokenType::ENUM:
        return "ENUM";
    case TokenType::ALIAS:
        return "ALIAS";
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
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";

    case TokenType::V_NUMBER:
        return "V_NUMBER";
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

    case TokenType::T_EOF:
        return "T_EOF";

    default:
        return "UNKNOWN TOKEN";
    }
}

TokenType Token::is_keyword(const std::string &string, size_t length) {
    switch (length) {
    case 2: {
        if (string == "if") {
            return TokenType::IF;
        } else if (string == "do") {
            return TokenType::DO;
        }
    }
    case 3: {
        if (string == "asm") {
            return TokenType::ASM;
        } else if (string == "for") {
            return TokenType::FOR;
        } else if (string == "int") {
            return TokenType::INT;
        } else if (string == "try") {
            return TokenType::TRY;
        }
    }
    case 4: {
        if (string == "self") {
            return TokenType::SELF;
        } else if (string == "enum") {
            return TokenType::ENUM;
        } else if (string == "else") {
            return TokenType::ELSE;
        } else if (string == "cast") {
            return TokenType::CAST;
        } else if (string == "case") {
            return TokenType::CASE;
        } else if (string == "true") {
            return TokenType::TRUE;
        } else if (string == "bool") {
            return TokenType::BOOL;
        } else if (string == "void") {
            return TokenType::VOID;
        } else if (string == "char") {
            return TokenType::CHAR;
        } else if (string == "goto") {
            return TokenType::GOTO;
        }
    }
    case 5: {
        if (string == "alias") {
            return TokenType::ALIAS;
        } else if (string == "break") {
            return TokenType::BREAK;
        } else if (string == "while") {
            return TokenType::WHILE;
        } else if (string == "union") {
            return TokenType::UNION;
        } else if (string == "false") {
            return TokenType::FALSE;
        } else if (string == "float") {
            return TokenType::FLOAT;
        } else if (string == "catch") {
            return TokenType::CATCH;
        }
    }
    case 6: {
        if (string == "struct") {
            return TokenType::STRUCT;
        } else if (string == "return") {
            return TokenType::RETURN;
        } else if (string == "import") {
            return TokenType::IMPORT;
        } else if (string == "switch") {
            return TokenType::SWITCH;
        } else if (string == "string") {
            return TokenType::STRING;
        } else if (string == "extern") {
            return TokenType::EXTERN;
        }
    }
    case 7: {
        if (string == "default") {
            return TokenType::DEFAULT;
        } else if (string == "private") {
            return TokenType::PRIVATE;
        }
    }
    case 8: {
        if (string == "volatile") {
            return TokenType::VOLATILE;
        } else if (string == "continue") {
            return TokenType::CONTINUE;
        }
    }
    default: {
        return TokenType::IDENTIFIER;
    }
    }
}