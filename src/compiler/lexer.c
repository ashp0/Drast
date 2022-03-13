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
        return lexer_make_token(#value2, type2, true);      \
    } else {                                                \
        return lexer_make_token(#value, type, true);        \
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
        if (lexer.current == '\n') {                 \
            lexer.position.line++;                   \
            lexer.position.column = 0;               \
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

void lexer_init(char *source) {
    lexer.source = source;
    lexer.source_length = strlen(lexer.source);
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
            lexer.position.line++;
            lexer.position.column = 0;
            break;
        case '?':
            return lexer_make_token("?", T_QUESTION, true);
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
            return lexer_make_token("~", T_BITWISE_NOT, true);
        case ':': {
            if (lexer_peek() == ':') {
                lexer_advance();
                return lexer_make_token("::", T_DOUBLE_COLON, true);
            } else
                return lexer_make_token(":", T_COLON, true);
        }
        case ';':
            return lexer_make_token(";", T_SEMICOLON, true);
        case '(':
            return lexer_make_token("(", T_PARENS_OPEN, true);
        case ')':
            return lexer_make_token(")", T_PARENS_CLOSE, true);
        case '[':
            return lexer_make_token("[", T_SQUARE_OPEN, true);
        case ']':
            return lexer_make_token("]", T_SQUARE_CLOSE, true);
        case '{':
            return lexer_make_token("{", T_BRACE_OPEN, true);
        case '}':
            return lexer_make_token("}", T_BRACE_CLOSE, true);
        case ',':
            return lexer_make_token(",", T_COMMA, true);
        case '.':
            return lexer_make_token(".", T_PERIOD, true);
        case '$':
            return lexer_make_token("$", T_DOLLAR, true);
        case '#':
            return lexer_make_token("#", T_HASHTAG, true);
        case '@':
            return lexer_make_token("@", T_AT, true);
        case '\\':
            return lexer_make_token("\\", T_BACKSLASH, true);
        case '\0':
            break;
        default:
            lexer_error("Unknown Character `%c` || %lu :: %lu\n", lexer.current, lexer.position.line,
                        lexer.position.column);
            exit(-1);
    }

    return lexer_make_token("", T_EOF, false);
}

Token lexer_identifier(void) {
    LEX_WHILE(isalnum(lexer.current), false)

    char *identifier = &lexer.source[lexer.start];

    int type = token_is_keyword(identifier, lexer.index - lexer.start);

    return lexer_make_token(identifier, type, true);
}

Token lexer_digit(void) {
    lexer.start = lexer.index;

    while (isdigit(lexer.current)) {
        lexer_advance();
    }

    char *digit = &lexer.source[lexer.start];

    return lexer_make_token(digit, T_NUMBER, true);
}

Token lexer_string(void) {
    LEX_WHILE(lexer.current != '"', true)

    char *string = &lexer.source[lexer.start];

    return lexer_make_token(string, T_STRING, true);
}

Token lexer_character(void) {
    LEX_WHILE(lexer.current != '\'', true)

    char *character = &lexer.source[lexer.start];

    return lexer_make_token(character, T_CHAR, true);
}

Token lexer_make_token(char *value, TokenType type, bool advances) {
    Token token;

    token.type = type;
    token.start = lexer.start;
    token.line = lexer.position.line;
    token.value = value;

    if (advances) {
        if (lexer.current == '\n') {
            lexer.position.line++;
            lexer.position.column = 0;
        }
        lexer_advance();
    }

    token.length = lexer.index - lexer.start;

    return token;
}

void lexer_skip_whitespace(void) {
    while (lexer.current == '\t' || lexer.current == ' ' || lexer.current == '\n' || lexer.current == '\r') {
        if (lexer.current == '\n') {
            lexer.position.column = 0;
            lexer.position.line++;
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
    lexer.position.line++;

    lexer_skip_whitespace();
}

void lexer_skip_block_comment(void) {
    lexer.index += 2;
    lexer.position.column += 2;
    lexer.current = lexer.source[lexer.index];

    while (lexer.current != '*' && lexer_peek() != '/') {
        if (lexer.current == '\n') {
            lexer.position.line++;
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
