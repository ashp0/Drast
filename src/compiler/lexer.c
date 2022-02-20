//
//  lexer.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "lexer.h"

static inline char advance(Lexer *lexer);

static inline Token *advance_token(int token_type, char *value, Lexer *lexer, bool does_not_advance);

static inline char peek_next(Lexer *lexer);

static inline char peek(Lexer *lexer, uintptr_t offset);

static inline void advance_block_comment(Lexer *lexer);

static inline void skip_line(Lexer *lexer);

static inline void skip_whitespace(Lexer *lexer);

static inline bool is_whitespace(Lexer *lexer);

static inline Token *parse_string(Lexer *lexer, char string_type);

static inline Token *parse_identifier(Lexer *lexer);

static inline Token *parse_number(Lexer *lexer);

static inline Token *is_identifier_token(char *identifier, Lexer *lexer);

static inline bool is_eof(Lexer *lexer);

static inline void check_eof(Lexer *lexer, char *error_message);

Lexer *lexer_init(char *source) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->source_length = strlen(lexer->source);

    lexer->index = 0;
    lexer->current = lexer->source[lexer->index];

    lexer->line = 1;
    lexer->position = 0;

    return lexer;
}

Token *lexer_get_next_token_without_advance(Lexer *lexer) {
    Lexer *new_lexer = lexer_init(lexer->source);
    new_lexer->current = lexer->current;
    new_lexer->source = lexer->source;
    new_lexer->index = lexer->index;
    new_lexer->position = lexer->position;
    new_lexer->line = lexer->line;
    new_lexer->source_length = lexer->source_length;

    return lexer_get_next_token(new_lexer);
}

