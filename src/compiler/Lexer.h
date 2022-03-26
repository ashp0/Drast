//
// Created by Ashwin Paudel on 2022-03-20.
//

#ifndef DRAST_LEXER_H
#define DRAST_LEXER_H

#include "Print.h"
#include "Token.h"
#include "Types.h"
#include <iostream>
#include <vector>

class Lexer {
  private:
    std::string &source; // Maybe add support for unicode?

    Location location;

    uint32_t start;
    uint32_t index;

    Print printer;

  public:
    std::vector<Token> tokens;

  public:
    explicit Lexer(std::string &source, std::string &file_name)
        : source(source), start(0), index(0), printer(file_name, source) {
        location.line = 1;
        location.column = 1;
    }

    void lex();

  private:
    Token getToken();

    Token identifier();

    Token number();

    Token string();

    Token character();

    Token returnToken(TokenType type, bool without_advance = false);

    Token returnToken(TokenType first_type, TokenType second_type);

    void skipWhitespace();

    void skipLine();

    void skipBlockComment();

    void advance();

    char peek(size_t offset = 1);

    template <typename predicate>
    Token lexWhile(TokenType type, predicate &&pred, bool is_string = false);

    int throw_error(std::string message);

    char &current() const { return this->source[this->index]; }
};

#endif // DRAST_LEXER_H
