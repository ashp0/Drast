//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "Lexer.h"

namespace drast::lexer {

Lexer::Lexer(const std::string &file_name, const Error &error)
    : error(error), location(1, 1) {

    utils::readFile(file_name, file_buffer);
    error.buffer = file_buffer;
}

void Lexer::lex() {
    for (auto token = getToken(); token.type != TokenType::T_EOF;
         token = getToken()) {
        tokens.push_back(token);
    }

    if (did_encounter_error) {
        error.displayMessages();
    }

    //    for (auto &token : tokens) {
    //        if (token.type != TokenType::NEW_LINE) {
    //            std::cout << token.toString(file_buffer) << '\n';
    //        }
    //    }
    tokens.push_back(returnToken(TokenType::T_EOF));
}

Token Lexer::getToken() {
    advanceWhitespace();
    start = buffer_index;

    switch (current()) {
    case 'a' ... 'z':
    case 'A' ... 'Z':
        return identifier();
    case '0' ... '9':
        return number();
    case '"':
        return string();
    case '\'':
        return character();
    case '\0':
        break;
    case '?':
        return createOperator(TokenType::QUESTION);
    case '<':
        if (peek() == '<') {
            advance();
            return createOperator(TokenType::BITWISE_SHIFT_LEFT,
                                  TokenType::BITWISE_SHIFT_LEFT_EQUAL);
        }
        return createOperator(TokenType::LESS_THAN, TokenType::LESS_THAN_EQUAL);
    case '>':
        if (peek() == '>') {
            advance();
            return createOperator(TokenType::BITWISE_SHIFT_RIGHT,
                                  TokenType::BITWISE_SHIFT_RIGHT_EQUAL);
        }
        return createOperator(TokenType::GREATER_THAN,
                              TokenType::GREATER_THAN_EQUAL);
    case '=':
        return createOperator(TokenType::EQUAL, TokenType::EQUAL_EQUAL);
    case '!':
        return createOperator(TokenType::NOT, TokenType::NOT_EQUAL);
    case '+':
        return createOperator(TokenType::OPERATOR_ADD,
                              TokenType::OPERATOR_ADD_EQUAL);
    case '-':
        return createOperator(TokenType::OPERATOR_SUB,
                              TokenType::OPERATOR_SUB_EQUAL);
    case '*':
        return createOperator(TokenType::OPERATOR_MUL,
                              TokenType::OPERATOR_MUL_EQUAL);
    case '/':
        if (peek() == '/') {
            advanceLineComment();
            return getToken();
        } else if (peek() == '*') {
            advanceMultilineComment();
            return getToken();
        }
        return createOperator(TokenType::OPERATOR_DIV,
                              TokenType::OPERATOR_DIV_EQUAL);
    case '%':
        return createOperator(TokenType::OPERATOR_MOD,
                              TokenType::OPERATOR_MOD_EQUAL);
    case '&':
        return createOperator(TokenType::BITWISE_AND,
                              TokenType::BITWISE_AND_EQUAL);
    case '|':
        if (peek() == '|') {
            advance();
            return createOperator(TokenType::BITWISE_PIPE_PIPE,
                                  TokenType::BITWISE_PIPE_PIPE_EQUAL);
        }
        return createOperator(TokenType::BITWISE_PIPE,
                              TokenType::BITWISE_PIPE_EQUAL);
    case '^':
        return createOperator(TokenType::BITWISE_POWER,
                              TokenType::BITWISE_POWER_EQUAL);
    case '~':
        return createOperator(TokenType::BITWISE_NOT);
    case ':':
        if (peek() == ':') {
            advance();
            return createOperator(TokenType::DOUBLE_COLON);
        }
        return createOperator(TokenType::COLON);
    case ';':
        return createOperator(TokenType::SEMICOLON);
    case '(':
        return createOperator(TokenType::PARENS_OPEN);
    case ')':
        return createOperator(TokenType::PARENS_CLOSE);
    case '[':
        return createOperator(TokenType::SQUARE_OPEN);
    case ']':
        return createOperator(TokenType::SQUARE_CLOSE);
    case '{':
        return createOperator(TokenType::BRACE_OPEN);
    case '}':
        return createOperator(TokenType::BRACE_CLOSE);
    case ',':
        return createOperator(TokenType::COMMA);
    case '.':
        return createOperator(TokenType::PERIOD);
    case '$':
        return createOperator(TokenType::DOLLAR);
    case '#':
        return createOperator(TokenType::HASHTAG);
    case '@':
        return createOperator(TokenType::AT);
    case '\\':
        return createOperator(TokenType::BACKSLASH);
    default:
        return throwError("Unexpected Character `" + std::string({current()}) +
                          "`");
    }

    return returnToken(TokenType::T_EOF, true);
}

Token Lexer::identifier() {
    start = buffer_index;

    while (utils::isAlphaNumeric(current())) {
        advance();
    }

    uint32_t length = buffer_index - start;

    std::string_view identifier(file_buffer.data() + start, length);

    TokenType type1 = Token::isKeyword(identifier);
    return returnToken(type1, true);
}

Token Lexer::hexadecimal() {
    advance();

    return lexWhile(TokenType::V_HEX,
                    [](char c) { return utils::isHexadecimal(c); });
}

Token Lexer::octal() {
    advance();

    return lexWhile(TokenType::V_OCTAL,
                    [](char c) { return utils::isOctal(c); });
}

Token Lexer::binary() {
    advance();

    return lexWhile(TokenType::V_BINARY,
                    [](char c) { return utils::isBinary(c); });
}

Token Lexer::number() {
    start = buffer_index;

    // Check for other numbers
    if (current() == '0') {
        advance();
        switch (current()) {
        case 'x':
        case 'X':
            return hexadecimal();
        case 'b':
        case 'B':
            return binary();
        case 'o':
        case 'O':
            return octal();
        }
    }

    bool is_float = false;

loop:
    while (utils::isNumber(current())) {
        if (current() == '\n') {
            break;
        }
        advance();
    }

    if (current() == '.') {
        if (is_float) {
            return throwError("Invalid number");
        }
        is_float = true;

        advance();
        if (!utils::isNumber(current())) {
            return throwError("Invalid number");
        }

        goto loop;
    }

    return returnToken(is_float ? TokenType::V_FLOAT : TokenType::V_INT, true);
}

Token Lexer::string() {
    advance();

    if (current() == '"' && peek() == '"') {
        return multilineString();
    }

    while (current() != '"') {
        switch (current()) {
        case '\n':
        case '\0':
            return throwError("Unterminated string literal");
        case '\\':
            advance();
            evaluateEscapeSequence();
            continue;
        default:
            break;
        }
        advance();
    }

    return returnToken(TokenType::V_STRING);
}

Token Lexer::character() {
    advance();

    switch (current()) {
    case '\\':
        advance();
        evaluateEscapeSequence();
        break;
    case '\'':
        return throwError("Empty character");
    case '\0':
        return throwError("Unterminated character");
    }
    advance();

    return returnToken(TokenType::V_CHAR);
}

Token Lexer::multilineString() {
    advance(2);

loop:
    // Run a loop until we find three of the string character
    while (current() != '"') {
        switch (current()) {
        case '\0':
            return throwError("Unterminated string literal");
        case '\n':
            advanceLine();
            break;
        case '\\':
            advance();
            evaluateEscapeSequence();
        default:
            break;
        }
        advance();
    }

    if (peek() == '"' && peek(2) == '"') {
        advance(3);
        return returnToken(TokenType::V_MULTILINE_STRING, true);
    } else {
        advance();
        goto loop;
    }
}

inline Token Lexer::createOperator(TokenType type) { return returnToken(type); }

inline Token Lexer::createOperator(TokenType type, TokenType type2) {
    if (peek() == '=') {
        advance();
        return returnToken(type2);
    }
    return returnToken(type);
}

inline Token Lexer::returnToken(TokenType type, bool without_advance) {
    auto return_token = Token(type, start, buffer_index - start, location);
    if (!without_advance) {
        advance();
        return_token.location.column += 1;
        return_token.length += 1;
    }
    return return_token;
}

inline void Lexer::advanceWhitespace() {
    while (isspace(current())) {
        if (current() == '\n') {
            tokens.push_back(returnToken(TokenType::NEW_LINE, true));
            advanceLine();
        }
        if (current() == '\0') {
            break;
        }
        advance();
    }
}

inline void Lexer::advanceLineComment() {
    while (current() != '\n') {
        if (current() == '\0') {
            break;
        }
        advance();
    }
}

inline void Lexer::advanceMultilineComment() {
    advance(2);
    auto comment_start = location;
    while (current() != '*' || peek() != '/') {
        switch (current()) {
        case '\0':
            throwError("Unterminated block comment.", comment_start);
            return;
        case '/':
            if (peek() == '*')
                advanceMultilineComment();
        case '\n':
            advanceLine();
            break;
        default:
            break;
        }
        advance();
    }
    advance(2);
}

inline void Lexer::advanceLine() {
    location.line += 1;
    location.column = 0;
}

void Lexer::advance() {
    location.column += 1;
    buffer_index += 1;
}

void Lexer::advance(size_t offset) {
    location.column += offset;
    buffer_index += offset;
}

void Lexer::evaluateEscapeSequence() {
    switch (current()) {
    case '0':
    case 'a':
    case 'b':
    case 'f':
    case 'n':
    case 'r':
    case 't':
    case 'v':
    case 'x':
    case '\'':
    case '\\':
        advance();
        return;
    default:
        throwError("Invalid escape sequence.");
        break;
    }
}

char Lexer::peek(size_t offset) { return file_buffer[buffer_index + offset]; }

Token Lexer::lexWhile(TokenType type, bool (*predicate)(char)) {
    while (predicate(current())) {
        switch (current()) {
        case '\0':
        case '\n':
            goto end;
        }
        advance();
    }
end:
    return returnToken(type, true);
}

Token Lexer::throwError(const std::string &message) {
    error.addError(message, location);
    did_encounter_error = true;
    advanceLineComment();
    return getToken();
}

Token Lexer::throwError(const std::string &message, const Location &loc) {
    error.addError(message, loc);
    did_encounter_error = true;
    advanceLineComment();
    return getToken();
}

} // namespace drast::lexer