Token *lexer_get_next_token(Lexer *lexer) {
    char c = lexer->current;

    if (is_eof(lexer))
        return advance_token(T_EOF, (char[2]) {lexer->current, 0}, lexer, false);

    switch (c) {
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            lexer->line += 1;
            lexer->position = 0;
            break;
        case '?':
            return advance_token(T_QUESTION, "?", lexer, false);
        case '<':
            if (peek_next(lexer) == '<') {
                advance(lexer);
                if (peek_next(lexer) == '=') {
                    advance(lexer);
                    return advance_token(T_BITWISE_SHIFT_LEFT_EQUAL, "<<=", lexer, false);
                } else {
                    return advance_token(T_BITWISE_SHIFT_LEFT, "<<", lexer, false);
                }
            } else if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_LESS_THAN_EQUAL, "<=", lexer, false);
            } else {
                return advance_token(T_LESS_THAN, "<", lexer, false);
            }
        case '>':
            if (peek_next(lexer) == '>') {
                advance(lexer);
                if (peek_next(lexer) == '=') {
                    advance(lexer);
                    return advance_token(T_BITWISE_SHIFT_RIGHT_EQUAL, ">>=", lexer, false);
                } else {
                    return advance_token(T_BITWISE_SHIFT_RIGHT, ">>", lexer, false);
                }
            } else if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_GREATER_THAN_EQUAL, ">=", lexer, false);
            } else {
                return advance_token(T_GREATER_THAN, ">", lexer, false);
            }
        case '=':
            if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_EQUAL_EQUAL, "==", lexer, false);
            } else {
                return advance_token(T_EQUAL, "=", lexer, false);
            }
        case '!':
            if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_NOT_EQUAL, "!=", lexer, false);
            } else {
                return advance_token(T_NOT, "!", lexer, false);
            }
        case '+':
            if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_OPERATOR_ADD_EQUAL, "+=", lexer, false);
            } else {
                return advance_token(T_OPERATOR_ADD, "+", lexer, false);
            }
        case '-':
            if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_OPERATOR_SUB_EQUAL, "-=", lexer, false);
            } else if (peek_next(lexer) == '>') {
                advance(lexer);
                return advance_token(T_ARROW, "->", lexer, false);
            } else {
                return advance_token(T_OPERATOR_SUB, "-", lexer, false);
            }
        case '*':
            if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_OPERATOR_MUL_EQUAL, "*=", lexer, false);
            } else {
                return advance_token(T_OPERATOR_MUL, "*", lexer, false);
            }
        case '/':
            if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_OPERATOR_DIV_EQUAL, "/=", lexer, false);
            } else if (peek_next(lexer) == '/') {
                skip_line(lexer);
                return lexer_get_next_token(lexer);
            } else if (peek_next(lexer) == '*') {
                advance_block_comment(lexer);
                return lexer_get_next_token(lexer);
            } else {
                return advance_token(T_OPERATOR_DIV, "/", lexer, false);
            }
        case '%':
            if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_OPERATOR_MOD_EQUAL, "%=", lexer, false);
            } else {
                return advance_token(T_OPERATOR_MOD, "%", lexer, false);
            }
        case '&':
            if (peek_next(lexer) == '&') {
                advance(lexer);
                if (peek_next(lexer) == '=') {
                    advance(lexer);
                    return advance_token(T_BITWISE_AND_AND_EQUAL, "&&=", lexer, false);
                } else {
                    return advance_token(T_BITWISE_AND_AND, "&&", lexer, false);
                }
            }
            if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_BITWISE_AND_EQUAL, "&=", lexer, false);
            } else {
                return advance_token(T_BITWISE_AND, "&", lexer, false);
            }
        case '|':
            if (peek_next(lexer) == '|') {
                advance(lexer);
                if (peek_next(lexer) == '=') {
                    advance(lexer);
                    return advance_token(T_BITWISE_PIPE_PIPE_EQUAL, "||=", lexer, false);
                } else {
                    return advance_token(T_BITWISE_PIPE_PIPE, "||", lexer, false);
                }
            }
            if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_BITWISE_PIPE_EQUAL, "|=", lexer, false);
            } else {
                return advance_token(T_BITWISE_PIPE, "|", lexer, false);
            }
        case '^':
            if (peek_next(lexer) == '=') {
                advance(lexer);
                return advance_token(T_BITWISE_POWER_EQUAL, "^=", lexer, false);
            } else {
                return advance_token(T_BITWISE_POWER, "^", lexer, false);
            }
        case '~':
            return advance_token(T_BITWISE_NOT, "~", lexer, false);
        case ':':
            return advance_token(T_COLON, ":", lexer, false);
        case ';':
            return advance_token(T_SEMICOLON, ";", lexer, false);
        case '(':
            return advance_token(T_PARENS_OPEN, "(", lexer, false);
        case ')':
            return advance_token(T_PARENS_CLOSE, ")", lexer, false);
        case '{':
            return advance_token(T_BRACE_OPEN, "{", lexer, false);
        case '}':
            return advance_token(T_BRACE_CLOSE, "}", lexer, false);
        case '[':
            return advance_token(T_SQUARE_OPEN, "[", lexer, false);
        case ']':
            return advance_token(T_SQUARE_CLOSE, "]", lexer, false);
        case ',':
            return advance_token(T_COMMA, ",", lexer, false);
        case '.':
            return advance_token(T_PERIOD, ".", lexer, false);
        case '$':
            return advance_token(T_DOLLAR, "$", lexer, false);
        case '#':
            return advance_token(T_HASHTAG, "#", lexer, false);
        case '@':
            return advance_token(T_AT, "@", lexer, false);
        case '\\':
            return advance_token(T_BACKSLASH, "\\", lexer, false);
        case '"':
            return parse_string(lexer, '"');
        case '\'':
            return parse_string(lexer, '\'');
        case '\0':
            break;
        default:
            if (isdigit(c))
                return parse_number(lexer);
            if (isalpha(c) || c == '_')
                return parse_identifier(lexer);
            fprintf(stderr, "Lexer: Unrecognized Character: `%c`\n", c);
            exit(-1);
    }

    return advance_token(T_EOF, (char[2]) {lexer->current, 0}, lexer, false);
}

