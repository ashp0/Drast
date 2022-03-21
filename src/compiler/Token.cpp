//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "Token.h"

static uint8_t string_compare(std::string &str, std::string str2, size_t len) {
	while (len--)
		if (str[len] != str2[len])
			return 1;
	return 0;
}

std::string tokenTypeAsLiteral(TokenType type) {
	switch (type) {
		// Keywords
		case TokenType::T_K_STRUCT:
			return "T_K_STRUCT";
		case TokenType::T_K_SELF:
			return "T_K_SELF";
		case TokenType::T_K_ENUM:
			return "T_K_ENUM";
		case TokenType::T_K_ALIAS:
			return "T_K_ALIAS";
		case TokenType::T_K_RETURN:
			return "T_K_RETURN";
		case TokenType::T_K_IF:
			return "T_K_IF";
		case TokenType::T_K_ELSE:
			return "T_K_ELSE";
		case TokenType::T_K_IMPORT:
			return "T_K_IMPORT";
		case TokenType::T_K_ASM:
			return "T_K_ASM";
		case TokenType::T_K_VOLATILE:
			return "T_K_VOLATILE";
		case TokenType::T_K_CAST:
			return "T_K_CAST";
		case TokenType::T_K_EXTERN:
			return "T_K_EXTERN";

		case TokenType::T_K_SWITCH:
			return "T_K_SWITCH";
		case TokenType::T_K_CASE:
			return "T_K_CASE";
		case TokenType::T_K_BREAK:
			return "T_K_BREAK";
		case TokenType::T_K_DEFAULT:
			return "T_K_DEFAULT";
		case TokenType::T_K_WHILE:
			return "T_K_WHILE";
		case TokenType::T_K_FOR:
			return "T_K_FOR";
		case TokenType::T_K_CONTINUE:
			return "T_K_CONTINUE";
		case TokenType::T_K_UNION:
			return "T_K_UNION";

		case TokenType::T_K_FALSE:
			return "T_K_FALSE";
		case TokenType::T_K_TRUE:
			return "T_K_TRUE";
		case TokenType::T_K_BOOL:
			return "T_K_BOOL";
		case TokenType::T_K_INT:
			return "T_K_INT";
		case TokenType::T_K_FLOAT:
			return "T_K_FLOAT";
		case TokenType::T_K_VOID:
			return "T_K_VOID";
		case TokenType::T_K_STRING:
			return "T_K_STRING";
		case TokenType::T_K_CHAR:
			return "T_K_CHAR";
		case TokenType::T_HEX:
			return "T_HEX";
		case TokenType::T_OCTAL:
			return "T_OCTAL";
		case TokenType::T_IDENTIFIER:
			return "T_IDENTIFIER";

		case TokenType::T_NUMBER:
			return "T_NUMBER";
		case TokenType::T_FLOAT:
			return "T_FLOAT";
		case TokenType::T_CHAR:
			return "T_CHAR";
		case TokenType::T_STRING:
			return "T_STRING";

		case TokenType::T_K_GOTO:
			return "T_K_GOTO";
		case TokenType::T_K_PRIVATE:
			return "T_K_PRIVATE";

		case TokenType::T_K_DO:
			return "T_K_DO";
		case TokenType::T_K_TRY:
			return "T_K_TRY";
		case TokenType::T_K_CATCH:
			return "T_K_CATCH";

		case TokenType::T_QUESTION:
			return "T_QUESTION";

		case TokenType::T_LESS_THAN:
			return "T_LESS_THAN";
		case TokenType::T_LESS_THAN_EQUAL:
			return "T_LESS_THAN_EQUAL";

		case TokenType::T_GREATER_THAN:
			return "T_GREATER_THAN";
		case TokenType::T_GREATER_THAN_EQUAL:
			return "T_GREATER_THAN_EQUAL";

		case TokenType::T_EQUAL:
			return "T_EQUAL";
		case TokenType::T_EQUAL_EQUAL:
			return "T_EQUAL_EQUAL";

		case TokenType::T_NOT:
			return "T_NOT";
		case TokenType::T_NOT_EQUAL:
			return "T_NOT_EQUAL";

		case TokenType::T_OPERATOR_ADD:
			return "T_OPERATOR_ADD";
		case TokenType::T_OPERATOR_ADD_EQUAL:
			return "T_OPERATOR_ADD_EQUAL";

		case TokenType::T_OPERATOR_SUB:
			return "T_OPERATOR_SUB";
		case TokenType::T_OPERATOR_SUB_EQUAL:
			return "T_OPERATOR_SUB_EQUAL";

		case TokenType::T_OPERATOR_MUL:
			return "T_OPERATOR_MUL";
		case TokenType::T_OPERATOR_MUL_EQUAL:
			return "T_OPERATOR_MUL_EQUAL";

		case TokenType::T_OPERATOR_DIV:
			return "T_OPERATOR_DIV";
		case TokenType::T_OPERATOR_DIV_EQUAL:
			return "T_OPERATOR_DIV_EQUAL";

		case TokenType::T_OPERATOR_MOD:
			return "T_OPERATOR_MOD";
		case TokenType::T_OPERATOR_MOD_EQUAL:
			return "T_OPERATOR_MOD_EQUAL";

		case TokenType::T_BITWISE_AND:
			return "T_BITWISE_AND";
		case TokenType::T_BITWISE_AND_EQUAL:
			return "T_BITWISE_AND_EQUAL";
		case TokenType::T_BITWISE_AND_AND:
			return "T_BITWISE_AND_AND";
		case TokenType::T_BITWISE_AND_AND_EQUAL:
			return "T_BITWISE_AND_AND_EQUAL";

		case TokenType::T_BITWISE_PIPE:
			return "T_BITWISE_PIPE";
		case TokenType::T_BITWISE_PIPE_EQUAL:
			return "T_BITWISE_PIPE_EQUAL";
		case TokenType::T_BITWISE_PIPE_PIPE:
			return "T_BITWISE_PIPE_PIPE";
		case TokenType::T_BITWISE_PIPE_PIPE_EQUAL:
			return "T_BITWISE_PIPE_PIPE_EQUAL";

		case TokenType::T_BITWISE_SHIFT_LEFT:
			return "T_BITWISE_SHIFT_LEFT";
		case TokenType::T_BITWISE_SHIFT_LEFT_EQUAL:
			return "T_BITWISE_SHIFT_LEFT_EQUAL";

		case TokenType::T_BITWISE_SHIFT_RIGHT:
			return "T_BITWISE_SHIFT_RIGHT";
		case TokenType::T_BITWISE_SHIFT_RIGHT_EQUAL:
			return "T_BITWISE_SHIFT_RIGHT_EQUAL";

		case TokenType::T_BITWISE_POWER:
			return "T_BITWISE_POWER";
		case TokenType::T_BITWISE_POWER_EQUAL:
			return "T_BITWISE_POWER_EQUAL";
		case TokenType::T_BITWISE_NOT:
			return "T_BITWISE_NOT";

		case TokenType::T_COLON:
			return "T_COLON";
		case TokenType::T_DOUBLE_COLON:
			return "T_DOUBLE_COLON";
		case TokenType::T_SEMICOLON:
			return "T_SEMICOLON";
		case TokenType::T_PARENS_OPEN:
			return "T_PARENS_OPEN";
		case TokenType::T_PARENS_CLOSE:
			return "T_PARENS_CLOSE";
		case TokenType::T_BRACE_OPEN:
			return "T_BRACE_OPEN";
		case TokenType::T_BRACE_CLOSE:
			return "T_BRACE_CLOSE";
		case TokenType::T_SQUARE_OPEN:
			return "T_SQUARE_OPEN";
		case TokenType::T_SQUARE_CLOSE:
			return "T_SQUARE_CLOSE";
		case TokenType::T_COMMA:
			return "T_COMMA";
		case TokenType::T_PERIOD:
			return "T_PERIOD";
		case TokenType::T_DOLLAR:
			return "T_DOLLAR";
		case TokenType::T_HASHTAG:
			return "T_HASHTAG";
		case TokenType::T_AT:
			return "T_AT";
		case TokenType::T_BACKSLASH:
			return "T_BACKSLASH";

		case TokenType::T_EOF:
			return "T_EOF";

		default:
			return "UNKNOWN TOKEN";
	}
}

