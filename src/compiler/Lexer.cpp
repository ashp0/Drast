//
// Created by Ashwin Paudel on 2022-03-20.
//

#include "Lexer.h"

void Lexer::lex() {
    for (;;) {
        auto token = this->getToken();
        if (token->type == TokenType::T_EOF) {
            break;
        }

        this->tokens.push_back(std::move(token));
    }

    this->tokens.push_back(returnToken(TokenType::T_EOF, true));
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
        return returnToken(TokenType::QUESTION);
    case '<':
        if (this->peek() == '<') {
            this->advance();
            return this->returnToken(TokenType::BITWISE_SHIFT_LEFT,
                                     TokenType::BITWISE_SHIFT_LEFT_EQUAL);
        }

        return this->returnToken(TokenType::LESS_THAN,
                                 TokenType::LESS_THAN_EQUAL);
    case '>':
        if (this->peek() == '>') {
            this->advance();
            return this->returnToken(TokenType::BITWISE_SHIFT_RIGHT,
                                     TokenType::BITWISE_SHIFT_RIGHT_EQUAL);
        }

        return this->returnToken(TokenType::GREATER_THAN,
                                 TokenType::GREATER_THAN_EQUAL);
    case '=':
        return this->returnToken(TokenType::EQUAL, TokenType::EQUAL_EQUAL);
    case '!':
        return this->returnToken(TokenType::NOT, TokenType::NOT_EQUAL);
    case '+':
        return this->returnToken(TokenType::OPERATOR_ADD,
                                 TokenType::OPERATOR_ADD_EQUAL);
    case '-':
        return this->returnToken(TokenType::OPERATOR_SUB,
                                 TokenType::OPERATOR_SUB_EQUAL);
    case '*':
        return this->returnToken(TokenType::OPERATOR_MUL,
                                 TokenType::OPERATOR_MUL_EQUAL);
    case '/':
        if (peek() == '/') {
            this->skipLine();
            return this->getToken();
        } else if (peek() == '*') {
            this->skipBlockComment();
            return this->getToken();
        }
        return this->returnToken(TokenType::OPERATOR_DIV,
                                 TokenType::OPERATOR_DIV_EQUAL);
    case '%':
        return this->returnToken(TokenType::OPERATOR_MOD,
                                 TokenType::OPERATOR_MOD_EQUAL);

    case '&':
        if (peek() == '&') {
            this->advance();
            return this->returnToken(TokenType::BITWISE_AND_AND,
                                     TokenType::BITWISE_AND_AND_EQUAL);
        }
        return this->returnToken(TokenType::BITWISE_AND,
                                 TokenType::BITWISE_AND_EQUAL);
    case '|':
        if (peek() == '|') {
            this->advance();
            return this->returnToken(TokenType::BITWISE_PIPE_PIPE,
                                     TokenType::BITWISE_PIPE_PIPE_EQUAL);
        }
        return this->returnToken(TokenType::BITWISE_PIPE,
                                 TokenType::BITWISE_PIPE_EQUAL);
    case '^':
        return this->returnToken(TokenType::BITWISE_POWER,
                                 TokenType::BITWISE_POWER_EQUAL);
    case '~':
        return this->returnToken(TokenType::BITWISE_NOT);
    case ':':
        if (peek() == ':') {
            this->advance();
            return this->returnToken(TokenType::DOUBLE_COLON);
        }
        return this->returnToken(TokenType::COLON);
    case ';':
        return this->returnToken(TokenType::SEMICOLON);
    case '(':
        return this->returnToken(TokenType::PARENS_OPEN);
    case ')':
        return this->returnToken(TokenType::PARENS_CLOSE);
    case '[':
        return this->returnToken(TokenType::SQUARE_OPEN);
    case ']':
        return this->returnToken(TokenType::SQUARE_CLOSE);
    case '{':
        return this->returnToken(TokenType::BRACE_OPEN);
    case '}':
        return this->returnToken(TokenType::BRACE_CLOSE);
    case ',':
        return this->returnToken(TokenType::COMMA);
    case '.':
        return this->returnToken(TokenType::PERIOD);
    case '$':
        return this->returnToken(TokenType::DOLLAR);
    case '#':
        return this->returnToken(TokenType::HASHTAG);
    case '@':
        return this->returnToken(TokenType::AT);
    case '\\':
        return this->returnToken(TokenType::BACKSLASH);
    case '\0':
        break;
    default:
        std::cout << "ERROR" << std::endl;
        exit(EXIT_FAILURE);
    }

    return returnToken(TokenType::T_EOF, true);
}

std::unique_ptr<Token> Lexer::identifier() {
    return this->lexWhile(TokenType::IDENTIFIER, [this]() {
        return isalnum(this->current) || this->current == '_';
    });
}

std::unique_ptr<Token> Lexer::number() {
    return this->lexWhile(TokenType::V_NUMBER,
                          [this]() { return isnumber(this->current); });
}

std::unique_ptr<Token> Lexer::string() {
    return this->lexWhile(
        TokenType::V_STRING, [this]() { return (this->current != '"'); }, true);
}

std::unique_ptr<Token> Lexer::character() {
    return this->lexWhile(
        TokenType::V_CHAR, [this]() { return (this->current != '\''); }, true);
}

std::unique_ptr<Token> Lexer::returnToken(TokenType type,
                                          bool without_advance) {
    std::string current_string = {this->current};
    return this->returnToken(type, current_string, without_advance);
}

std::unique_ptr<Token> Lexer::returnToken(TokenType type, std::string &string,
                                          bool without_advance) {
    auto return_token =
        std::make_unique<Token>(string, type, this->line, this->column);
    if (!without_advance) {
        this->advance();
    }
    return return_token;
}

std::unique_ptr<Token> Lexer::returnToken(TokenType first_type,
                                          TokenType second_type) {
    return this->returnToken(this->peek() == '=' ? second_type : first_type);
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
    while (this->current != '\n' && this->current != '\0') {
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

char Lexer::peek(size_t offset) { return this->source[this->index + offset]; }

template <typename predicate>
std::unique_ptr<Token> Lexer::lexWhile(TokenType type, predicate &&pred,
                                       bool is_string) {
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

    auto position =
        is_string ? this->index - this->start - 1 : this->index - this->start;
    std::string value = this->source.substr(this->start, position);

    if (type == TokenType::IDENTIFIER) {
        TokenType type1 = Token::is_keyword(value, this->index - this->start);
        return this->returnToken(type1, value, true);
    }
    return this->returnToken(type, value, true);
}