static inline Token *parse_string(Lexer *lexer, char string_type) {
    char *value = calloc(1, sizeof(char));
    uintptr_t value_count = 0;
    advance(lexer);

    while (lexer->current != string_type) {
        check_eof(lexer, "Unterminated String Literal\n");

        value = realloc(value, (value_count + 2) * sizeof(char));
        value_count++;
        strcat(value, (char[2]) {lexer->current, 0});

        advance(lexer);
    }

    advance(lexer);

    return advance_token(T_STRING, value, lexer, true);
}

static inline Token *parse_number(Lexer *lexer) {
    char *value = calloc(1, sizeof(char));
    uintptr_t value_count = 0;
    bool is_float_value = false;

    while (isdigit(lexer->current) || lexer->current == '.' || lexer->current == '-' || lexer->current == '+') {
        value = realloc(value, (value_count + 2) * sizeof(char));
        value_count++;
        strcat(value, (char[2]) {lexer->current, 0});
        advance(lexer);

        if (strchr(value, '.')) {
            is_float_value = true;
        }
    }

    if (is_float_value)
        return advance_token(T_FLOAT, value, lexer, true);
    else
        return advance_token(T_INT, value, lexer, true);
}

static inline Token *parse_identifier(Lexer *lexer) {
    char *value = calloc(1, sizeof(char));
    uintptr_t value_count = 0;

    while (isalnum(lexer->current) || lexer->current == '_') {
        value = realloc(value, (value_count + 2) * sizeof(char));
        value_count++;
        strcat(value, (char[2]) {lexer->current, 0});
        advance(lexer);
    }

    return is_identifier_token(value, lexer);
}

static inline Token *is_identifier_token(char *identifier, Lexer *lexer) {
    if (strcmp(identifier, "func") == 0)
        return advance_token(T_FUNC, identifier, lexer, true);
    else if (strcmp(identifier, "let") == 0)
        return advance_token(T_LET, identifier, lexer, true);
    else if (strcmp(identifier, "var") == 0)
        return advance_token(T_VAR, identifier, lexer, true);
    else if (strcmp(identifier, "struct") == 0)
        return advance_token(T_STRUCT, identifier, lexer, true);
    else if (strcmp(identifier, "enum") == 0)
        return advance_token(T_ENUM, identifier, lexer, true);
    else if (strcmp(identifier, "alias") == 0)
        return advance_token(T_ALIAS, identifier, lexer, true);
    else if (strcmp(identifier, "return") == 0)
        return advance_token(T_RETURN, identifier, lexer, true);
    else if (strcmp(identifier, "if") == 0)
        return advance_token(T_IF, identifier, lexer, true);
    else if (strcmp(identifier, "else") == 0)
        return advance_token(T_ELSE, identifier, lexer, true);
    else if (strcmp(identifier, "import") == 0)
        return advance_token(T_IMPORT, identifier, lexer, true);
    else if (strcmp(identifier, "print") == 0)
        return advance_token(T_PRINT, identifier, lexer, true);
    else if (strcmp(identifier, "asm") == 0)
        return advance_token(T_ASM, identifier, lexer, true);
    else if (strcmp(identifier, "volatile") == 0)
        return advance_token(T_VOLATILE, identifier, lexer, true);
    else if (strcmp(identifier, "cast") == 0)
        return advance_token(T_CAST, identifier, lexer, true);
    else if (strcmp(identifier, "switch") == 0)
        return advance_token(T_SWITCH, identifier, lexer, true);
    else if (strcmp(identifier, "case") == 0)
        return advance_token(T_CASE, identifier, lexer, true);
    else if (strcmp(identifier, "break") == 0)
        return advance_token(T_BREAK, identifier, lexer, true);
    else if (strcmp(identifier, "default") == 0)
        return advance_token(T_DEFAULT, identifier, lexer, true);
    else if (strcmp(identifier, "while") == 0)
        return advance_token(T_WHILE, identifier, lexer, true);
    else if (strcmp(identifier, "for") == 0)
        return advance_token(T_FOR, identifier, lexer, true);
    else if (strcmp(identifier, "continue") == 0)
        return advance_token(T_CONTINUE, identifier, lexer, true);
    else if (strcmp(identifier, "union") == 0)
        return advance_token(T_UNION, identifier, lexer, true);
    else if (strcmp(identifier, "false") == 0)
        return advance_token(T_FALSE, identifier, lexer, true);
    else if (strcmp(identifier, "true") == 0)
        return advance_token(T_TRUE, identifier, lexer, true);
    else if (strcmp(identifier, "bool") == 0)
        return advance_token(T_BOOL, identifier, lexer, true);
    else if (strcmp(identifier, "int") == 0)
        return advance_token(T_INT, identifier, lexer, true);
    else if (strcmp(identifier, "float") == 0)
        return advance_token(T_FLOAT, identifier, lexer, true);
    else if (strcmp(identifier, "void") == 0)
        return advance_token(T_VOID, identifier, lexer, true);
    else if (strcmp(identifier, "string") == 0)
        return advance_token(T_STRING, identifier, lexer, true);
    else if (strcmp(identifier, "goto") == 0)
        return advance_token(T_GOTO, identifier, lexer, true);
    else if (strcmp(identifier, "private") == 0)
        return advance_token(T_PRIVATE, identifier, lexer, true);
    else if (strcmp(identifier, "do") == 0)
        return advance_token(T_DO, identifier, lexer, true);
    else if (strcmp(identifier, "try") == 0)
        return advance_token(T_TRY, identifier, lexer, true);
    else if (strcmp(identifier, "catch") == 0)
        return advance_token(T_CATCH, identifier, lexer, true);
    else if (strcmp(identifier, "throw") == 0)
        return advance_token(T_THROW, identifier, lexer, true);
    else if (strcmp(identifier, "throws") == 0)
        return advance_token(T_THROWS, identifier, lexer, true);
    else
        return advance_token(T_IDENTIFIER, identifier, lexer, true);
}

