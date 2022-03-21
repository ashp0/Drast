//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "Lexer.h"

void Lexer::lex() {
	for (;;) {
		std::unique_ptr<Token> token = this->getToken();
		if (token->type == TokenType::T_EOF) {
			break;
		}

		this->tokens.push_back(std::move(token));
	}
}

std::unique_ptr<Token> Lexer::getToken() {
	this->skipWhitespace();

	switch (this->current) {
		case 'a' ... 'z':
		case 'A' ... 'Z':
		case '_':
			return this->identifier();
		case '0' ... '9':
			return this->number();
		case '"':
			return this->string();
		case '\'':
			return this->character();
		case ' ':
		case '\r':
		case '\t':
			break;
		case '\n':
			this->line++;
			this->column = 0;
			break;
		case '?':
			return returnToken(TokenType::T_QUESTION);
		case '<':
			if (this->peek() == '<') {
				this->advance();
				return this->returnToken(TokenType::T_BITWISE_SHIFT_LEFT,
				                         TokenType::T_BITWISE_SHIFT_LEFT_EQUAL);
			}

			return this->returnToken(TokenType::T_LESS_THAN, TokenType::T_LESS_THAN_EQUAL);
		case '>':
			if (this->peek() == '>') {
				this->advance();
				return this->returnToken(TokenType::T_BITWISE_SHIFT_RIGHT,
				                         TokenType::T_BITWISE_SHIFT_RIGHT_EQUAL);
			}

			return this->returnToken(TokenType::T_GREATER_THAN, TokenType::T_GREATER_THAN_EQUAL);
		case '=':
			return this->returnToken(TokenType::T_EQUAL, TokenType::T_EQUAL_EQUAL);
		case '!':
			return this->returnToken(TokenType::T_NOT, TokenType::T_NOT_EQUAL);
		case '+':
			return this->returnToken(TokenType::T_OPERATOR_ADD, TokenType::T_OPERATOR_ADD_EQUAL);
		case '-':
			return this->returnToken(TokenType::T_OPERATOR_SUB, TokenType::T_OPERATOR_SUB_EQUAL);
		case '*':
			return this->returnToken(TokenType::T_OPERATOR_MUL, TokenType::T_OPERATOR_MUL_EQUAL);
		case '/':
			if (peek() == '/') {
				this->skipLine();
				return this->getToken();
			} else if (peek() == '*') {
				this->skipBlockComment();
				return this->getToken();
			}
			return this->returnToken(TokenType::T_OPERATOR_DIV, TokenType::T_OPERATOR_DIV_EQUAL);
		case '%':
			return this->returnToken(TokenType::T_OPERATOR_MOD, TokenType::T_OPERATOR_MOD_EQUAL);

		case '&':
			if (peek() == '&') {
				this->advance();
				return this->returnToken(TokenType::T_BITWISE_AND_AND,
				                         TokenType::T_BITWISE_AND_AND_EQUAL);
			}
			return this->returnToken(TokenType::T_BITWISE_AND, TokenType::T_BITWISE_AND_EQUAL);
		case '|':
			if (peek() == '|') {
				this->advance();
				return this->returnToken(TokenType::T_BITWISE_PIPE_PIPE,
				                         TokenType::T_BITWISE_PIPE_PIPE_EQUAL);
			}
			return this->returnToken(TokenType::T_BITWISE_PIPE, TokenType::T_BITWISE_PIPE_EQUAL);
		case '^':
			return this->returnToken(TokenType::T_BITWISE_POWER, TokenType::T_BITWISE_POWER_EQUAL);
		case '~':
			return this->returnToken(TokenType::T_BITWISE_NOT);
		case ':':
			if (peek() == ':') {
				this->advance();
				return this->returnToken(TokenType::T_DOUBLE_COLON);
			}
			return this->returnToken(TokenType::T_COLON);
		case ';':
			return this->returnToken(TokenType::T_SEMICOLON);
		case '(':
			return this->returnToken(TokenType::T_PARENS_OPEN);
		case ')':
			return this->returnToken(TokenType::T_PARENS_CLOSE);
		case '[':
			return this->returnToken(TokenType::T_SQUARE_OPEN);
		case ']':
			return this->returnToken(TokenType::T_SQUARE_CLOSE);
		case '{':
			return this->returnToken(TokenType::T_BRACE_OPEN);
		case '}':
			return this->returnToken(TokenType::T_BRACE_CLOSE);
		case ',':
			return this->returnToken(TokenType::T_COMMA);
		case '.':
			return this->returnToken(TokenType::T_PERIOD);
		case '$':
			return this->returnToken(TokenType::T_DOLLAR);
		case '#':
			return this->returnToken(TokenType::T_HASHTAG);
		case '@':
			return this->returnToken(TokenType::T_AT);
		case '\\':
			return this->returnToken(TokenType::T_BACKSLASH);
		case '\0':
			break;
		default:
			std::cout << "ERROR" << std::endl;
			exit(EXIT_FAILURE);
	}

	return returnToken(TokenType::T_EOF);
}

