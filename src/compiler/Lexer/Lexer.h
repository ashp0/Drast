//
// Created by Ashwin Paudel on 2022-03-20.
//

#ifndef DRAST_LEXER_H
#define DRAST_LEXER_H

#include "../Common/Error.h"
#include "../Common/Types.h"
#include "../Utils/Utils.h"
#include "Token.h"
#include <fstream>
#include <iostream>
#include <vector>

namespace drast::lexer {

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
    Lexer(const std::string &file_name, const Error &error);

    void lex();

    Token getToken();

  private:
    Token identifier();

    Token hexadecimal();

    Token octal();

    Token binary();

    Token number();

    Token string();

    Token character();

    Token multilineString();

    Token createOperator(TokenType type);

    Token createOperator(TokenType type, TokenType type2);

    Token returnToken(TokenType type, bool without_advance = false);

    void advanceWhitespace();

    void advanceLineComment();

    void advanceMultilineComment();

    void advanceLine();

    inline void advance();

    inline void advance(size_t offset);

    void evaluateEscapeSequence();

    inline char peek(size_t offset = 1);

    Token lexWhile(TokenType type, bool (*predicate)(char));

    Token throwError(const std::string &message);

    Token throwError(const std::string &message, const Location &loc);

    inline const char &current() const {
        return this->file_buffer[this->buffer_index];
    }
};

} // namespace drast::lexer

#endif // DRAST_LEXER_H
