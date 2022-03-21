//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "Token.h"

std::string tokenTypeAsLiteral(TokenType type) {
	switch (type) {
		// Keywords
		case TokenType::STRUCT:
			return "T_K_STRUCT";
		case TokenType::SELF:
			return "T_K_SELF";
		case TokenType::ENUM:
			return "T_K_ENUM";
		case TokenType::ALIAS:
			return "T_K_ALIAS";
		case TokenType::RETURN:
			return "T_K_RETURN";
		case TokenType::IF:
			return "T_K_IF";
		case TokenType::ELSE:
			return "T_K_ELSE";
		case TokenType::IMPORT:
			return "T_K_IMPORT";
		case TokenType::ASM:
			return "T_K_ASM";
		case TokenType::VOLATILE:
			return "T_K_VOLATILE";
		case TokenType::CAST:
			return "T_K_CAST";
		case TokenType::EXTERN:
			return "T_K_EXTERN";

		case TokenType::SWITCH:
			return "T_K_SWITCH";
		case TokenType::CASE:
			return "T_K_CASE";
		case TokenType::BREAK:
			return "T_K_BREAK";
		case TokenType::DEFAULT:
			return "T_K_DEFAULT";
		case TokenType::WHILE:
			return "T_K_WHILE";
		case TokenType::FOR:
			return "T_K_FOR";
		case TokenType::CONTINUE:
			return "T_K_CONTINUE";
		case TokenType::UNION:
			return "T_K_UNION";

		case TokenType::FALSE:
			return "T_K_FALSE";
		case TokenType::TRUE:
			return "T_K_TRUE";
		case TokenType::BOOL:
			return "T_K_BOOL";
		case TokenType::INT:
			return "T_K_INT";
		case TokenType::FLOAT:
			return "T_K_FLOAT";
		case TokenType::VOID:
			return "T_K_VOID";
		case TokenType::STRING:
			return "T_K_STRING";
		case TokenType::CHAR:
			return "T_K_CHAR";
		case TokenType::V_HEX:
			return "T_HEX";
		case TokenType::V_OCTAL:
			return "T_OCTAL";
		case TokenType::IDENTIFIER:
			return "T_IDENTIFIER";

		case TokenType::V_NUMBER:
			return "T_NUMBER";
		case TokenType::V_FLOAT:
			return "T_FLOAT";
		case TokenType::V_CHAR:
			return "T_CHAR";
		case TokenType::V_STRING:
			return "T_STRING";

		case TokenType::GOTO:
			return "T_K_GOTO";
		case TokenType::PRIVATE:
			return "T_K_PRIVATE";

		case TokenType::DO:
			return "T_K_DO";
		case TokenType::TRY:
			return "T_K_TRY";
		case TokenType::CATCH:
			return "T_K_CATCH";

		case TokenType::QUESTION:
			return "T_QUESTION";

		case TokenType::LESS_THAN:
			return "T_LESS_THAN";
		case TokenType::LESS_THAN_EQUAL:
			return "T_LESS_THAN_EQUAL";

		case TokenType::GREATER_THAN:
			return "T_GREATER_THAN";
		case TokenType::GREATER_THAN_EQUAL:
			return "T_GREATER_THAN_EQUAL";

		case TokenType::EQUAL:
			return "T_EQUAL";
		case TokenType::EQUAL_EQUAL:
			return "T_EQUAL_EQUAL";

		case TokenType::NOT:
			return "T_NOT";
		case TokenType::NOT_EQUAL:
			return "T_NOT_EQUAL";

		case TokenType::OPERATOR_ADD:
			return "T_OPERATOR_ADD";
		case TokenType::OPERATOR_ADD_EQUAL:
			return "T_OPERATOR_ADD_EQUAL";

		case TokenType::OPERATOR_SUB:
			return "T_OPERATOR_SUB";
		case TokenType::OPERATOR_SUB_EQUAL:
			return "T_OPERATOR_SUB_EQUAL";

		case TokenType::OPERATOR_MUL:
			return "T_OPERATOR_MUL";
		case TokenType::OPERATOR_MUL_EQUAL:
			return "T_OPERATOR_MUL_EQUAL";

		case TokenType::OPERATOR_DIV:
			return "T_OPERATOR_DIV";
		case TokenType::OPERATOR_DIV_EQUAL:
			return "T_OPERATOR_DIV_EQUAL";

		case TokenType::OPERATOR_MOD:
			return "T_OPERATOR_MOD";
		case TokenType::OPERATOR_MOD_EQUAL:
			return "T_OPERATOR_MOD_EQUAL";

		case TokenType::BITWISE_AND:
			return "T_BITWISE_AND";
		case TokenType::BITWISE_AND_EQUAL:
			return "T_BITWISE_AND_EQUAL";
		case TokenType::BITWISE_AND_AND:
			return "T_BITWISE_AND_AND";
		case TokenType::BITWISE_AND_AND_EQUAL:
			return "T_BITWISE_AND_AND_EQUAL";

		case TokenType::BITWISE_PIPE:
			return "T_BITWISE_PIPE";
		case TokenType::BITWISE_PIPE_EQUAL:
			return "T_BITWISE_PIPE_EQUAL";
		case TokenType::BITWISE_PIPE_PIPE:
			return "T_BITWISE_PIPE_PIPE";
		case TokenType::BITWISE_PIPE_PIPE_EQUAL:
			return "T_BITWISE_PIPE_PIPE_EQUAL";

		case TokenType::BITWISE_SHIFT_LEFT:
			return "T_BITWISE_SHIFT_LEFT";
		case TokenType::BITWISE_SHIFT_LEFT_EQUAL:
			return "T_BITWISE_SHIFT_LEFT_EQUAL";

		case TokenType::BITWISE_SHIFT_RIGHT:
			return "T_BITWISE_SHIFT_RIGHT";
		case TokenType::BITWISE_SHIFT_RIGHT_EQUAL:
			return "T_BITWISE_SHIFT_RIGHT_EQUAL";

		case TokenType::BITWISE_POWER:
			return "T_BITWISE_POWER";
		case TokenType::BITWISE_POWER_EQUAL:
			return "T_BITWISE_POWER_EQUAL";
		case TokenType::BITWISE_NOT:
			return "T_BITWISE_NOT";

		case TokenType::COLON:
			return "T_COLON";
		case TokenType::DOUBLE_COLON:
			return "T_DOUBLE_COLON";
		case TokenType::SEMICOLON:
			return "T_SEMICOLON";
		case TokenType::PARENS_OPEN:
			return "T_PARENS_OPEN";
		case TokenType::PARENS_CLOSE:
			return "T_PARENS_CLOSE";
		case TokenType::BRACE_OPEN:
			return "T_BRACE_OPEN";
		case TokenType::BRACE_CLOSE:
			return "T_BRACE_CLOSE";
		case TokenType::SQUARE_OPEN:
			return "T_SQUARE_OPEN";
		case TokenType::SQUARE_CLOSE:
			return "T_SQUARE_CLOSE";
		case TokenType::COMMA:
			return "T_COMMA";
		case TokenType::PERIOD:
			return "T_PERIOD";
		case TokenType::DOLLAR:
			return "T_DOLLAR";
		case TokenType::HASHTAG:
			return "T_HASHTAG";
		case TokenType::AT:
			return "T_AT";
		case TokenType::BACKSLASH:
			return "T_BACKSLASH";

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