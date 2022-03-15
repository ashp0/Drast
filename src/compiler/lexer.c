//
//  lexer.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "lexer.h"

static Lexer lexer;

#define LEX_OPERATOR_WITH_EQUAL(value, type, value2, type2) \
    if (lexer_peek() == '=') {                              \
        lexer_advance();                                    \
        return lexer_make_token(#value2, 2, true, type2);   \
    } else {                                                \
        return lexer_make_token(#value, 1, true, type);     \
    }

#define LEX_WHILE(condition, is_string_or_character) \
    lexer.start = lexer.index;                       \
    if (is_string_or_character)                      \
        lexer_advance();                             \
    while (condition) {                              \
        if ((is_string_or_character) && lexer_peek() == '\0') { \
                     lexer_error("Unterminated String Or Character Literal"); \
                exit(-1);                            \
        }                                            \
        lexer_advance();                             \
        if ((is_string_or_character) && lexer.current == T_BACKSLASH) { \
            lexer_advance();                         \
            lexer_advance();                         \
        }                                            \
        if (lexer.current == '\0') {                 \
            if (is_string_or_character) {            \
                lexer_error("Unterminated String Or Character Literal"); \
                exit(-1);                            \
            }                                        \
            break;                                   \
        }                                            \
    }

#define lexer_error printf

void lexer_init(char *source, long length) {
    lexer.source = source;
    lexer.source_length = length;
    lexer.start = 0;
    lexer.index = 0;
    lexer.current = lexer.source[lexer.index];

    lexer.position.line = 1;
    lexer.position.column = 0;
}

Token lexer_get_token() {
    lexer.start = lexer.index - 1;
    lexer_skip_whitespace();

    switch (lexer.current) {
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
            return lexer_identifier();
        case '0' ... '9':
            return lexer_digit();
        case '"':
            return lexer_string();
        case '\'':
            return lexer_character();
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            break;
        case '?':
            return lexer_make_token("?", 1, true, T_QUESTION);
        case '<': {
            if (lexer_peek() == '<') {
                lexer_advance();
                LEX_OPERATOR_WITH_EQUAL(<<, T_BITWISE_SHIFT_LEFT, <<=, T_BITWISE_SHIFT_LEFT_EQUAL)
            } else {
                LEX_OPERATOR_WITH_EQUAL(<, T_LESS_THAN, <=, T_LESS_THAN_EQUAL)
            }
        }
        case '>': {
            if (lexer_peek() == '>') {
                lexer_advance();
                LEX_OPERATOR_WITH_EQUAL(>>, T_BITWISE_SHIFT_RIGHT, >>=, T_BITWISE_SHIFT_RIGHT_EQUAL)
            } else {
                LEX_OPERATOR_WITH_EQUAL(>, T_GREATER_THAN, >=, T_GREATER_THAN_EQUAL)
            }
        }
        case '=':
            LEX_OPERATOR_WITH_EQUAL(=, T_EQUAL, ==, T_EQUAL_EQUAL)
        case '!':
            LEX_OPERATOR_WITH_EQUAL(!, T_NOT, !=, T_NOT_EQUAL)
        case '+':
            LEX_OPERATOR_WITH_EQUAL(+, T_OPERATOR_ADD, +=, T_OPERATOR_ADD_EQUAL)
        case '-':
            LEX_OPERATOR_WITH_EQUAL(-, T_OPERATOR_SUB, -=, T_OPERATOR_SUB_EQUAL)
        case '*':
            LEX_OPERATOR_WITH_EQUAL(*, T_OPERATOR_MUL, *=, T_OPERATOR_MUL_EQUAL)
        case '/': {
            if (lexer_peek() == '/') {
                lexer_skip_line();
                return lexer_get_token();
            } else if (lexer_peek() == '*') {
                lexer_skip_block_comment();
                return lexer_get_token();
            }
            LEX_OPERATOR_WITH_EQUAL(/, T_OPERATOR_DIV, /=, T_OPERATOR_DIV_EQUAL)
        }
        case '%':
            LEX_OPERATOR_WITH_EQUAL(%, T_OPERATOR_MOD, %=, T_OPERATOR_MOD_EQUAL)
        case '&': {
            if (lexer_peek() == '&') {
                lexer_advance();
                LEX_OPERATOR_WITH_EQUAL(&&, T_BITWISE_AND_AND, &&=, T_BITWISE_AND_AND_EQUAL)
            } else {
                LEX_OPERATOR_WITH_EQUAL(&, T_BITWISE_AND, &=, T_BITWISE_AND_EQUAL)
            }
        }
        case '|': {
            if (lexer_peek() == '|') {
                lexer_advance();
                LEX_OPERATOR_WITH_EQUAL(||, T_BITWISE_PIPE_PIPE, ||=, T_BITWISE_PIPE_PIPE_EQUAL)
            } else {
                LEX_OPERATOR_WITH_EQUAL(|, T_BITWISE_PIPE, |=, T_BITWISE_PIPE_EQUAL)
            }
        }

        case '^':
            LEX_OPERATOR_WITH_EQUAL(^, T_BITWISE_POWER, ^=, T_BITWISE_POWER_EQUAL)
        case '~':
            return lexer_make_token("~", 1, true, T_BITWISE_NOT);
        case ':': {
            if (lexer_peek() == ':') {
                lexer_advance();
                return lexer_make_token("::", 2, true, T_DOUBLE_COLON);
            } else
                return lexer_make_token(":", 1, true, T_COLON);
        }
        case ';':
            return lexer_make_token(";", 1, true, T_SEMICOLON);
        case '(':
            return lexer_make_token("(", 1, true, T_PARENS_OPEN);
        case ')':
            return lexer_make_token(")", 1, true, T_PARENS_CLOSE);
        case '[':
            return lexer_make_token("[", 1, true, T_SQUARE_OPEN);
        case ']':
            return lexer_make_token("]", 1, true, T_SQUARE_CLOSE);
        case '{':
            return lexer_make_token("{", 1, true, T_BRACE_OPEN);
        case '}':
            return lexer_make_token("}", 1, true, T_BRACE_CLOSE);
        case ',':
            return lexer_make_token(",", 1, true, T_COMMA);
        case '.':
            return lexer_make_token(".", 1, true, T_PERIOD);
        case '$':
            return lexer_make_token("$", 1, true, T_DOLLAR);
        case '#':
            return lexer_make_token("#", 1, true, T_HASHTAG);
        case '@':
            return lexer_make_token("@", 1, true, T_AT);
        case '\\':
            return lexer_make_token("\\", 1, true, T_BACKSLASH);
        case '\0':
            break;
        default:
            lexer_error("Unknown Character `%c` || %lu :: %lu\n", lexer.current, lexer.position.line,
                        lexer.position.column);
            exit(-1);
    }

    return lexer_make_token("", 1, false, T_EOF);
}

Token lexer_identifier(void) {
    LEX_WHILE(isalnum(lexer_peek()), false)

    char *identifier = &lexer.source[lexer.start];

    int type = token_is_keyword(identifier, lexer.index - lexer.start);

    return lexer_make_token(identifier, (lexer.index - lexer.start + 1), true, type);
}

Token lexer_digit(void) {
    lexer.start = lexer.index;

    while (isdigit(lexer_peek())) {
        lexer_advance();
    }

    char *digit = &lexer.source[lexer.start];

    return lexer_make_token(digit, lexer.index - lexer.start + 1, true, T_NUMBER);
}

Token lexer_string(void) {
    LEX_WHILE(lexer.current != '"', true)

    char *string = &lexer.source[lexer.start];

    return lexer_make_token(string, lexer.index - lexer.start + 1, true, T_STRING);
}

Token lexer_character(void) {
    LEX_WHILE(lexer.current != '\'', true)

    char *character = &lexer.source[lexer.start];

    return lexer_make_token(character, lexer.index - lexer.start + 1, true, T_CHAR);
}

Token lexer_make_token(char *value, size_t length, bool advances, TokenType type) {
    Token token;

    token.type = type;
    token.start = lexer.start;
    token.line = lexer.position.line;
    token.column = lexer.position.column;
    token.value = value;
    token.length = length;

    if (advances) {
        if (lexer.current == '\n') {
            lexer.position.line++;
            lexer.position.column = 0;
        }
        lexer_advance();
    }

    return token;
}

void lexer_skip_whitespace(void) {
    while (lexer.current == '\t' || lexer.current == ' ' || lexer.current == '\n' || lexer.current == '\r') {
        if (lexer.current == '\n') {
            lexer.position.column = 0;
            lexer.position.line += 1;
        }
        if (lexer.current == '\0') {
            return;
        }
        lexer_advance();
    }
}

void lexer_skip_line(void) {
    while (lexer.current != '\n' && lexer.current != '\0') {
        lexer_advance();
    }

    lexer.position.column = 0;

    lexer_skip_whitespace();
}

void lexer_skip_block_comment(void) {
    lexer.index += 2;
    lexer.position.column += 2;
    lexer.current = lexer.source[lexer.index];

    for (;;) {
        if (lexer.current == '*') {
            lexer_advance();
            if (lexer.current == '/') {
                break;
            }

            continue;
        }

        if (lexer.current == '\n') {
            lexer.position.line += 1;
            lexer.position.column = 0;
        }
        if (lexer.current == '\0') {
            fprintf(stderr, "Lexer: Unterminated Block Comment\n");
            exit(-1);
        }
        lexer_advance();
    }

    lexer.index += 2;
    lexer.position.column += 2;
    lexer.current = lexer.source[lexer.index];
}

void lexer_advance(void) {
    lexer.index++;
    lexer.current = lexer.source[lexer.index];
    lexer.position.column++;
}

char lexer_peek(void) {
    return lexer.source[lexer.index + 1];
}

Lexer *lexer_get(void) {
    return &lexer;
}
