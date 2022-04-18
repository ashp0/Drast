//
// Created by Ashwin Paudel on 2022-03-20.
//

#ifndef DRAST_LEXER_H
#define DRAST_LEXER_H

#include "Error.h"
#include "Token.h"
#include "Types.h"
#include "Utils.h"
#include <fstream>
#include <iostream>
#include <vector>

class Lexer {
  private:
    Error error;
    bool did_encounter_error = false;

    Location location;

    uint32_t start = 0;

    std::string file_buffer;
    uint32_t buffer_index = 0;

  public:
    std::vector<Token> tokens;

  public:
    Lexer(std::string &file_name, Error &error);

    void lex();

    Token getToken();

  private:
    Token lexOperator();

    Token identifier();

    Token hexadecimal();

    Token octal();

    Token binary();

    Token number();

    Token string();

    Token character();

    Token multilineString();

    Token returnToken(size_t trim, TokenType type);

    Token returnToken(TokenType type, bool without_advance = false);

    bool equalAndAdvance();

    void advanceWhitespace();

    void advanceLineComment();

    void advanceBlockComment();

    void advance();

    void evaluateEscapeSequence();

    char peek(size_t offset = 1);

    template <typename predicate>
    Token lexWhile(TokenType type, predicate &&pred);

    Token throwError(const std::string &message);

    Token throwError(const std::string &message, Location &loc);

    [[nodiscard]] char &current() {
        return this->file_buffer[this->buffer_index];
    }
};

#endif // DRAST_LEXER_H
