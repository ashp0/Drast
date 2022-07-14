//
// Created by Ashwin Paudel on 2022-06-01.
//

#ifndef DRAST_LEXER_H
#define DRAST_LEXER_H

#include <string>
#include <iostream>
#include "Token.h"
#include "../Common/Location.h"

class Lexer {
private:
    const std::string &source;
    Location location;

    size_t start = 0;
    size_t current = 0;
public:
    explicit Lexer(const std::string &source);

    Token getToken();

    bool isAtEnd();

private:
    Token identifier();

    Token number();

    Token string();

    Token character();

    inline Token makeToken(TokenType type, const std::string &literal);

    inline Token makeToken(TokenType type);

    inline void skipLine();

    void skipBlockComment();

    bool matchNext(char c);

    inline char peek(size_t offset = 1);

    inline void advance();

    Token handleTab();
};


#endif //DRAST_LEXER_H