TokenType Token::is_keyword(std::string &string, size_t length) {
	switch (length) {
		case 2: {
			if (string_compare(string, "if", length) == 0) {
				return TokenType::T_K_IF;
			} else if (string_compare(string, "do", length) == 0) {
				return TokenType::T_K_DO;
			}
		}
		case 3: {
			if (string_compare(string, "asm", length) == 0) {
				return TokenType::T_K_ASM;
			} else if (string_compare(string, "for", length) == 0) {
				return TokenType::T_K_FOR;
			} else if (string_compare(string, "int", length) == 0) {
				return TokenType::T_K_INT;
			} else if (string_compare(string, "try", length) == 0) {
				return TokenType::T_K_TRY;
			}
		}
		case 4: {
			if (string_compare(string, "self", length) == 0) {
				return TokenType::T_K_SELF;
			} else if (string_compare(string, "enum", length) == 0) {
				return TokenType::T_K_ENUM;
			} else if (string_compare(string, "else", length) == 0) {
				return TokenType::T_K_ELSE;
			} else if (string_compare(string, "cast", length) == 0) {
				return TokenType::T_K_CAST;
			} else if (string_compare(string, "case", length) == 0) {
				return TokenType::T_K_CASE;
			} else if (string_compare(string, "true", length) == 0) {
				return TokenType::T_K_TRUE;
			} else if (string_compare(string, "bool", length) == 0) {
				return TokenType::T_K_BOOL;
			} else if (string_compare(string, "void", length) == 0) {
				return TokenType::T_K_VOID;
			} else if (string_compare(string, "char", length) == 0) {
				return TokenType::T_K_CHAR;
			} else if (string_compare(string, "goto", length) == 0) {
				return TokenType::T_K_GOTO;
			}
		}
		case 5: {
			if (string_compare(string, "alias", length) == 0) {
				return TokenType::T_K_ALIAS;
			} else if (string_compare(string, "break", length) == 0) {
				return TokenType::T_K_BREAK;
			} else if (string_compare(string, "while", length) == 0) {
				return TokenType::T_K_WHILE;
			} else if (string_compare(string, "union", length) == 0) {
				return TokenType::T_K_UNION;
			} else if (string_compare(string, "false", length) == 0) {
				return TokenType::T_K_FALSE;
			} else if (string_compare(string, "float", length) == 0) {
				return TokenType::T_K_FLOAT;
			} else if (string_compare(string, "catch", length) == 0) {
				return TokenType::T_K_CATCH;
			}
		}
		case 6: {
			if (string_compare(string, "struct", length) == 0) {
				return TokenType::T_K_STRUCT;
			} else if (string_compare(string, "return", length) == 0) {
				return TokenType::T_K_RETURN;
			} else if (string_compare(string, "import", length) == 0) {
				return TokenType::T_K_IMPORT;
			} else if (string_compare(string, "switch", length) == 0) {
				return TokenType::T_K_SWITCH;
			} else if (string_compare(string, "string", length) == 0) {
				return TokenType::T_K_STRING;
			} else if (string_compare(string, "extern", length) == 0) {
				return TokenType::T_K_EXTERN;
			}
		}
		case 7: {
			if (string_compare(string, "default", length) == 0) {
				return TokenType::T_K_DEFAULT;
			} else if (string_compare(string, "private", length) == 0) {
				return TokenType::T_K_PRIVATE;
			}
		}
		case 8: {
			if (string_compare(string, "volatile", length) == 0) {
				return TokenType::T_K_VOLATILE;
			} else if (string_compare(string, "continue", length) == 0) {
				return TokenType::T_K_CONTINUE;
			}
		}
		default: {
			return TokenType::T_IDENTIFIER;
		}
	}
}