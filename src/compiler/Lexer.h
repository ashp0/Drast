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
    const std::string &source; // Maybe add support for unicode?

    Location location;

    size_t start;
    size_t index;
    char current;

    Print printer;

  public:
    std::vector<std::unique_ptr<Token>> tokens;

  public:
    explicit Lexer(std::string &source, std::string &file_name)
        : source(source), start(0), index(0), current(source[0]),
          printer(source, file_name) {
        location.line = 1;
        location.column = 0;
    }

    void lex();

  private:
    std::unique_ptr<Token> getToken();

    std::unique_ptr<Token> identifier();

    std::unique_ptr<Token> number();

    std::unique_ptr<Token> string();

    std::unique_ptr<Token> character();

    std::unique_ptr<Token> returnToken(TokenType type,
                                       bool without_advance = false);

    std::unique_ptr<Token> returnToken(TokenType type, std::string &string,
                                       bool without_advance = false);

    std::unique_ptr<Token> returnToken(TokenType first_type,
                                       TokenType second_type);

    void skipWhitespace();

    void skipLine();

    void skipBlockComment();

    void advance();

    char peek(size_t offset = 1);

    template <typename predicate>
    std::unique_ptr<Token> lexWhile(TokenType type, predicate &&pred,
                                    bool is_string = false);

    int throw_error(std::string message);
};

#endif // DRAST_LEXER_H
