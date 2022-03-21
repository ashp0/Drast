//
// Created by Ashwin Paudel on 2022-03-20.
//

#ifndef DRAST_LEXER_H
#define DRAST_LEXER_H

#include <iostream>
#include <vector>
#include "Token.h"

class Lexer {
private:
	std::string source; // Support for unicode

	size_t line;
	size_t column;

	size_t start;
	size_t index;
	char current;

public:
	std::vector<std::unique_ptr<Token>> tokens;

public:
	explicit Lexer(const std::string &source) : source(source), line(1), column(0), start(0), index(0),
	                                            current(source[0]) {}

	void lex();

protected:
	std::unique_ptr<Token> getToken();

	std::unique_ptr<Token> identifier();

	std::unique_ptr<Token> number();

	std::unique_ptr<Token> string();

	std::unique_ptr<Token> character();

	std::unique_ptr<Token> returnToken(TokenType type);

	std::unique_ptr<Token> returnToken(TokenType type, std::string &string, bool without_advance = false);

	std::unique_ptr<Token> returnToken(TokenType first_type, TokenType second_type);

	void skipWhitespace();

	void skipLine();

	void skipBlockComment();

	void advance();

	char peek(size_t offset = 1);

	template<typename predicate>
	std::unique_ptr<Token> lexWhile(TokenType type, predicate &&pred, bool is_string = false);
};


#endif //DRAST_LEXER_H
