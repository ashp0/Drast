#pragma once

#include <deque>
#include <string>
#include <vector>
#include "Token.h"

namespace drast {

class Lexer final {
public:
    explicit Lexer(std::string source, std::string fileName = "");

    std::vector<Token> lex();

private:
    std::string source_;
    size_t current_ = 0;
    SourceLocation location_;

    std::vector<int> indentStack_;
    std::deque<Token> pendingTokens_;
    bool atLineStart_ = true;
    bool inUseLine_ = false;  // suppresses keyword recognition for `.h` paths after `use`

    Token next();
    void processIndentation();
    int measureIndent();
    void skipWhitespaceInline();
    void skipLineComment();
    void skipBlockComment();

    Token identifier(SourceLocation start, size_t startOffset);
    Token number(SourceLocation start, size_t startOffset);
    Token quoted(SourceLocation start);
    Token make(TokenKind kind, std::string text, SourceLocation location) const;

    char peek(size_t offset = 0) const;
    bool match(char expected);
    void advance();
};

} // namespace drast
