//
// Created by Ashwin Paudel on 2022-06-01.
//

#include "Lexer.h"
#include "../Common/Report.h"

Lexer::Lexer(const std::string &source) : source(source), location(1, 1) {}

Token Lexer::getToken() {
    start = current;
    auto c = peek(0);

    switch (c) {
        case '\0':
            return makeToken(TokenType::T_EOF);
        case '\n':
            location.line++;
            location.column = 0;
            return makeToken(TokenType::NEW_LINE);
        case '\t':
            return handleTab();
        case ' ':
        case '\r':
            advance();
            return getToken();
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
            return identifier();
        case '0' ... '9':
            return number();
        case '"':
            return string();
        case '\'':
            return character();
        case '?':
            return makeToken(TokenType::QUESTION);
        case '<':
            if (matchNext('<')) {
                return makeToken(matchNext('=') ? TokenType::BITWISE_SHIFT_LEFT_EQUAL
                                                : TokenType::BITWISE_SHIFT_LEFT);
            }
            return makeToken(matchNext('=') ? TokenType::LESS_THAN_EQUAL : TokenType::LESS_THAN);
        case '>':
            if (matchNext('>')) {
                return makeToken(matchNext('=') ? TokenType::BITWISE_SHIFT_RIGHT_EQUAL
                                                : TokenType::BITWISE_SHIFT_RIGHT);
            }
            return makeToken(matchNext('=') ? TokenType::GREATER_THAN_EQUAL : TokenType::GREATER_THAN);
        case '=':
            return makeToken(matchNext('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        case '!':
            return makeToken(matchNext('=') ? TokenType::NOT_EQUAL : TokenType::NOT);
        case '+':
            return makeToken(matchNext('=') ? TokenType::OPERATOR_ADD_EQUAL : TokenType::OPERATOR_ADD);
        case '-':
            return makeToken(
                    matchNext('=') ? TokenType::OPERATOR_SUB_EQUAL : matchNext('>') ? TokenType::ARROW
                                                                                    : TokenType::OPERATOR_SUB);
        case '*':
            return makeToken(matchNext('=') ? TokenType::OPERATOR_MUL_EQUAL : TokenType::OPERATOR_MUL);
        case '/':
            if (matchNext('/')) {
                advance();
                skipLine();
                return getToken();
            }
            if (matchNext('*')) {
                skipBlockComment();
                return getToken();
            }
            return makeToken(matchNext('=') ? TokenType::OPERATOR_DIV_EQUAL : TokenType::OPERATOR_DIV);
        case '%':
            return makeToken(matchNext('=') ? TokenType::OPERATOR_MOD_EQUAL : TokenType::OPERATOR_MOD);
        case '&':
            if (matchNext('&')) {
                return makeToken(
                        matchNext('=') ? TokenType::BITWISE_AND_AND_EQUAL : TokenType::BITWISE_AND_AND);
            }
            return makeToken(matchNext('&') ? TokenType::BITWISE_AND : TokenType::BITWISE_AND_EQUAL);
        case '|':
            if (matchNext('|')) {
                return makeToken(matchNext('=') ? TokenType::BITWISE_PIPE_PIPE_EQUAL
                                                : TokenType::BITWISE_PIPE_PIPE);
            }
            return makeToken(matchNext('=') ? TokenType::BITWISE_PIPE_EQUAL : TokenType::BITWISE_PIPE);
        case '^':
            return makeToken(matchNext('=') ? TokenType::BITWISE_POWER_EQUAL : TokenType::BITWISE_POWER);
        case '~':
            return makeToken(TokenType::BITWISE_NOT);
        case ':':
            return makeToken(matchNext('=') ? TokenType::DECLARE_EQUAL : TokenType::COLON);
        case ';':
            return makeToken(TokenType::SEMICOLON);
        case '(':
            return makeToken(TokenType::PARENS_OPEN);
        case ')':
            return makeToken(TokenType::PARENS_CLOSE);
        case '[':
            return makeToken(TokenType::SQUARE_OPEN);
        case ']':
            return makeToken(TokenType::SQUARE_CLOSE);
        case ',':
            return makeToken(TokenType::COMMA);
        case '.':
            return makeToken(TokenType::PERIOD);
        case '$':
            return makeToken(TokenType::DOLLAR);
        case '#':
            return makeToken(TokenType::HASHTAG);
        case '@':
            return makeToken(TokenType::AT);
        case '\\':
            return makeToken(TokenType::BACKSLASH);
        default:
            throw Report("Unexpected character '" + std::string(1, c) + "'", location);
    }
}

Token Lexer::identifier() {
    while (isalnum(peek(0)) || peek(0) == '_') advance();

    auto text = source.substr(start, current - start);

    return makeToken(checkKeyword(text), text);
}

Token Lexer::number() {
    bool is_float = false;

    loop:
    while (isdigit(peek(0))) {
        advance();
    }

    if (peek(0) == '.' && isdigit(peek())) {
        if (!is_float) {
            is_float = true;
            advance();
            goto loop;
        }
        throw Report("Invalid number", location);
    }

    auto text = source.substr(start, current - start);

    return makeToken(is_float ? TokenType::LV_FLOAT : TokenType::LV_INT, text);
}

Token Lexer::string() {
    advance();
    auto temp_location = location;
    while (peek(0) != '\"') {
        auto c = peek(0);
        if (c == '\n' || c == '\0') {
            throw Report("Unterminated string literal", temp_location);
        }
        if (c == '\\') {
            advance();
            // Other advance is handled below
        }
        advance();
    }
    advance();

    auto text = source.substr(start + 1, current - start - 2);
    return makeToken(TokenType::LV_STRING, text);
}

Token Lexer::character() {
    advance();
    auto temp_location = location;
    auto c = peek(0);
    if (c == '\n' || c == '\0') {
        throw Report("Characters must be 1 character long", temp_location);
    }
    if (c == '\\') {
        advance();
        // Other advance is handled below
    }
    advance();
    advance();

    auto text = source.substr(start + 1, current - start - 2);
    return makeToken(TokenType::LV_CHAR, text);
}

inline Token Lexer::makeToken(TokenType type, const std::string &literal) {
    return {type, literal, start, location};
}

inline Token Lexer::makeToken(TokenType type) {
    advance();
    return {type, start, location};
}

inline void Lexer::skipLine() {
    while (!isAtEnd() && peek(0) != '\n') {
        advance();
    }
    location.column = 1;
}

void Lexer::skipBlockComment() {
    int nesting = 1;
    auto temp_location = location;
    while (nesting > 0) {
        if (peek(0) == '\0') {
            throw Report("Unterminated block comment", temp_location);
        }

        if (peek(0) == '\n') {
            location.line++;
            location.column = 0;
        }

        if (peek(0) == '/' && peek() == '*') {
            advance();
            ++nesting;
        } else if (peek(0) == '*' && peek() == '/') {
            advance();
            --nesting;
        }

        advance();
    }
}

bool Lexer::matchNext(char c) {
    bool next = (peek() == c);
    if (next) advance();
    return next;
}

inline char Lexer::peek(size_t offset) {
    return source[current + offset];
}

inline void Lexer::advance() {
    location.column += 1;
    current++;
}

bool Lexer::isAtEnd() {
    return current >= source.size();
}

Token Lexer::handleTab() {
    advance();
    size_t tab_width = 1;
    while (peek(0) == '\t') {
        advance();
        tab_width++;
    }

    return {TokenType::TAB, start, location, tab_width};
}