static inline void advance_block_comment(Lexer *lexer) {
    advance(lexer);
    advance(lexer);
    while (lexer->current != '*' && peek_next(lexer) != '/') {
        advance(lexer);
        check_eof(lexer, "Unterminated Block Comment\n");
    }
    advance(lexer);
    advance(lexer);
}

static inline bool is_whitespace(Lexer *lexer) {
    return lexer->current == '\t' || lexer->current == ' ' || lexer->current == '\n' || lexer->current == '\r';
}

static inline void skip_whitespace(Lexer *lexer) {
    do {
        if (lexer->current == '\n') {
            lexer->position = 0;
            lexer->line++;
        }
        advance(lexer);
    } while (lexer->current == '\t' || lexer->current == ' ' || lexer->current == '\n' || lexer->current == '\r');
}

static inline void skip_line(Lexer *lexer) {
    while (lexer->current != '\n')
        advance(lexer);
    if (is_whitespace(lexer))
        skip_whitespace(lexer);
    lexer->line++;
    lexer->position = 0;
}

static inline char peek_next(Lexer *lexer) {
    return lexer->source[lexer->index + 1];
}

static inline char peek(Lexer *lexer, uintptr_t offset) {
    return lexer->source[lexer->index + offset];
}

static inline char advance(Lexer *lexer) {
    lexer->position += 1;
    char previous = lexer->current;
    lexer->index += 1;
    lexer->current = lexer->source[lexer->index];

    return previous;
}

static inline Token *advance_token(int token_type, char *value, Lexer *lexer, bool does_not_advance) {
    Token *new_token = malloc(sizeof(Token));
    new_token->value = value;
    new_token->type = token_type;

    if (!does_not_advance)
        advance(lexer);
    if (is_whitespace(lexer))
        skip_whitespace(lexer);

    return new_token;
}

static inline bool is_eof(Lexer *lexer) {
    return (lexer->index >= lexer->source_length);
}

static inline void check_eof(Lexer *lexer, char *error_message) {
    if (is_eof(lexer)) {
        fprintf(stderr, "Lexer: %s\n", error_message);
        exit(-1);
    }
}