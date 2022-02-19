//
//  lexer.c
//  drast
//
//  Created by Ashwin Paudel on 2022-02-05.
//

#include "lexer.h"

static inline char advance(Lexer *lexer);

static inline Token *advance_token(int token_type, char *value, Lexer *lexer, bool does_not_advance);

static inline char peek(Lexer *lexer, uintptr_t offset);

static inline void skip_whitespace(Lexer *lexer);

static inline bool is_whitespace(Lexer *lexer);

static inline Token *parse_string(Lexer *lexer);

static inline Token *parse_identifier(Lexer *lexer);

static inline Token *parse_number(Lexer *lexer);

static inline Token *is_identifier_token(char *identifier, Lexer *lexer);

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

Token *lexer_get_next_token(Lexer *lexer) {
    char c = lexer->current;

    switch (c) {
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            lexer->line += 1;
            lexer->position = 0;
            break;
        case '(':
            return advance_token(T_PARENS_OPEN, "(", lexer, false);
        case ')':
            return advance_token(T_PARENS_CLOSE, ")", lexer, false);
        case '{':
            return advance_token(T_BRACE_OPEN, "{", lexer, false);
        case '}':
            return advance_token(T_BRACE_CLOSE, "}", lexer, false);
        case ',':
            return advance_token(T_COMMA, ",", lexer, false);
        case '.':
            return advance_token(T_PERIOD, ".", lexer, false);
        case '<':
            return advance_token(T_LESS_THAN, "<", lexer, false);
        case '>':
            return advance_token(T_GREATER_THAN, ">", lexer, false);
        case ':':
            return advance_token(T_COLON, ":", lexer, false);
        case ';':
            return advance_token(T_SEMICOLON, ";", lexer, false);
        case '=':
            return advance_token(T_EQUAL, "=", lexer, false);
        case '-':
            if (peek(lexer, 1) == '>') {
                advance(lexer);
                return advance_token(T_ARROW, "->", lexer, false);
            } else
                return advance_token(T_OPERATOR_SUB, "-", lexer, false);
        case '+':
            return advance_token(T_OPERATOR_ADD, "+", lexer, false);
        case '*':
            return advance_token(T_OPERATOR_MUL, "*", lexer, false);
        case '/':
            return advance_token(T_OPERATOR_DIV, "/", lexer, false);
        case '&':
            return advance_token(T_ADDRESS, "&", lexer, false);
        case '"':
            return parse_string(lexer);
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

    return advance_token(T_EOF, "\0", lexer, false);
}

static inline Token *parse_string(Lexer *lexer) {
    char *value = calloc(1, sizeof(char));
    uintptr_t value_count = 0;
    advance(lexer);

    while (lexer->current != '"') {
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

    while (isdigit(lexer->current) || lexer->current == '.') {
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
    else if (strcmp(identifier, "return") == 0)
        return advance_token(T_RETURN, identifier, lexer, true);
    else if (strcmp(identifier, "print") == 0)
        return advance_token(T_PRINT, identifier, lexer, true);
    else if (strcmp(identifier, "if") == 0)
        return advance_token(T_IF, identifier, lexer, true);
    else if (strcmp(identifier, "import") == 0)
        return advance_token(T_IMPORT, identifier, lexer, true);
    else if (strcmp(identifier, "Int") == 0)
        return advance_token(T_INT, identifier, lexer, true);
    else if (strcmp(identifier, "Float") == 0)
        return advance_token(T_FLOAT, identifier, lexer, true);
    else if (strcmp(identifier, "Void") == 0)
        return advance_token(T_VOID, identifier, lexer, true);
    else if (strcmp(identifier, "String") == 0)
        return advance_token(T_STRING, identifier, lexer, true);
    else if (strcmp(identifier, "Bool") == 0)
        return advance_token(T_BOOL, identifier, lexer, true);
    else if (strcmp(identifier, "true") == 0)
        return advance_token(T_BOOL, identifier, lexer, true);
    else if (strcmp(identifier, "false") == 0)
        return advance_token(T_BOOL, identifier, lexer, true);
    else
        return advance_token(T_IDENTIFIER, identifier, lexer, true);
}

static inline bool is_whitespace(Lexer *lexer) {
    return lexer->current == '\t' || lexer->current == ' ' || lexer->current == '\n' || lexer->current == '\r';
}

static inline void skip_whitespace(Lexer *lexer) {
    while (lexer->current == '\t' || lexer->current == ' ' || lexer->current == '\n' || lexer->current == '\r') {
        if (lexer->current == '\n') {
            lexer->position = 0;
            lexer->line++;
        }
        advance(lexer);
    }
}

static inline char peek(Lexer *lexer, uintptr_t offset) {
    return lexer->source[lexer->index + offset];
}

static inline char advance(Lexer *lexer) {
    lexer->position += 1;
    char previous = lexer->current;
    lexer->index++;
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