std::unique_ptr<Token> Lexer::identifier() {
	return this->lexWhile(TokenType::T_IDENTIFIER,
	                      [this]() { return isalnum(this->current) || this->current == '_'; });
}

std::unique_ptr<Token> Lexer::number() {
	return this->lexWhile(TokenType::T_NUMBER, [this]() { return isnumber(this->current); });
}

std::unique_ptr<Token> Lexer::string() {
	return this->lexWhile(TokenType::T_STRING, [this]() { return (this->current != '"'); }, true);
}

std::unique_ptr<Token> Lexer::character() {
	return this->lexWhile(TokenType::T_CHAR, [this]() { return (this->current != '\''); }, true);
}

std::unique_ptr<Token> Lexer::returnToken(TokenType type) {
	std::string current_string = std::string(1, this->current);
	return this->returnToken(type, current_string);
}

std::unique_ptr<Token> Lexer::returnToken(TokenType type, std::string &string, bool without_advance) {
	std::unique_ptr<Token> return_token = std::make_unique<Token>(string, type, this->line, this->column);
	if (!without_advance) {
		this->advance();
	}
	return return_token;
}

std::unique_ptr<Token> Lexer::returnToken(TokenType first_type, TokenType second_type) {
	if (this->peek() == '=') {
		return this->returnToken(second_type);
	}

	return this->returnToken(first_type);
}

void Lexer::skipWhitespace() {
	while (isspace(this->current)) {
		if (this->current == '\n') {
			this->line += 1;
			this->column = 0;
		}
		if (this->current == '\0') {
			break;
		}
		this->advance();
	}
}

void Lexer::skipLine() {
	for (;;) {
		if (this->current == '\n' || this->current == '\0') {
			break;
		}

		this->advance();
	}

	this->line += 1;
	this->column = 0;
}

void Lexer::skipBlockComment() {
	this->advance();
	this->advance();

	for (;;) {
		if (this->current == '*') {
			if (peek() == '/') {
				this->advance();
				this->advance();
				break;
			}
		}

		if (this->current == '\0') {
			// Show error message
			break;
		}

		this->advance();
	}
}

void Lexer::advance() {
	this->column += 1;
	this->index += 1;
	this->current = this->source[this->index];
}

char Lexer::peek(size_t offset) {
	return this->source[this->index + offset];
}

template<typename predicate>
std::unique_ptr<Token> Lexer::lexWhile(TokenType type, predicate &&pred, bool is_string) {
	if (is_string) {
		this->advance();
	}

	this->start = this->index;

	while (pred()) {
		if (this->current == '\0') {
			break;
		}
		if (this->current == '\n') {
			this->line += 1;
			this->column = 0;
		}
		if (is_string && this->current == '\\') {
			this->advance();
			this->advance();
		}

		this->advance();
	}

	if (is_string) {
		this->advance();
	}

	size_t position = is_string ? this->index - this->start - 1 : this->index - this->start;
	std::string value = this->source.substr(this->start, position);

	if (type == TokenType::T_IDENTIFIER) {
		TokenType type1 = Token::is_keyword(value, this->index - this->start);
		return this->returnToken(type1, value, true);
	}
	return this->returnToken(type, value, true);
}
